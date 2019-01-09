#include "libclp.h"
#include "IntExpDomainRISCit.h"
#include "IntExpDomainRISC.h"

//////////////////////////////////////////////////////////////////////////////

#ifdef DEBUG
//#define DEBUG_UNIT
#endif

//////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;

//////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(clp::IntExpDomainRISC, clp::IntExpDomain);

//////////////////////////////////////////////////////////////////////////////

CLP_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////

void
IntExpDomainRISC::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(IntExpDomainRISC));
    const IntExpDomainRISC& dr = (const IntExpDomainRISC&)rhs;

    // copy base
    IntExpDomain::copy(dr);
    RevIntSpanCol::copy(dr);

#ifdef DEBUG_UNIT
    validate();
#endif
}

//////////////////////////////////////////////////////////////////////////////

utl::String
IntExpDomainRISC::toString() const
{
    std::ostringstream ss;
    const IntSpan* span = head()->next();
    if (span->v0() == 0)
    {
        span = span->next();
    }
    bool first = true;
    while (!span->isTail())
    {
        if (!first)
        {
            ss << ",";
        }
        if (span->min() == span->max())
        {
            ss << "[" << span->min() << "]";
        }
        else
        {
            ss << "[" << span->min() << ".." << span->max() << "]";
        }
        span = span->next()->next();
        first = false;
    }
    return ss.str().c_str();
}

//////////////////////////////////////////////////////////////////////////////

void
IntExpDomainRISC::intersect(const IntExpDomain* rhs)
{
    if (dynamic_cast<const IntExpDomainRISC*>(rhs) == nullptr)
    {
        IntExpDomain::intersect(rhs);
        return;
    }

    const IntExpDomainRISC& rhsDomain = (const IntExpDomainRISC&)*rhs;
    const IntSpan* rhsSpan = rhsDomain.head()->next();
    const IntSpan* rhsTail = rhsDomain.tail();
    if (rhsSpan->v0() == 0)
    {
        rhsSpan = rhsSpan->next();
    }
    int lastVal = int_t_min + 1;
    while (rhsSpan != rhsTail)
    {
        remove(lastVal + 1, rhsSpan->min() - 1);
        lastVal = rhsSpan->max();
        rhsSpan = rhsSpan->next()->next();
    }
    remove(lastVal + 1, int_t_max);
}

//////////////////////////////////////////////////////////////////////////////

bool
IntExpDomainRISC::has(int val) const
{
    if ((val < _min) || (val > _max))
    {
        return false;
    }
    const IntSpan* span = find(val);
    return (span->v0() == 1);
}

//////////////////////////////////////////////////////////////////////////////

IntExpDomainIt*
IntExpDomainRISC::begin() const
{
    if (_size == 0) return end();
    IntExpDomainRISCit* it
        = new IntExpDomainRISCit(this, _head, int_t_min);
    it->next();
    return it;
}

//////////////////////////////////////////////////////////////////////////////

IntExpDomainIt*
IntExpDomainRISC::end() const
{
    IntExpDomainRISCit* it = new IntExpDomainRISCit(this, _tail, int_t_max);
    return it;
}

//////////////////////////////////////////////////////////////////////////////

int
IntExpDomainRISC::getPrev(int val) const
{
    if (val <= _min)
    {
        return int_t_min;
    }

    if (val > _max)
    {
        return _max;
    }

    const IntSpan* span = find(val - 1);
    if (span->v0() == 0)
    {
        return (span->min() - 1);
    }
    return (val - 1);
}

//////////////////////////////////////////////////////////////////////////////

int
IntExpDomainRISC::getNext(int val) const
{
    if (val < _min)
    {
        return _min;
    }

    if (val >= _max)
    {
        return int_t_max;
    }

    const IntSpan* span = find(val + 1);
    if (span->v0() == 0)
    {
        return (span->max() + 1);
    }
    return (val + 1);
}

//////////////////////////////////////////////////////////////////////////////

void
IntExpDomainRISC::_saveState()
{
    IntExpDomain::_saveState();
    RevIntSpanCol::_saveState();
}

//////////////////////////////////////////////////////////////////////////////

