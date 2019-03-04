#include "libcls.h"
#include <libutl/BufferedFDstream.h>
#include <clp/FailEx.h>
#include <clp/IntExpDomainARit.h>
#include <clp/IntExpDomainRISC.h>
#include <clp/IntSpan.h>
#include "CompositeResource.h"
#include "CompositeResourceRequirement.h"
#include "ESboundInt.h"
#include "Schedule.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef DEBUG
// #define DEBUG_UNIT
#endif

//////////////////////////////////////////////////////////////////////////////f
UTL_NS_USE;
LUT_NS_USE;
CLP_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(cls::ESboundInt);

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

ESboundInt::ESboundInt(IntActivity* act, int lb)
    : SchedulableBound(act->manager(), clp::bound_lb, lb)
{
    _act = act;
    _numResources = 0;
    _caps = nullptr;
    _resources = nullptr;
    _prs = nullptr;
    _cspans = nullptr;
    _visited = nullptr;
    _numVisited = 0;
    _efBound = nullptr;
    _baseWorkPerTS = 0.0;
    _minMultiple = 0;
    _maxMultiple = 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ESboundInt::initialize()
{
    ASSERTD(_act != nullptr);
    ASSERTD(_act->schedule() != nullptr);
    _numResources = _act->requirements().size();

    // no requirements => nothing else to do
    if (_numResources == 0)
    {
        return;
    }

    _caps = new uint_t[_numResources];
    _resources = new CompositeResource*[_numResources];
    _prs = new const PreferredResources*[_numResources];
    _cspans = new CompositeSpan*[_numResources];

    // fill in _caps[], _resources[], and init _visited[]
    _numVisited = 0;
    uint_t idx = 0;
    for (auto crr_ : _act->requirements())
    {
        auto crr = utl::cast<CompositeResourceRequirement>(crr_);
        _caps[idx] = crr->minCapacity();
        _resources[idx] = crr->resource();
        _prs[idx] = crr->preferredResources();
        _numVisited += _resources[idx]->numResources();
        ++idx;
    }
    _visited = new DiscreteResource*[_numVisited];
    _numVisited = 0;

    // find maximum multiple of base capacity
    initMinMaxMultiple();

    // invalidate and queue
    queueFind();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ESboundInt::registerEvents()
{
    IntExp* breakList = _act->breakList();
    if (breakList == nullptr)
    {
        return;
    }
    breakList->addDomainBound(this);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ESboundInt::allocateCapacity()
{
    IntExp* breakList = _act->breakList();

    // no break-list defined => nothing to do
    if (breakList == nullptr)
    {
        return;
    }

    breakList->removeDomainBound(this);

    // zero processing-time => nothing else to do
    if (_act->processingTime() == 0)
    {
        return;
    }

    breakList->setDeferRemoves(true);

    // find forward (with allocation)
    int ef;
    findForward(_bound, ef, true);

    // allocation failed?!
    if (_bound == int_t_max)
    {
        throw FailEx(_name + ": allocation failed");
    }

    breakList->setDeferRemoves(false);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ESboundInt::deallocateCapacity()
{
    _act->deallocate();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ESboundInt::setMaxMultiple(uint_t maxMultiple)
{
    ASSERTD(_act != nullptr);
    Manager* mgr = _act->manager();
    mgr->revSet(_maxMultiple, utl::min(_maxMultiple, maxMultiple));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int
ESboundInt::find()
{
    int ef;
    const IntExpDomainRISC* domain;

    saveState();

    // reference break-list
    const IntExp* breakList = _act->breakList();
    if (breakList == nullptr)
    {
        ef = _efBound->get();
        _bound = utl::max(_bound, ef + 1);
        ef = utl::max(ef, _bound - 1);
        goto succeed;
    }

    // note: breakList != nullptr

    if (breakList->size() == 0)
        goto fail;
    domain = breakList->domainRISC();

    // handle (pt == 0) as special case
    if (_act->processingTime() == 0)
    {
        ef = _efBound->get();
        if (_bound <= ef)
        {
            // adjust es,ef so that es = (ef + 1) and es not in break
            _bound = ef + 1;
            const IntSpan* span = domain->find(_bound);
            if (span->v0() == 0)
                span = span->next();
            if (span->v0() == 0)
                goto fail;
            _bound = max(_bound, span->min());
            ef = _bound - 1;
        }
        else // _bound > ef
        {
            ef = _bound - 1;
        }
        goto succeed;
    }

    // undo previous allocation?
    /*if (_act->allocated())
    {
#ifdef DEBUG
        utl::cerr
            << "moving a previously scheduled activity: "
            << _act->id() <<
            utl::endlf;
#endif
        deallocateCapacity();
    }*/

    // given es, find ef
    findForward(_bound, ef);
    if (ef == int_t_max)
        goto fail;

#ifdef DEBUG_UNIT
    if (_act->id() == 472)
    {
        utl::cout << "--- find() --------------------" << utl::endlf;
        utl::cout << "act-" << _act->id() << ", es = " << _bound << ", ef = " << ef << utl::endlf;
    }
#endif

    // finish too early?
    if (ef < _efBound->get())
    {
        // given ef, find es
        ef = _efBound->get();
        findBackward(_bound, ef);
        if (_bound == int_t_min)
            goto fail;
        findForward(_bound, ef);

#ifdef DEBUG_UNIT
        if (_act->id() == 472)
        {
            utl::cout << "ef < " << _efBound->get() << " => "
                      << "es = " << _bound << "ef = " << ef << utl::endlf;
        }
#endif
    }

    // re-allocate?
    /*if (_act->allocated())
    {
        allocateCapacity();
    }*/
succeed:
    _findPoint = _bound;
    _efBound->setFindPoint(ef);
    _efBound->setLB(ef);
    return _bound;
fail:
    _bound = ef = int_t_max;
    throw FailEx(_name + ": no workable bound");
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ESboundInt::deInit()
{
    delete[] _caps;
    delete[] _resources;
    delete[] _prs;
    delete[] _cspans;
    delete[] _visited;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ESboundInt::initMinMaxMultiple()
{
    uint_t idx;

    // normalize _caps[], set minMultiple
    if (_numResources == 1)
    {
        _minMultiple = _caps[0];
        _caps[0] = 1;
    }
    else
    {
        // initially, gcd = minimum of the minimum capacities
        uint_t gcd = uint_t_max;
        for (idx = 0; idx < _numResources; ++idx)
        {
            gcd = utl::min(gcd, _caps[idx]);
        }

        // gcd = largest value that is a common divisor for _caps[]
        for (; gcd > 1; --gcd)
        {
            for (idx = 0; idx < _numResources; ++idx)
            {
                if ((_caps[idx] % gcd) != 0)
                {
                    break;
                }
            }
            if (idx == _numResources)
                break;
        }

        // use gcd as _minMultiple, and normalize _caps[]
        _minMultiple = gcd;
        for (idx = 0; idx < _numResources; ++idx)
        {
            _caps[idx] /= gcd;
        }
    }

    _baseWorkPerTS = 1.0 / (double)_minMultiple;

    // maxCaps[] : maximum allowed capacity for each resource
    uint_t* maxCaps = new uint_t[_numResources];

    // fill in maxCaps[], determine if there is an upper bound on capacity
    idx = 0;
    for (auto crr_ : _act->requirements())
    {
        auto crr = utl::cast<CompositeResourceRequirement>(crr_);
        auto cres = crr->resource();
        // set maxCaps[idx]
        maxCaps[idx] = utl::min(crr->maxCapacity(), cres->numResources());
        maxCaps[idx] = utl::max(maxCaps[idx], _caps[idx]);
        ++idx;
    }

    // _maxMultiple = highest multiple without exceeding maxCaps[]
    bool done = false;
    uint_t multiple;
    for (multiple = _minMultiple + 1;; ++multiple)
    {
        for (idx = 0; idx < _numResources; ++idx)
        {
            if ((_caps[idx] * multiple) > maxCaps[idx])
            {
                done = true;
                break;
            }
        }

        // done?
        if (done)
        {
            break;
        }
    }
    _maxMultiple = multiple - 1;

    // release maxCaps[]
    delete[] maxCaps;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ESboundInt::findForward(int& es, int& ef, bool allocate)
{
    bool firstExecution = true;
    double remainingWork = _act->processingTime();
    Resource** resourcesArray = _act->schedule()->resourcesArray();

    int t;

    // reference break-list
    const IntExp* breakList = _act->breakList();
    ASSERTD(breakList != nullptr);
    ASSERTD(breakList->size() > 0);
    const IntExpDomainRISC* domain = breakList->domainRISC();

    // find breakList span
    const IntSpan* span = domain->find(es);
    if (span->v0() == 0)
        span = span->next();
    if (span->v0() == 0)
        goto fail;
    es = max(es, span->min());

    // initialize _cspans[]
    uint_t i, j;
    for (i = 0; i < _numResources; ++i)
    {
        Resource* res = _resources[i];
        ASSERTD(res->isA(CompositeResource));
        CompositeResource* cres = (CompositeResource*)res;
        const CompositeTimetableDomain* ttd = cres->timetable().domain();
        _cspans[i] = (CompositeSpan*)ttd->find(es);
    }

#ifdef DEBUG_UNIT
    if (_act->id() == 472)
    {
        utl::cout << "--- findForward() -------------------- " << utl::endlf;
    }
#endif

    // use maximum permitted capacity until the work is done
    t = es;
    for (;;)
    {
#ifdef DEBUG_UNIT
        if (_act->id() == 472)
        {
            utl::cout << "remainingWork = " << remainingWork << utl::endlf;
        }
#endif

        // find end of overlap: current time window is defined by
        //     [t,overlapMax]
        // also, determine capacity multiplier
        int overlapMax = span->max();
        uint_t multiple = _maxMultiple;
        for (i = 0; i < _numResources; ++i)
        {
            CompositeSpan* cspan = _cspans[i];

#ifdef DEBUG_UNIT
            if (_act->id() == 472)
                utl::cout << "span = " << span->toString() << ", cspan = " << cspan->toString()
                          << utl::endlf;
#endif

            // limit overlapMax
            overlapMax = utl::min(overlapMax, cspan->max());

            // count remaining capacity in the CompositeSpan
            // (but don't count previously visited resources)
            uint_t remCap = 0;
            uint_t maxCap = (multiple == uint_t_max) ? uint_t_max : multiple * _caps[i];
            IntExpDomainIt* it;
            for (it = cspan->resIds()->begin(); !it->atEnd(); it->next())
            {
                int resId = it->get();
                Resource* res = resourcesArray[resId];
                ASSERTD(res->isA(DiscreteResource));
                DiscreteResource* dres = (DiscreteResource*)res;

                // already visited?
                if (dres->visited())
                {
                    continue;
                }

                dres->visited() = true;
                _visited[_numVisited++] = dres;

                // reached the maximum?
                if (++remCap == maxCap)
                {
                    break;
                }
            }
            delete it;

            multiple = utl::min(multiple, remCap / _caps[i]);
        }

        // clear _visited[]
        for (j = 0; j < _numVisited; ++j)
        {
            _visited[j]->visited() = false;
        }
        _numVisited = 0;

        // execute during [t,overlapMax] ?
        if (multiple >= _minMultiple)
        {
            if (firstExecution)
            {
                es = t;
                firstExecution = false;
            }

            uint_t windowSize = (overlapMax - t + 1);
            double work = _baseWorkPerTS * multiple * windowSize;

#ifdef DEBUG_UNIT
            if (_act->id() == 472)
            {
                utl::cout << "es = " << es << ", t = " << t << ", windowSize = " << windowSize
                          << ", work = " << work << utl::endlf;
            }
#endif

            // done?
            if (work >= remainingWork)
            {
                // done: set ef
                double pct = remainingWork / work;
                double pct_x_ws = pct * windowSize;
                double pct_x_ws_floor = floor(pct_x_ws);
                uint_t numTS = (uint_t)pct_x_ws_floor;
                if ((pct_x_ws - pct_x_ws_floor) > 0.0001)
                {
                    ++numTS;
                }
                ef = t + numTS - 1;
                overlapMax = ef;
#ifdef DEBUG_UNIT
                if (_act->id() == 472)
                {
                    utl::cout << "pct = " << pct << ", pct_x_ws = " << pct_x_ws
                              << ", pct_x_ws_floor = " << pct_x_ws_floor << utl::endlf << ef
                              << " = " << t << " + " << numTS << " - " << 1 << utl::endlf;
                }
#endif
                remainingWork = 0.0;
            }
            else
            {
                // not done: update remainingWork
                remainingWork -= work;
            }

            // allocate (if requested)
            if (allocate && (work > 0.0))
            {
                for (i = 0; i < _numResources; ++i)
                {
                    // move cspans[i] out of the way
                    _cspans[i] = (CompositeSpan*)_cspans[i]->prev()->prev();
                    _resources[i]->allocate(t, overlapMax, multiple * _caps[i], _act, _prs[i]);
                }
            }

            // done?
            if (remainingWork == 0.0)
            {
                break;
            }
        }

        // bump t
        t = overlapMax + 1;

        // bump span
        if (t > span->max())
        {
            while (t > span->max())
            {
                span = span->next()->next();
            }

            // ran out of time?
            if (span->v0() == 0)
                goto fail;

            // update t
            t = utl::max(t, span->min());
        }

        // bump _cspans[]
        for (i = 0; i < _numResources; ++i)
        {
            CompositeSpan* cspan = _cspans[i];
            while (t > cspan->max())
            {
                cspan = (CompositeSpan*)cspan->next();
            }
            _cspans[i] = cspan;
        }
    }

    return;
fail:
    es = ef = int_t_max;
    return;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ESboundInt::findBackward(int& es, int& ef, bool allocate)
{
    bool firstExecution = true;
    double remainingWork = _act->processingTime();
    Resource** resourcesArray = _act->schedule()->resourcesArray();

    int t;

    // reference break-list
    const IntExp* breakList = _act->breakList();
    ASSERTD(breakList != nullptr);
    ASSERTD(breakList->size() > 0);
    const IntExpDomainRISC* domain = breakList->domainRISC();

    // find breakList span
    const IntSpan* span = domain->find(ef);
    if (span->v0() == 0)
        span = span->next();
    if (span->v0() == 0)
        goto fail;
    ef = max(ef, span->min());

    // initialize _cspans[]
    uint_t i, j;
    for (i = 0; i < _numResources; ++i)
    {
        Resource* res = _resources[i];
        ASSERTD(res->isA(CompositeResource));
        CompositeResource* cres = (CompositeResource*)res;
        const CompositeTimetableDomain* ttd = cres->timetable().domain();
        _cspans[i] = (CompositeSpan*)ttd->find(ef);
    }

#ifdef DEBUG_UNIT
    if (_act->id() == 472)
    {
        utl::cout << "--- findBackward() -------------------- " << utl::endlf;
    }
#endif

    // use maximum permitted capacity until the work is done
    t = ef;
    for (;;)
    {
#ifdef DEBUG_UNIT
        if (_act->id() == 472)
        {
            utl::cout << "remainingWork = " << remainingWork << utl::endlf;
        }
#endif

        // find end of overlap: current time window is defined by
        //     [overlapMin,t]
        // also, determine capacity multiplier
        int overlapMin = span->min();
        uint_t multiple = _maxMultiple;
        for (i = 0; i < _numResources; ++i)
        {
            CompositeSpan* cspan = _cspans[i];

#ifdef DEBUG_UNIT
            if (_act->id() == 472)
                utl::cout << "span = " << span->toString() << ", cspan = " << cspan->toString()
                          << utl::endlf;
#endif

            // limit overlapMax
            overlapMin = utl::max(overlapMin, cspan->min());

            // count remaining capacity in the CompositeSpan
            // (but don't count previously visited resources)
            uint_t remCap = 0;
            uint_t maxCap = (multiple == uint_t_max) ? uint_t_max : multiple * _caps[i];
            IntExpDomainIt* it;
            for (it = cspan->resIds()->begin(); !it->atEnd(); it->next())
            {
                int resId = it->get();
                Resource* res = resourcesArray[resId];
                ASSERTD(res->isA(DiscreteResource));
                DiscreteResource* dres = (DiscreteResource*)res;

                // already visited?
                if (dres->visited())
                {
                    continue;
                }

                dres->visited() = true;
                _visited[_numVisited++] = dres;

                // reached the maximum?
                if (++remCap == maxCap)
                {
                    break;
                }
            }
            delete it;

            multiple = utl::min(multiple, remCap / _caps[i]);
        }

        // clear _visited[]
        for (j = 0; j < _numVisited; ++j)
        {
            _visited[j]->visited() = false;
        }
        _numVisited = 0;

        // execute during [overlapMin,t] ?
        if (multiple >= _minMultiple)
        {
            if (firstExecution)
            {
                ef = t;
                firstExecution = false;
            }

            uint_t windowSize = (t - overlapMin + 1);
            double work = _baseWorkPerTS * multiple * windowSize;

#ifdef DEBUG_UNIT
            if (_act->id() == 472)
            {
                utl::cout << "ef = " << ef << ", t = " << t << ", windowSize = " << windowSize
                          << ", work = " << work << utl::endlf;
            }
#endif

            // done?
            if (work >= remainingWork)
            {
                // done: set ef
                double pct = remainingWork / work;
                double pct_x_ws = pct * windowSize;
                double pct_x_ws_floor = floor(pct_x_ws);
                uint_t numTS = (uint_t)pct_x_ws_floor;
                if ((pct_x_ws - pct_x_ws_floor) > 0.0001)
                {
                    ++numTS;
                }
                es = t - numTS + 1;
                overlapMin = es;
#ifdef DEBUG_UNIT
                if (_act->id() == 472)
                    utl::cout << "pct:" << pct << ", pct_x_ws:" << pct_x_ws
                              << ", pct_x_ws_floor:" << pct_x_ws_floor << utl::endlf << es << " = "
                              << t << " - " << numTS << " + 1" << utl::endlf;
#endif
                remainingWork = 0.0;
            }
            else
            {
                // not done: update remainingWork
                remainingWork -= work;
            }

            // allocate (if requested)
            if (allocate && (work > 0.0))
            {
                for (i = 0; i < _numResources; ++i)
                {
                    // move cspans[i] out of the way
                    _cspans[i] = (CompositeSpan*)_cspans[i]->next()->next();
                    _resources[i]->allocate(overlapMin, t, multiple * _caps[i], _act, _prs[i]);
                }
            }

            // done?
            if (remainingWork == 0.0)
            {
                break;
            }
        }

        // bump t
        t = overlapMin - 1;

        // bump span
        if (t < span->min())
        {
            while (t < span->min())
            {
                span = span->prev()->prev();
            }

            // ran out of time?
            if (span->v0() == 0)
                goto fail;

            // update t
            t = utl::min(t, span->max());
        }

        // bump _cspans[]
        for (i = 0; i < _numResources; ++i)
        {
            CompositeSpan* cspan = _cspans[i];
            while (t < cspan->min())
            {
                cspan = (CompositeSpan*)cspan->prev();
            }
            _cspans[i] = cspan;
        }
    }

    return;
fail:
    es = ef = int_t_min;
    return;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_END;
