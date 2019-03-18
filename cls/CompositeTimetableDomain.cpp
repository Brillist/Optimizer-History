#include "libcls.h"
#include <clp/IntExpDomainRISC.h>
#include <clp/IntVar.h>
#include <clp/Manager.h>
#include "CompositeResource.h"
#include "CompositeTimetableDomain.h"
#include "DiscreteResource.h"
#include "IntActivity.h"
#include "Resource.h"
#include "Schedule.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef DEBUG
//#define DEBUG_UNIT
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;
CLP_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(cls::CompositeTimetableDomain);

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

void
CompositeTimetableDomain::initialize(Schedule* schedule, const uint_set_t& resIds)
{
    ASSERTD(_head->next() == _tail);
    _schedule = schedule;
    auto mgr = schedule->manager();
    RevIntSpanCol::setManager(mgr);

    // _values[]
    _numValues = resIds.size();
    _values = new int[_numValues];
    int* ptr = _values;
    for (auto resId : resIds)
    {
        *ptr++ = resId;
    }

    RevIntSpanCol::clear();
    set(int_t_min + 1, int_t_max - 1, 0, 0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

IntExp*
CompositeTimetableDomain::addCapExp(uint_t cap)
{
    ASSERTD(cap > 0);

    // we already have an expression for cap?
    if ((cap < _capExpsSize) && (_capExps[cap] != nullptr))
    {
        // increment its count and return it
        _mgr->revSetIndirect(_capExpCounts, cap);
        ++_capExpCounts[cap];
        return _capExps[cap];
    }

    // need to grow _capExps[], _capExpCounts[] ?
    if (_capExpsSize <= cap)
    {
        IntExp* nullIntExpPtr = nullptr;
        uint_t zero = 0;
        utl::arrayGrow(_capExps, _capExpsSize, utl::max((size_t)16, ((size_t)cap + 1)), size_t_max,
                       &nullIntExpPtr);
        utl::arrayGrow(_capExpCounts, _capExpCountsSize, utl::max((size_t)16, ((size_t)cap + 1)),
                       size_t_max, &zero);
    }

    auto capExp = new IntVar(_mgr);
    capExp->setFailOnEmpty(false);
    _mgr->add(capExp);
    _mgr->revSetIndirect(_capExps, cap);
    _mgr->revSetIndirect(_capExpCounts, cap);
    _capExps[cap] = capExp;
    _capExpCounts[cap] = 1;

    int removeMin = int_t_min;
    int removeMax = int_t_min;
    for (auto span = _head->next(); span != _tail; span = span->next())
    {
        if (span->capacity() < cap)
        {
            if (span->min() == (removeMax + 1))
            {
                removeMax = span->max();
            }
            else
            {
                capExp->remove(removeMin, removeMax);
                removeMin = span->min();
                removeMax = span->max();
            }
        }
    }
    capExp->remove(removeMin, removeMax);

    return capExp;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
CompositeTimetableDomain::remCapExp(uint_t cap)
{
    ASSERTD(_capExpsSize > cap);
    uint_t* capExpCounts = _capExpCounts;
    _mgr->revSetIndirect(capExpCounts, cap);
    if (--_capExpCounts[cap] == 0)
    {
        _mgr->revSetIndirect(_capExps, cap);
        _capExps[cap] = nullptr;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
CompositeTimetableDomain::add(int min, int max, uint_t resId)
{
    ASSERTD(_mgr != nullptr);

    min = utl::max(min, int_t_min + 1);
    max = utl::min(max, int_t_max - 1);

    if (min > max)
    {
        return;
    }

    // --- initialize ---------------------------------------------------------

#ifdef DEBUG_UNIT
    validate();
#endif

    saveState();

    // range event
    _events |= ef_range;

    // ttSpan iterator
    IntSpan* prev[CLP_INTSPAN_MAXDEPTH];
    auto ttSpan = utl::cast<CompositeSpan>(findPrev(min, prev));

    // add capacity ...
    int t = min;
    int e;
    while (!ttSpan->isTail() && (t <= max))
    {
        // find end of modified region (e)
        e = utl::min(max, ttSpan->max());

        bool minEdge = (t == ttSpan->min());
        bool maxEdge = (e == ttSpan->max());
        auto resIds = ttSpan->resIds()->clone();

        // add resId during overlap
        resIds->add(resId);

        if (minEdge && maxEdge)
        {
            _mgr->revAllocate(resIds);
            ttSpan->saveState(_mgr);
            ttSpan->setResIds(resIds);
        }
        else if (minEdge)
        {
            ttSpan->saveState(_mgr);
            ttSpan->setMin(e + 1);
            _mgr->revAllocate(resIds);
            auto newSpan = newCS(t, e, resIds);
            insertAfter(newSpan, prev);
        }
        else if (maxEdge)
        {
            findPrevForward(e + 1, prev);
            ttSpan->saveState(_mgr);
            ttSpan->setMax(t - 1);
            _mgr->revAllocate(resIds);
            auto newSpan = newCS(t, e, resIds);
            insertAfter(newSpan, prev);
        }
        else
        {
            // "squish" ttSpan
            int spanMin = ttSpan->min();
            int spanMax = ttSpan->max();
            ttSpan->saveState(_mgr);
            ttSpan->setMin(t);
            ttSpan->setMax(e);

            // create resIds for (left, right) spans
            auto lhsResIds = ttSpan->resIds();
            auto rhsResIds = lhsResIds->clone();
            _mgr->revAllocate(rhsResIds);

            // middle
            _mgr->revAllocate(resIds);
            ttSpan->setResIds(resIds);

            // left
            auto newLHS = newCS(spanMin, t - 1, lhsResIds);
            insertAfter(newLHS, prev);

            // right
            findPrevForward(e + 1, prev);
            auto newRHS = newCS(e + 1, spanMax, rhsResIds);
            insertAfter(newRHS, prev);
        }

        while (prev[0]->canMergeWith(prev[0]->next()))
        {
            prev[0]->saveState(_mgr);
            prev[0]->setMax(prev[0]->next()->max());
            eclipseNext(prev);
        }

        // adjust ttSpan
        t = e + 1;
        ttSpan = utl::cast<CompositeSpan>(findPrevForward(t, prev));
        // need to back up?
        if (t < ttSpan->min())
        {
            ttSpan = utl::cast<CompositeSpan>(findPrev(t, prev));
        }
    }

#ifdef DEBUG_UNIT
    validate();
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
CompositeTimetableDomain::allocate(int min,
                                   int max,
                                   uint_t cap,
                                   Activity* act,
                                   const PreferredResources* pr,
                                   const IntExp* breakList,
                                   uint_t resId,
                                   bool updateDiscrete)
{
    ASSERTD(_mgr != nullptr);

    min = utl::max(min, int_t_min + 1);
    max = utl::min(max, int_t_max - 1);

    if ((min > max) || (cap == 0))
    {
        return;
    }

    // --- initialize ---------------------------------------------------------

#ifdef DEBUG_UNIT
    validate();
#endif

    saveState();

    // range event
    _events |= ef_range;

    // reference int-act
    IntActivity* intact = nullptr;
    if (act != nullptr)
    {
        breakList = act->breakList();
    }
    if ((act != nullptr) && act->isA(IntActivity))
        intact = utl::cast<IntActivity>(act);

    // ttSpan iterator
    IntSpan* prev[CLP_INTSPAN_MAXDEPTH];
    auto ttSpan = utl::cast<CompositeSpan>(findPrev(min, prev));

    // blSpan iterator
    const IntSpan* blSpan =
        (breakList == nullptr) ? _dummyIntSpan : breakList->domainRISC()->find(min);

    // should not be in break at min
    if (blSpan->v0() == 0)
    {
        _events |= ef_empty;
        return;
    }

    // allocate ...
    int t = min;
    int e;
    while ((blSpan->v0() == 1) && !ttSpan->isTail() && (t <= max))
    {
        // find end of allocation (e)
        e = utl::min(max, ttSpan->max());
        e = utl::min(e, blSpan->max());

        bool minEdge = (t == ttSpan->min());
        bool maxEdge = (e == ttSpan->max());
        auto resIds = ttSpan->resIds();
        resIds = resIds->clone();

        // allocate overlap
        // (but first - move blSpan out of the way)
        blSpan = blSpan->prev()->prev();
        if (allocate(resIds, cap, intact, pr, t, e, resId, updateDiscrete))
        {
            if (minEdge && maxEdge)
            {
                _mgr->revAllocate(resIds);
                ttSpan->saveState(_mgr);
                ttSpan->setResIds(resIds);
            }
            else if (minEdge)
            {
                ttSpan->saveState(_mgr);
                ttSpan->setMin(e + 1);
                _mgr->revAllocate(resIds);
                auto newSpan = newCS(t, e, resIds);
                insertAfter(newSpan, prev);
            }
            else if (maxEdge)
            {
                findPrevForward(e + 1, prev);
                ttSpan->saveState(_mgr);
                ttSpan->setMax(t - 1);
                _mgr->revAllocate(resIds);
                auto newSpan = newCS(t, e, resIds);
                insertAfter(newSpan, prev);
            }
            else
            {
                // "squish" ttSpan
                int spanMin = ttSpan->min();
                int spanMax = ttSpan->max();
                ttSpan->saveState(_mgr);
                ttSpan->setMin(t);
                ttSpan->setMax(e);

                // create resIds for (left, right) spans
                auto lhsResIds = ttSpan->resIds();
                auto rhsResIds = lhsResIds->clone();
                _mgr->revAllocate(rhsResIds);

                // middle
                _mgr->revAllocate(resIds);
                ttSpan->setResIds(resIds);

                // left
                auto newLHS = newCS(spanMin, t - 1, lhsResIds);
                insertAfter(newLHS, prev);

                // right
                findPrevForward(e + 1, prev);
                auto newRHS = newCS(e + 1, spanMax, rhsResIds);
                insertAfter(newRHS, prev);
            }
        }
        else
        {
            delete resIds;
            if (resId == uint_t_max)
            {
                _events |= ef_empty;
                return;
            }
        }

        while (prev[0]->canMergeWith(prev[0]->next()))
        {
            prev[0]->saveState(_mgr);
            prev[0]->setMax(prev[0]->next()->max());
            eclipseNext(prev);
        }

        // adjust blSpan
        t = e + 1;
        if (blSpan->v0() == 0)
        {
            blSpan = blSpan->next();
            if (blSpan->v0() == 0)
                blSpan = blSpan->next();
        }
        while (blSpan->max() < t)
            blSpan = blSpan->next()->next();
        t = utl::max(t, blSpan->min());

        // adjust ttSpan
        ttSpan = utl::cast<CompositeSpan>(findPrevForward(t, prev));
        // need to back up?
        if (t < ttSpan->min())
        {
            ttSpan = utl::cast<CompositeSpan>(findPrev(t, prev));
        }
    }

    // e should be (max) otherwise allocation failed
    if (e != max)
    {
        _events |= ef_empty;
        return;
    }

#ifdef DEBUG_UNIT
    validate();
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CompositeSpan*
CompositeTimetableDomain::newCS(int min, int max, IntExpDomainAR* resIds, uint_t level)
{
    ASSERTD(_values != nullptr);

    if (resIds == nullptr)
    {
        resIds = new IntExpDomainAR(_mgr, _numValues, _values, false, true);
        _mgr->revAllocate(resIds);
    }

    CompositeSpan* span = new CompositeSpan(min, max, resIds, level);

    _mgr->revAllocate(span);
    return span;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

IntSpan*
CompositeTimetableDomain::newIntSpan(int min,
                                     int max,
                                     uint_t, // v0 (unused)
                                     uint_t, // v1 (unused)
                                     uint_t level)
{
    ASSERTD(_values != nullptr);

    IntExpDomainAR* resIds = new IntExpDomainAR(_mgr, _numValues, _values, false, true);
    _mgr->revAllocate(resIds);

    CompositeSpan* span = new CompositeSpan(min, max, resIds, level);

    return span;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
CompositeTimetableDomain::init()
{
    // dummy IntSpan (for (act == nullptr) case)
    _dummyIntSpan = new IntSpan(int_t_min, int_t_max, 1, 0);
    _dummyIntSpan->setLevel(1);
    _dummyIntSpan->setPrev(_dummyIntSpan);
    _dummyIntSpan->setNext(0, _dummyIntSpan);

    _schedule = nullptr;
    _values = nullptr;
    _numValues = 0;
    _capExps = nullptr;
    _capExpCounts = nullptr;
    _capExpsSize = 0;
    _capExpCountsSize = 0;
    _events = 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
CompositeTimetableDomain::deInit()
{
    delete _dummyIntSpan;
    delete[] _values;
    delete[] _capExps;
    delete[] _capExpCounts;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool
CompositeTimetableDomain::allocate(IntExpDomainAR* resIds,
                                   uint_t cap,
                                   IntActivity* act,
                                   const PreferredResources* pr,
                                   int min,
                                   int max,
                                   uint_t resId,
                                   bool updateDiscrete)
{
    // sanity check params
    ASSERTD(cap > 0);
    ASSERTD((min == int_t_max) == (max == int_t_max));
    ASSERTD((resId == uint_t_max) || (cap == 1));

    // can't perform the allocation?
    if ((resIds->size() < cap) || (((resId != uint_t_max) && !resIds->has(resId))))
    {
        return false;
    }

    // preferred resources iterator
    uint_vector_t::const_iterator prIt, prLim;
    if (pr != nullptr)
    {
        prIt = pr->resIds().begin();
        prLim = pr->resIds().end();
    }

    uint_t oldCap = resIds->size();
    auto resourcesArray = _schedule->resourcesArray();
    while (cap-- > 0)
    {
        // choose a preferred resource?
        uint_t curResId;
        if (resId == uint_t_max)
        {
            curResId = resIds->min();
            while ((pr != nullptr) && (prIt != prLim))
            {
                uint_t prefId = *prIt;
                ++prIt;
                if (resIds->has(prefId))
                {
                    curResId = prefId;
                    break;
                }
            }
        }
        else
        {
            curResId = resId;
        }
        resIds->remove(curResId);

        if ((min == int_t_max) || (act == nullptr))
            continue;

        // update composite-allocations in act
        act->addAllocation(curResId, min, max);

        // reference discrete resource
        auto res = resourcesArray[curResId];
        auto dres = utl::cast<DiscreteResource>(res);

        // allocate capacity from discrete resource timetable
        // (to prevent overallocation and for purpose of costing)
        if (updateDiscrete)
        {
            dres->allocate(min, max, 100, nullptr, false);
        }

        // remove cur-res from other composite timetables
        const auto& crIds = dres->crIds();
        ASSERTD(crIds.size() >= 1);
        if (crIds.size() == 1)
            continue;
        for (auto crId : crIds)
        {
            auto cres = utl::cast<CompositeResource>(resourcesArray[crId]);
            // skip over self...
            if (cres->timetable().domain() == this)
                continue;
            // allocate curResId in cres
            cres->allocate(min, max, 1, nullptr, nullptr, nullptr, curResId, false);
        }
    }

    if (min == int_t_max)
        return true;

    // update cap-exps
    if (_capExpsSize > 0)
    {
        uint_t newCap = resIds->size();
        uint_t lim = utl::min(oldCap, (uint_t)_capExpsSize - 1);
        for (uint_t cap = newCap + 1; cap <= lim; ++cap)
        {
            auto capExp = _capExps[cap];
            if (capExp == nullptr)
                continue;
            capExp->remove(min, max);
        }
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_END;