void
IntExpDomainRISC::addRange(int min, int max)
{
    ASSERTD(mgr() != nullptr);

    if (min > max)
    {
        return;
    }

    saveState();

    // --- initialize ---------------------------------------------------------

    // find minSpan, maxSpan
    IntSpan* prev[CLP_INTSPAN_MAXDEPTH];
    IntSpan* next[CLP_INTSPAN_MAXDEPTH];
    IntSpan* minSpan = findPrev(min, prev);
    IntSpan* maxSpan;
    if (minSpan->v0() == 1)
    {
        minSpan = minSpan->next();
        findPrevForward(minSpan->min(), prev);
    }
    if (minSpan->min() > max)
    {
        return;
    }
    // note: minSpan->min() <= max
    if (minSpan->max() >= max)
    {
        maxSpan = minSpan;
        prevToNext(minSpan, prev, next);
    }
    else
    {
        maxSpan = findNext(max, next);
        if (maxSpan->v0() == 1)
        {
            maxSpan = maxSpan->prev();
            backward(next);
        }
    }
    IntSpan* prev0 = prev[0];
    IntSpan* next0 = next[0];

    // determine minEdge,maxEdge
    int minSpanMin = minSpan->min();
    int maxSpanMax = maxSpan->max();
    int minAdd = utl::max(min, minSpanMin);
    int maxAdd = utl::min(max, maxSpanMax);
    bool minEdge = (minAdd == minSpanMin);
    bool maxEdge = (maxAdd == maxSpanMax);

    // nothing to do?
    if (minAdd > maxAdd)
    {
        return;
    }

    // --- update domain ------------------------------------------------------

    // save state of minSpan
    minSpan->saveState(mgr());

    // update min,max
    if (minAdd < _min)
    {
        _events |= ef_min;
        _events |= ef_range;
        _min = minAdd;
    }
    if (maxAdd > _max)
    {
        _events |= ef_max;
        _events |= ef_range;
        _max = maxAdd;
    }

    // flag events
    _events |= ef_domain;

    // --- minSpan == maxSpan -------------------------------------------------

    if (minSpan == maxSpan)
    {
        IntSpan* span = minSpan;

        // update size
        _size += (maxAdd - minAdd + 1);

        // add deltas
        if (minEdge && maxEdge)
        {
            findNextForward(next0->max(), next);
            link(prev, next);

            prev0->max() = next0->max();
        }
        if (minEdge)
        {
            prev0->saveState(mgr());
            span->saveState(mgr());
            prev0->max() = maxAdd;
            span->min() = (maxAdd + 1);
        }
        else if (maxEdge)
        {
            span->saveState(mgr());
            next0->saveState(mgr());
            span->max() = (minAdd - 1);
            next0->min() = minAdd;
        }
        else
        {
            int spanMax = span->max();
            findPrevForward(spanMax + 1, prev);
            span->saveState(mgr());
            span->max() = (minAdd - 1);
            IntSpan* newSpan = new IntSpan(minAdd, maxAdd, 1, 0);
            mgr()->revAllocate(newSpan);
            insertAfter(newSpan, prev);
            findPrevForward(maxAdd + 1, prev);
            newSpan = new IntSpan(maxAdd + 1, spanMax, 0, 0);
            mgr()->revAllocate(newSpan);
            insertAfter(newSpan, prev);
        }
#ifdef DEBUG_UNIT
        validate(true);
#endif
        return;
    }

    // --- minSpan != maxSpan -------------------------------------------------

    maxSpan->saveState(mgr());

    // update size
    _size += (minSpan->max() - minAdd + 1);
    _size += (maxAdd - maxSpan->min() + 1);
    for (IntSpan* span = minSpan->next()->next();
         span != maxSpan;
         span = span->next()->next())
    {
        _size += span->size();
    }

    // add deltas, update domain
    if (minEdge && maxEdge)
    {
        //    001100
        // => 111111
        findNextForward(next0->max(), next);
        link(prev, next);

        prev0->max() = next0->max();
    }
    else if (minEdge)
    {
        //    001100
        // => 111110
        backward(next);
        link(prev, next);

        prev0->max() = maxAdd;
        maxSpan->min() = (maxAdd + 1);
    }
    else if (maxEdge)
    {
        //    001100
        // => 011111
        findPrevForward(minSpan->max() + 1, prev);
        link(prev, next);

        minSpan->max() = (minAdd - 1);
        maxSpan->next()->min() = minAdd;
    }
    else
    {
        //    001100
        // => 011110
        findPrevForward(minSpan->next()->max() + 1, prev);
        backward(next);
        link(prev, next);

        minSpan->max() = (minAdd - 1);
        minSpan->next()->min() = minAdd;

        maxSpan->prev()->max() = maxAdd;
        maxSpan->min() = (maxAdd + 1);
    }

#ifdef DEBUG_UNIT
    validate(true);
#endif
}

