#include "libcls.h"
#include <clp/IntVar.h>
#include <clp/Manager.h>
#include "DiscreteTimetableDomain.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef DEBUG
//#define DEBUG_UNIT
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;
CLP_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(cls::DiscreteTimetableDomain, utl::Object);

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

void
DiscreteTimetableDomain::initialize(Manager* mgr)
{
    ASSERTD(_head->next() == _tail);
    RevIntSpanCol::setManager(mgr);
    set(int_t_min + 1, int_t_max - 1, 0, 0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

IntExp*
DiscreteTimetableDomain::addCapExp(uint_t cap)
{
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
        utl::arrayGrow(_capExps, _capExpsSize, utl::max(K(1), ((size_t)cap + 1)), K(1024),
                       &nullIntExpPtr);
        utl::arrayGrow(_capExpCounts, _capExpCountsSize, utl::max(K(1), ((size_t)cap + 1)), K(1024),
                       &zero);
    }

    IntExp* capExp = new IntVar(_mgr);
    capExp->failOnEmpty() = false;
    _mgr->add(capExp);
    _mgr->revSetIndirect(_capExps, cap);
    _mgr->revSetIndirect(_capExpCounts, cap);
    _capExps[cap] = capExp;
    _capExpCounts[cap] = 1;

    int removeBegin = int_t_min;
    int removeEnd = int_t_min;
    IntSpan* span;
    for (span = _head->next(); span != _tail; span = span->next())
    {
        if (span->capacity() < cap)
        {
            if (span->min() == (removeEnd + 1))
            {
                removeEnd = span->max();
            }
            else
            {
                capExp->remove(removeBegin, removeEnd);
                removeBegin = span->min();
                removeEnd = span->max();
            }
        }
    }
    capExp->remove(removeBegin, removeEnd);

    return capExp;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
DiscreteTimetableDomain::remCapExp(uint_t cap)
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

////////////////////////////////////////////////////////////////////////////////////////////////////

utl::uint_t
DiscreteTimetableDomain::add(int min, int max, int v0, int v1)
{
    ASSERTD(_mgr != nullptr);

    if ((v0 == 0) && (v1 == 0))
    {
        return 0;
    }

    min = utl::max(min, int_t_min + 2);
    max = utl::min(max, int_t_max - 2);

    if (min > max)
    {
        return 0;
    }

    // --- initialize ---------------------------------------------------------

#ifdef DEBUG_UNIT
    validate();
#endif

    uint_t minCap = int_t_max;
    saveState();

    // find minSpan, maxSpan
    IntSpan* prev[CLP_INTSPAN_MAXDEPTH];
    IntSpan* next[CLP_INTSPAN_MAXDEPTH];
    IntSpan* minSpan = findPrev(min, prev);
    IntSpan* maxSpan;
    if (minSpan->max() >= max)
    {
        maxSpan = minSpan;
        prevToNext(minSpan, prev, next);
    }
    else
    {
        maxSpan = findNext(max, next);
    }
    IntSpan* prev0 = prev[0];
    IntSpan* next0 = next[0];

    // determine minEdge,minMerge
    int minv0 = (int)minSpan->v0();
    int minv1 = (int)minSpan->v1();
    int newminv0 = minv0 + v0;
    int newminv1 = minv1 + v1;
    if ((newminv1 < newminv0) && (v1 < 0))
        newminv1 = newminv0;
    minCap = utl::min((int)minCap, newminv1 - newminv0);
    bool minEdge, minMerge;
    if (min == minSpan->min())
    {
        minEdge = true;
        minMerge = (((int)prev0->v0() == newminv0) && ((int)prev0->v1() == newminv1));
    }
    else
    {
        minEdge = minMerge = false;
    }

    // determine maxEdge,maxMerge
    int maxv0 = (int)maxSpan->v0();
    int maxv1 = (int)maxSpan->v1();
    int newmaxv0 = maxv0 + v0;
    int newmaxv1 = maxv1 + v1;
    if ((newmaxv1 < newmaxv0) && (v1 < 0))
        newmaxv1 = newmaxv0;
    minCap = utl::min((int)minCap, newmaxv1 - newmaxv0);
    bool maxEdge, maxMerge;
    if (max == maxSpan->max())
    {
        maxEdge = true;
        maxMerge = (((int)next0->v0() == newmaxv0) && ((int)next0->v1() == newmaxv1));
    }
    else
    {
        maxEdge = maxMerge = false;
    }

    _events |= ef_range;
    if ((newminv0 > newminv1) || (newmaxv0 > newmaxv1))
    {
        _events |= ef_empty;
        return minCap;
    }

    minSpan->saveState(_mgr);

    // --- minSpan == maxSpan -------------------------------------------------

    if (minSpan == maxSpan)
    {
        IntSpan* span = minSpan;
        uint_t newv0 = newminv0;
        uint_t newv1 = newminv1;

        if (_capExpsSize > 0)
        {
            uint_t oldCap = span->capacity();
            uint_t newCap = newv1 - newv0;
            uint_t lim = utl::min(oldCap, (uint_t)_capExpsSize - 1);
            for (uint_t cap = newCap + 1; cap <= lim; ++cap)
            {
                IntExp* capExp = _capExps[cap];
                if (capExp != nullptr)
                {
                    capExp->remove(min, max);
                }
            }
        }

        if (minEdge && maxEdge)
        {
            if (minMerge && maxMerge)
            {
                // prev eclipses span and next
                findNextForward(next0->max(), next);
                link(prev, next);
                prev0->max() = next0->max();
            }
            else if (minMerge)
            {
                link(prev, next);
                prev0->max() = span->max();
            }
            else if (maxMerge)
            {
                link(prev, next);
                next0->min() = span->min();
            }
            else
            {
                // don't merge on either side
                span->v0() = newv0;
                span->v1() = newv1;
            }
        }
        else if (minEdge) // && !maxEdge
        {
            span->min() = (max + 1);
            if (minMerge)
            {
                prev0->saveState(_mgr);
                prev0->max() = max;
            }
            else
            {
                IntSpan* newSpan = new IntSpan(min, max, newv0, newv1);
                _mgr->revAllocate(newSpan);
                insertAfter(newSpan, prev);
            }
        }
        else if (maxEdge) // && !minEdge
        {
            if (maxMerge)
            {
                next0->saveState(_mgr);
                next0->min() = min;
                span->max() = (min - 1);
            }
            else
            {
                findPrevForward(span->max() + 1, prev);
                span->max() = (min - 1);
                IntSpan* newSpan = new IntSpan(min, max, newv0, newv1);
                _mgr->revAllocate(newSpan);
                insertAfter(newSpan, prev);
            }
        }
        else // !minEdge && !maxEdge => bisect
        {
            uint_t sv0 = span->v0();
            uint_t sv1 = span->v1();
            int spanMin = span->min();
            int spanMax = span->max();
            span->min() = min;
            span->max() = max;
            span->v0() = newminv0;
            span->v1() = newminv1;

            // insert 2 new spans
            IntSpan* newLHS = new IntSpan(spanMin, min - 1, sv0, sv1);
            _mgr->revAllocate(newLHS);
            insertAfter(newLHS, prev);
            findPrevForward(max + 1, prev);
            IntSpan* newRHS = new IntSpan(max + 1, spanMax, sv0, sv1);
            _mgr->revAllocate(newRHS);
            insertAfter(newRHS, prev);
        }
#ifdef DEBUG_UNIT
        validate();
#endif
        return minCap;
    }

    // --- minSpan != maxSpan -------------------------------------------------

    maxSpan->saveState(_mgr);

    // left side
    if (_capExpsSize > 0)
    {
        uint_t oldCap = (minv1 - minv0);
        uint_t newCap = (newminv1 - newminv0);
        uint_t lim = utl::min(oldCap, (uint_t)_capExpsSize - 1);
        for (uint_t cap = newCap + 1; cap <= lim; ++cap)
        {
            IntExp* capExp = _capExps[cap];
            if (capExp != nullptr)
            {
                capExp->remove(min, minSpan->max());
            }
        }
    }
    if (minEdge)
    {
        if (minMerge)
        {
            prev0->saveState(_mgr);
            prev0->max() = minSpan->max();
            eclipseNext(prev);
            minSpan = minSpan->next();
        }
    }
    else
    {
        int minSpanMax = minSpan->max();
        IntSpan* newSpan = new IntSpan(min, minSpanMax, newminv0, newminv1);
        _mgr->revAllocate(newSpan);
        findPrevForward(minSpanMax + 1, prev);
        insertAfter(newSpan, prev);
        minSpan->max() = (min - 1);
        minSpan = newSpan->next();
    }

    // right side
    if (_capExpsSize > 0)
    {
        uint_t oldCap = (maxv1 - maxv0);
        uint_t newCap = (newmaxv1 - newmaxv0);
        uint_t lim = utl::min(oldCap, (uint_t)_capExpsSize - 1);
        for (uint_t cap = newCap + 1; cap <= lim; ++cap)
        {
            IntExp* capExp = _capExps[cap];
            if (capExp != nullptr)
            {
                capExp->remove(maxSpan->min(), max);
            }
        }
    }
    if (maxEdge)
    {
        if (maxMerge)
        {
            findPrevForward(maxSpan->min(), prev);
            next0->saveState(_mgr);
            next0->min() = maxSpan->min();
            eclipseNext(prev);
            maxSpan = maxSpan->prev();
        }
    }
    else
    {
        int maxSpanMin = maxSpan->min();
        IntSpan* newSpan = new IntSpan(maxSpanMin, max, newmaxv0, newmaxv1);
        _mgr->revAllocate(newSpan);
        findPrevForward(maxSpanMin, prev);
        insertAfter(newSpan, prev);
        maxSpan->min() = (max + 1);
        maxSpan = newSpan->prev();
    }

    // add to intermediate spans
    IntSpan* span = minSpan;
    int lim = maxSpan->max();
    while (span->max() <= lim)
    {
        span->saveState(_mgr);
        uint_t oldCap = span->capacity();
        int spanV0 = (int)span->v0() + v0;
        int spanV1 = (int)span->v1() + v1;
        if ((spanV1 < spanV0) && (v1 < 0))
            spanV1 = spanV0;
        span->v0() = spanV0;
        span->v1() = spanV1;
        if (span->v0() > span->v1())
        {
            _events |= ef_empty;
            return minCap;
        }
        uint_t newCap = span->capacity();
        minCap = utl::min(minCap, newCap);

        if (_capExpsSize > 0)
        {
            uint_t lim = utl::min(oldCap, (uint_t)_capExpsSize - 1);
            for (uint_t cap = newCap + 1; cap <= lim; ++cap)
            {
                IntExp* capExp = _capExps[cap];
                if (capExp != nullptr)
                {
                    capExp->remove(span->min(), span->max());
                }
            }
        }

        span = span->next();
    }

#ifdef DEBUG_UNIT
    validate();
#endif

    return minCap;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
DiscreteTimetableDomain::init()
{
    _capExps = nullptr;
    _capExpCounts = nullptr;
    _capExpsSize = 0;
    _capExpCountsSize = 0;
    _events = 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
DiscreteTimetableDomain::deInit()
{
    delete[] _capExps;
    delete[] _capExpCounts;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_END;
