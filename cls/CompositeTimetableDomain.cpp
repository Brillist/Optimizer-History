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

//////////////////////////////////////////////////////////////////////////////

#ifdef DEBUG
//#define DEBUG_UNIT
#endif

//////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;
CLP_NS_USE;

//////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(cls::CompositeTimetableDomain, utl::Object);

//////////////////////////////////////////////////////////////////////////////

CLS_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////

void
CompositeTimetableDomain::initialize(
    Schedule* schedule,
    const uint_set_t& resIds)
{
    ASSERTD(_head->next() == _tail);
    _schedule = schedule;
    Manager* mgr = schedule->manager();
    RevIntSpanCol::setManager(mgr);

    // _values[]
    _numValues = resIds.size();
    _values = new int[_numValues];
    int* ptr = _values;
    uint_set_t::const_iterator it;
    for (it = resIds.begin(); it != resIds.end(); ++it)
    {
        uint_t resId = *it;
        *ptr++ = resId;
    }

    RevIntSpanCol::clear();
    set(int_t_min + 1, int_t_max - 1, 0, 0);
}

//////////////////////////////////////////////////////////////////////////////

IntExp*
CompositeTimetableDomain::addCapExp(uint_t cap)
{
    ASSERTD(cap > 0);

    if ((cap < _capExpsSize) && (_capExps[cap] != nullptr))
    {
        _mgr->revSetIndirect(_capExpCounts, cap);
        ++_capExpCounts[cap];
        return _capExps[cap];
    }

    if (_capExpsSize <= cap)
    {
        IntExp* nullIntExpPtr = nullptr;
        uint_t zero = 0;
        utl::arrayGrow(
            _capExps,
            _capExpsSize,
            utl::max(K(1), ((size_t)cap + 1)),
            K(1024),
            &nullIntExpPtr);
        utl::arrayGrow(
            _capExpCounts,
            _capExpCountsSize,
            utl::max(K(1), ((size_t)cap + 1)),
            K(1024),
            &zero);
    }

    IntExp* capExp = new IntVar(_mgr);
    capExp->failOnEmpty() = false;
    _mgr->add(capExp);
    _mgr->revSetIndirect(_capExps, cap);
    _mgr->revSetIndirect(_capExpCounts, cap);
    _capExps[cap] = capExp;
    _capExpCounts[cap] = 1;

    int removeMin = int_t_min;
    int removeMax = int_t_min;
    IntSpan* span;
    for (span = _head->next(); span != _tail; span = span->next())
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

//////////////////////////////////////////////////////////////////////////////

void
CompositeTimetableDomain::remCapExp(uint_t cap)
{
    ASSERTD(_capExpsSize > cap);
    uint_t* capExpCounts = (uint_t*)_capExpCounts;
    _mgr->revSetIndirect(capExpCounts, cap);
    if (--_capExpCounts[cap] == 0)
    {
        _mgr->revSetIndirect(_capExps, cap);
        _capExps[cap] = nullptr;
    }
}

//////////////////////////////////////////////////////////////////////////////

/*void
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

    // find minSpan, maxSpan
    IntSpan* prev[CLP_INTSPAN_MAXDEPTH];
    IntSpan* next[CLP_INTSPAN_MAXDEPTH];
    CompositeSpan* minSpan = (CompositeSpan*)findPrev(min, prev);
    CompositeSpan* maxSpan;
    if (minSpan->max() >= max)
    {
        maxSpan = minSpan;
        prevToNext(minSpan, prev, next);
    }
    else
    {
        maxSpan = (CompositeSpan*)findNext(max, next);
    }

    // determine minEdge,maxEdge
    bool minEdge = (min == minSpan->min());
    bool maxEdge = (max == maxSpan->max());

    _events |= ef_range;

    // --- minSpan == maxSpan -------------------------------------------------

    if (minSpan == maxSpan)
    {
        CompositeSpan* span = minSpan;

        if (minEdge && maxEdge)
        {
            span->resIds()->add(resId);
        }
        else if (minEdge) // && !maxEdge
        {
            span->saveState(_mgr);
            span->min() = (max + 1);
            CompositeSpan* newSpan = newCS(min, max);
            newSpan->copyFlags(span);
            newSpan->resIds()->add(resId);
            insertAfter(newSpan, prev);
        }
        else if (maxEdge) // && !minEdge
        {
            findPrevForward(span->max() + 1, prev);
            span->saveState(_mgr);
            span->max() = (min - 1);
            CompositeSpan* newSpan = newCS(min, max);
            newSpan->copyFlags(span);
            newSpan->resIds()->add(resId);
            insertAfter(newSpan, prev);
        }
        else // !minEdge && !maxEdge => bisect
        {
            // create new spans
            CompositeSpan* newLHS = newCS(span->min(), min - 1);
            newLHS->copyFlags(span);
            CompositeSpan* newRHS = newCS(max + 1, span->max());
            newRHS->copyFlags(span);

            // modify extant span
            span->saveState(_mgr);
            span->min() = min;
            span->max() = max;
            span->resIds()->add(resId);

            // add a new span on each side
            insertAfter(newLHS, prev);
            findPrevForward(max + 1, prev);
            insertAfter(newRHS, prev);
        }
#ifdef DEBUG_UNIT
        validate();
#endif
        return;
    }

    // --- minSpan != maxSpan -------------------------------------------------

    // left side
    if (!minEdge)
    {
        int minSpanMax = minSpan->max();
        CompositeSpan* newSpan = newCS(min, minSpanMax);
        newSpan->copyFlags(minSpan);

        findPrevForward(minSpanMax + 1, prev);
        insertAfter(newSpan, prev);
        minSpan->saveState(_mgr);
        minSpan->max() = (min - 1);
        minSpan = newSpan;
    }

    // right side
    if (!maxEdge)
    {
        int maxSpanMin = maxSpan->min();
        CompositeSpan* newSpan = newCS(maxSpanMin, max);
        newSpan->copyFlags(maxSpan);
        newSpan->resIds()->add(resId);

        findPrevForward(maxSpanMin, prev);
        insertAfter(newSpan, prev);
        maxSpan->saveState(_mgr);
        maxSpan->min() = (max + 1);
        maxSpan = (CompositeSpan*)newSpan->prev();
    }

    // add to intermediate spans
    CompositeSpan* span = minSpan;
    for (;;)
    {
        span->resIds()->add(resId);
        if (span == maxSpan) break;
        span = (CompositeSpan*)span->next();
    }

#ifdef DEBUG_UNIT
    validate();
#endif
}*/

//////////////////////////////////////////////////////////////////////////////

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
    CompositeSpan* ttSpan = (CompositeSpan*)findPrev(min, prev);

    // add capacity ...
    int t = min;
    int e;
    while (!ttSpan->isTail() && (t <= max))
    {
        // find end of modified region (e)
        e = utl::min(max, ttSpan->max());

        bool minEdge = (t == ttSpan->min());
        bool maxEdge = (e == ttSpan->max());
        IntExpDomainAR* resIds = ttSpan->resIds()->clone();

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
            ttSpan->min() = (e + 1);
            _mgr->revAllocate(resIds);
            CompositeSpan* newSpan = newCS(t, e, resIds);
            insertAfter(newSpan, prev);
        }
        else if (maxEdge)
        {
            findPrevForward(e + 1, prev);
            ttSpan->saveState(_mgr);
            ttSpan->max() = (t - 1);
            _mgr->revAllocate(resIds);
            CompositeSpan* newSpan = newCS(t, e, resIds);
            insertAfter(newSpan, prev);
        }
        else
        {
            // "squish" ttSpan
            int spanMin = ttSpan->min();
            int spanMax = ttSpan->max();
            ttSpan->saveState(_mgr);
            ttSpan->min() = t;
            ttSpan->max() = e;

            // create resIds for (left, right) spans
            IntExpDomainAR* lhsResIds = ttSpan->resIds();
            IntExpDomainAR* rhsResIds = lhsResIds->clone();
            _mgr->revAllocate(rhsResIds);

            // middle
            _mgr->revAllocate(resIds);
            ttSpan->setResIds(resIds);

            // left
            CompositeSpan* newLHS = newCS(spanMin, t - 1, lhsResIds);
            insertAfter(newLHS, prev);

            // right
            findPrevForward(e + 1, prev);
            CompositeSpan* newRHS = newCS(e + 1, spanMax, rhsResIds);
            insertAfter(newRHS, prev);
        }

        while (prev[0]->canMergeWith(prev[0]->next()))
        {
            prev[0]->saveState(_mgr);
            prev[0]->max() = prev[0]->next()->max();
            eclipseNext(prev);
        }

        // adjust ttSpan
        t = e + 1;
        ttSpan = (CompositeSpan*)findPrevForward(t, prev);
        // need to back up?
        if (t < ttSpan->min())
        {
            ttSpan = (CompositeSpan*)findPrev(t, prev);
        }
    }