//////////////////////////////////////////////////////////////////////////////

void
IntExpDomainRISC::removeRange(int min, int max)
{
    ASSERTD(mgr() != nullptr);

    min = utl::max(min, _min);
    max = utl::min(max, _max);

    if ((min > max) || (min > _max) || (max < _min))
    {
        return;
    }

    saveState();

    if ((min <= _min) && (max >= _max))
    {
        _events |= ef_empty;
        _size = 0;
        _min = int_t_max;
        _max = int_t_min;
        return;
    }

    // --- increase minimum? --------------------------------------------------

    IntSpan* minSpan;
    IntSpan* maxSpan;
    IntSpan* prev0;
    IntSpan* next0;

    prev0 = _head->next();
    minSpan = prev0->next();
    if ((min <= minSpan->min()) && (max < minSpan->max()))
    {
        // adjust size, min
        _size -= (max - minSpan->min() + 1);
        _min = (max + 1);

        // events
        _events |= ef_domain;
        _events |= ef_min;
        _events |= ef_range;
        if (_min == _max)
        {
            _events |= ef_value;
        }

        prev0->saveState(mgr());
        minSpan->saveState(mgr());
        prev0->max() = max;
        minSpan->min() = (max + 1);

        return;
    }

    // --- decrease maximum? --------------------------------------------------

    next0 = _tail->prev();
    maxSpan = next0->prev();
    if ((min > maxSpan->min()) && (max >= maxSpan->max()))
    {
        // adjust size, max
        _size -= (maxSpan->max() - min + 1);
        _max = (min - 1);

        // events
        _events |= ef_domain;
        _events |= ef_max;
        _events |= ef_range;
        if (_min == _max)
        {
            _events |= ef_value;
        }

        maxSpan->saveState(mgr());
        next0->saveState(mgr());
        maxSpan->max() = (min - 1);
        next0->min() = min;
        return;
    }

    // --- initialize ---------------------------------------------------------

    // find minSpan, maxSpan
    IntSpan* prev[CLP_INTSPAN_MAXDEPTH];
    IntSpan* next[CLP_INTSPAN_MAXDEPTH];
    minSpan = findPrev(min, prev);
    if (minSpan->v0() == 0)
    {
        minSpan = minSpan->next();
        findPrevForward(minSpan->min(), prev);
    }
    if (minSpan->min() > max)
    {
        return;
    }
    // note: minSpan->min() <= max
    if (minSpan->max() >= max)
    {
        maxSpan = minSpan;
        prevToNext(minSpan, prev, next);
    }
    else
    {
        maxSpan = findNext(max, next);
        if (maxSpan->v0() == 0)
        {
            maxSpan = maxSpan->prev();
            backward(next);
        }
    }
    prev0 = prev[0];
    next0 = next[0];

    // determine minEdge,maxEdge
    int minSpanMin = minSpan->min();
    int maxSpanMax = maxSpan->max();
    int minRemove = utl::max(min, minSpanMin);
    int maxRemove = utl::min(max, maxSpanMax);
    bool minEdge = (minRemove == minSpanMin);
    bool maxEdge = (maxRemove == maxSpanMax);

    // --- update domain ------------------------------------------------------

    // save state of minSpan
    minSpan->saveState(mgr());

    // update min,max
    if (minRemove == _min)
    {
        _events |= ef_min;
        _events |= ef_range;
        _min = maxEdge ? maxSpan->next()->next()->min() : (maxRemove + 1);
    }
    if (maxRemove == _max)
    {
        _events |= ef_max;
        _events |= ef_range;
        _max = minEdge ? minSpan->prev()->prev()->max() : (minRemove - 1);
    }

    // flag events
    _events |= ef_domain;
    if (_min == _max)
    {
        _events |= ef_value;
    }

    // --- minSpan == maxSpan -------------------------------------------------

    if (minSpan == maxSpan)
    {
        IntSpan* span = minSpan;

        // update size
        _size -= (maxRemove - minRemove + 1);

        // add deltas
        if (minEdge && maxEdge)
        {
            findNextForward(next0->max(), next);
            link(prev, next);

            prev0->max() = next0->max();
        }
        else if (minEdge)
        {
            prev0->saveState(mgr());
            span->saveState(mgr());
            prev0->max() = max;
            span->min() = (max + 1);
        }
        else if (maxEdge)
        {
            span->saveState(mgr());
            next0->saveState(mgr());
            span->max() = (min - 1);
            next0->min() = min;
        }
        else
        {
            int spanMax = span->max();
            findPrevForward(spanMax + 1, prev);
            span->saveState(mgr());
            span->max() = (min - 1);
            IntSpan* newSpan = new IntSpan(min, max, 0, 0);
            mgr()->revAllocate(newSpan);
            insertAfter(newSpan, prev);
            findPrevForward(max + 1, prev);
            newSpan = new IntSpan(max + 1, spanMax, 1, 0);
            mgr()->revAllocate(newSpan);
            insertAfter(newSpan, prev);
        }
#ifdef DEBUG_UNIT
        validate(true);
#endif
        return;
    }

    // --- minSpan != maxSpan -------------------------------------------------

    maxSpan->saveState(mgr());

    // update size
    if (_min == _max)
    {
        _size = 1;
    }
    else
    {
        _size -= (minSpan->max() - minRemove + 1);
        _size -= (maxRemove - maxSpan->min() + 1);
        for (IntSpan* span = minSpan->next()->next();
             span != maxSpan;
             span = span->next()->next())
        {
            _size -= span->size();
        }
    }

    // add deltas, update domain
    if (minEdge && maxEdge)
    {
        //    110011
        // => 000000
        findNextForward(next0->max(), next);
        link(prev, next);

        prev0->max() = next0->max();
    }
    else if (minEdge)
    {
        //    110011
        // => 000001
        backward(next);
        link(prev, next);

        prev0->max() = maxRemove;
        maxSpan->min() = (maxRemove + 1);
    }
    else if (maxEdge)
    {
        //    110011
        // => 110000
        findPrevForward(minSpan->max() + 1, prev);
        link(prev, next);

        minSpan->max() = (minRemove - 1);
        maxSpan->next()->min() = minRemove;
    }
    else
    {
        //    110011
        // => 100001
        findPrevForward(minSpan->next()->max() + 1, prev);
        backward(next);
        link(prev, next);

        minSpan->max() = (minRemove - 1);
        minSpan->next()->min() = minRemove;

        maxSpan->prev()->max() = maxRemove;
        maxSpan->min() = (maxRemove + 1);
    }

#ifdef DEBUG_UNIT
    validate(true);
#endif
}