#ifdef DEBUG_UNIT
    validate();
#endif
}

//////////////////////////////////////////////////////////////////////////////

void
CompositeTimetableDomain::allocate(
    int min,
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
    if ((act != nullptr) && act->isA(IntActivity)) intact = (IntActivity*)act;

    // ttSpan iterator
    IntSpan* prev[CLP_INTSPAN_MAXDEPTH];
    CompositeSpan* ttSpan = (CompositeSpan*)findPrev(min, prev);

    // blSpan iterator
    const IntSpan* blSpan
        = (breakList == nullptr) ? _dummyIntSpan
                              : breakList->domainRISC()->find(min);

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
        IntExpDomainAR* resIds = ttSpan->resIds();
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
                ttSpan->min() = (e + 1);
                _mgr->revAllocate(resIds);
                CompositeSpan* newSpan = newCS(t, e, resIds);
                insertAfter(newSpan, prev);
            }
            else if (maxEdge)
            {
                findPrevForward(e + 1, prev);
                ttSpan->saveState(_mgr);
                ttSpan->max() = (t - 1);
                _mgr->revAllocate(resIds);
                CompositeSpan* newSpan = newCS(t, e, resIds);
                insertAfter(newSpan, prev);
            }
            else
            {
                // "squish" ttSpan
                int spanMin = ttSpan->min();
                int spanMax = ttSpan->max();
                ttSpan->saveState(_mgr);
                ttSpan->min() = t;
                ttSpan->max() = e;

                // create resIds for (left, right) spans
                IntExpDomainAR* lhsResIds = ttSpan->resIds();
                IntExpDomainAR* rhsResIds = lhsResIds->clone();
                _mgr->revAllocate(rhsResIds);

                // middle
                _mgr->revAllocate(resIds);
                ttSpan->setResIds(resIds);

                // left
                CompositeSpan* newLHS = newCS(spanMin, t - 1, lhsResIds);
                insertAfter(newLHS, prev);

                // right
                findPrevForward(e + 1, prev);
                CompositeSpan* newRHS = newCS(e + 1, spanMax, rhsResIds);
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
            prev[0]->max() = prev[0]->next()->max();
            eclipseNext(prev);
        }

        // adjust blSpan
        t = e + 1;
        if (blSpan->v0() == 0)
        {
            blSpan = blSpan->next();
            if (blSpan->v0() == 0) blSpan = blSpan->next();
        }
        while (blSpan->max() < t) blSpan = blSpan->next()->next();
        t = utl::max(t, blSpan->min());

        // adjust ttSpan
        ttSpan = (CompositeSpan*)findPrevForward(t, prev);
        // need to back up?
        if (t < ttSpan->min())
        {
            ttSpan = (CompositeSpan*)findPrev(t, prev);
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

//////////////////////////////////////////////////////////////////////////////

CompositeSpan*
CompositeTimetableDomain::newCS(
    int min,
    int max,
    IntExpDomainAR* resIds,
    uint_t level)
{
    ASSERTD(_values != nullptr);

    if (resIds == nullptr)
    {
        resIds
            = new IntExpDomainAR(
                _mgr,
                _numValues,
                _values,
                false,
                true);
        _mgr->revAllocate(resIds);
    }

    CompositeSpan* span
        = new CompositeSpan(
            min,
            max,
            resIds,
            level);

    _mgr->revAllocate(span);
    return span;
}

//////////////////////////////////////////////////////////////////////////////

IntSpan*
CompositeTimetableDomain::newIntSpan(
    int min,
    int max,
    uint_t,             // v0 (unused)
    uint_t,             // v1 (unused)
    uint_t level)
{
    ASSERTD(_values != nullptr);

    IntExpDomainAR* resIds
        = new IntExpDomainAR(
            _mgr,
            _numValues,
            _values,
            false,
            true);
    _mgr->revAllocate(resIds);

    CompositeSpan* span
        = new CompositeSpan(
            min,
            max,
            resIds,
            level);

    return span;
}

//////////////////////////////////////////////////////////////////////////////

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

//////////////////////////////////////////////////////////////////////////////

void
CompositeTimetableDomain::deInit()
{
    delete _dummyIntSpan;
    delete [] _values;
    delete [] _capExps;
    delete [] _capExpCounts;
}

//////////////////////////////////////////////////////////////////////////////

bool
CompositeTimetableDomain::allocate(
    IntExpDomainAR* resIds,
    uint_t cap,
    IntActivity* act,
    const PreferredResources* pr,
    int min,
    int max,
    utl::uint_t resId,
    bool updateDiscrete)
{
    ASSERTD(cap > 0);

    // sanity check params
#ifdef DEBUG
    ASSERT((min == int_t_max) == (max == int_t_max));
    if (resId != uint_t_max) ASSERT(cap == 1);
#endif

    // can't perform the allocation?
    if ((resIds->size() < cap)
        || (((resId != uint_t_max) && !resIds->has(resId))))
    {
        return false;
    }

    // preferred resources iterator
    std::vector<uint_t>::const_iterator prIt, prLim;
    if (pr != nullptr)
    {
        prIt = pr->resIds().begin();
        prLim = pr->resIds().end();
    }

    uint_t oldCap = resIds->size();
    Resource** resourcesArray = _schedule->resourcesArray();
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

        if ((min == int_t_max) || (act == nullptr)) continue;

        // update composite-allocations in act
        act->addAllocation(curResId, min, max);

        // reference discrete resource
        Resource* res = resourcesArray[curResId];
        ASSERTD(res->isA(DiscreteResource));
        DiscreteResource* dres = (DiscreteResource*)res;

        // allocate capacity from discrete resource timetable
        // (to prevent overallocation and for purpose of costing)
        if (updateDiscrete)
        {
            dres->allocate(min, max, 100, nullptr, false);
        }

        // remove cur-res from other composite timetables
        const uint_vector_t& crIds = dres->crIds();
        ASSERTD(crIds.size() >= 1);
        if (crIds.size() == 1) continue;
        uint_vector_t::const_iterator it;
        for (it = crIds.begin(); it != crIds.end(); ++it)
        {
            uint_t crId = *it;
            Resource* res = resourcesArray[crId];
            ASSERTD(res->isA(CompositeResource));
            CompositeResource* cres = (CompositeResource*)res;
            // skip over self...
            if (cres->timetable().domain() == this) continue;
            // allocate curResId in cres
            cres->allocate(min, max, 1, nullptr, nullptr, nullptr, curResId, false);
        }
    }

    if (min == int_t_max) return true;

    // update cap-exps
    if (_capExpsSize > 0)
    {
        uint_t newCap = resIds->size();
        uint_t lim = utl::min(oldCap, (uint_t)_capExpsSize - 1);
        for (uint_t cap = newCap + 1; cap <= lim; ++cap)
        {
            IntExp* capExp = _capExps[cap];
            if (capExp == nullptr) continue;
            capExp->remove(min, max);
        }
    }
    return true;
}

//////////////////////////////////////////////////////////////////////////////

CLS_NS_END;