//////////////////////////////////////////////////////////////////////////////

void
IntExpDomainRISC::set(IntSpan* span)
{
    // update min, max
    if (span->v0() == 1)
    {
        _min = utl::min(_min, span->min());
        _max = utl::max(_max, span->max());
        _size += span->size();
    }

    RevIntSpanCol::set(span);
}

//////////////////////////////////////////////////////////////////////////////

uint_t
IntExpDomainRISC::validate(bool initialized) const
{
    uint_t size = RevIntSpanCol::validate(initialized);
    if (initialized && (_head->next() != _tail))
    {
        IntSpan* minSpan = _head->next()->next();
        IntSpan* maxSpan = _tail->prev()->prev();
        if (minSpan->min() < _min)
        {
            size -= (_min - minSpan->min());
        }
        if (maxSpan->max() > _max)
        {
            size -= (maxSpan->max() - _max);
        }
    }
    ASSERT(size == _size);
    return size;
}

//////////////////////////////////////////////////////////////////////////////

void
IntExpDomainRISC::init()
{
    ASSERTD(_head->next() == _tail);
    RevIntSpanCol::setManager(mgr());
    set(int_t_min + 2, int_t_max - 2, 1, 0);
    set(int_t_min + 1, int_t_min + 1, 0, 0);
    set(int_t_max - 1, int_t_max - 1, 0, 0);
}

//////////////////////////////////////////////////////////////////////////////

CLP_NS_END;
