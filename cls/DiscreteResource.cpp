#include "libcls.h"
#include "TimetableBound.h"
#include "CompositeResource.h"
#include "DiscreteResource.h"
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

CLS_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// TimetableBoundOrderingDecCap ///////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

class TimetableBoundOrderingDecCap : public Ordering
{
    UTL_CLASS_DECL(TimetableBoundOrderingDecCap, Ordering);
    UTL_CLASS_DEFID;

public:
    virtual int cmp(const Object* lhs, const Object* rhs) const;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

int
TimetableBoundOrderingDecCap::cmp(const Object* lhs, const Object* rhs) const
{
    ASSERTD(lhs->isA(TimetableBound));
    ASSERTD(rhs->isA(TimetableBound));
    uint_t lhsCap = ((TimetableBound*)lhs)->capacity();
    uint_t rhsCap = ((TimetableBound*)rhs)->capacity();
    if (lhsCap == rhsCap)
        return lhs->compare(*rhs);
    return utl::compare(rhsCap, lhsCap);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// DiscreteResource ///////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void
DiscreteResource::initialize()
{
    // init timetable's reference to self
    _timetable.res() = this;

    // scan the timetable
    int horizonTS = int_t_min;
    uint_t maxCap = 0;
    const IntSpan* span;
    const IntSpan* tail = _timetable.tail();
    for (span = _timetable.head()->next(); span != tail; span = span->next())
    {
        uint_t cap = span->v1();
        if (cap > 0)
        {
            maxCap = utl::max(maxCap, cap);
            horizonTS = utl::max(horizonTS, span->max() + 1);
        }
    }

    _unary = (maxCap == 100);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
DiscreteResource::addProvidedCapacity(int startTime, int endTime, uint_t cap)
{
    _timetable.add(startTime, endTime - 1, 0, cap);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
DiscreteResource::allocate(int min, int max, uint_t cap, PtActivity* act, bool updateComposite)
{
#ifdef DEBUG_UNIT
    static uint_t iter = 0;
    static uint_t debugId = 66;
    static uint_t debugTS = 3900;
    static uint_t debugCap = 100;

    if (id() == debugId)
    {
        ++iter;
        IntSpan* span = _timetable.find(debugTS);
        uint_t spanCap = span->capacity();
        if (spanCap < debugCap)
        {
            BREAKPOINT;
        }
    }
#endif

    _timetable.add(min, max, cap, 0);

#ifdef DEBUG_UNIT
    if (id() == debugId)
    {
        IntSpan* span = _timetable.find(debugTS);
        uint_t spanCap = span->capacity();
        if (spanCap < debugCap)
        {
            BREAKPOINT;
        }
    }
#endif

    if (!updateComposite)
    {
        return;
    }

    // update composite resource timetables
    Resource** resourcesArray = schedule()->resourcesArray();
    uint_vect_t::iterator it, lim = _crIds.end();
    for (it = _crIds.begin(); it != lim; ++it)
    {
        uint_t resId = *it;
        ASSERTD(resourcesArray[resId]->isA(CompositeResource));
        CompositeResource* res = (CompositeResource*)resourcesArray[resId];
        res->allocate(min, max, 1, act, nullptr, nullptr, this->serialId(), false);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
DiscreteResource::deallocate(int min, int max, uint_t cap, bool updateComposite)
{
    _timetable.add(min, max, -(int)cap, 0);

    if (!updateComposite)
    {
        return;
    }

    // update composite resource timetables
    Resource** resourcesArray = schedule()->resourcesArray();
    uint_vect_t::iterator it, lim = _crIds.end();
    for (it = _crIds.begin(); it != lim; ++it)
    {
        uint_t resId = *it;
        ASSERTD(resourcesArray[resId]->isA(CompositeResource));
        CompositeResource* cres = (CompositeResource*)resourcesArray[resId];
        cres->add(min, max, this->serialId());
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
DiscreteResource::selectCapacity(uint_t cap, uint_t maxCap)
{
    // what is the delta?
    int diff = (int)cap - (int)maxCap;
    ASSERTD(diff <= 0);

    // no change => do nothing else
    if (diff == 0)
        return;

    // find horizon (this works but it's not ideal...)
    int horizonTS = int_t_min;
    const IntSpan* span;
    const IntSpan* last = _timetable.tail()->prev();
    for (span = last; span != nullptr; span = span->prev())
    {
        if (span->v1() > 0)
        {
            horizonTS = span->max();
            break;
        }
    }
    ASSERTD(horizonTS != int_t_min);

    // exclude timetable bounds that require excessive capacity
    forEachIt(utl::RBtree, _timetableBounds, TimetableBound, ttb) if (ttb.capacity() <= cap) break;
    ttb.exclude();
    endForEach;

    // remove capacity from timetable
    _timetable.add(0, horizonTS, 0, diff);

    // update composite resource timetables
    int originTS = getNonBreakTimeNext(-1);
    horizonTS = getNonBreakTimePrev(horizonTS + 1);
    Resource** resourcesArray = schedule()->resourcesArray();
    uint_vect_t::iterator it, lim = _crIds.end();
    ASSERTD(_calendar != nullptr);
    for (it = _crIds.begin(); it != lim; ++it)
    {
        uint_t resId = *it;
        ASSERTD(resourcesArray[resId]->isA(CompositeResource));
        CompositeResource* res = (CompositeResource*)resourcesArray[resId];
        res->allocate(originTS, horizonTS, 1, nullptr, nullptr, _calendar->breakList(),
                      this->serialId(), false);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
DiscreteResource::doubleProvidedCap()
{
    IntSpan* span;
    const IntSpan* tail = _timetable.tail();
    for (span = _timetable.head()->next(); span != tail; span = span->next())
    {
        uint_t cap = span->v1();
        // this check is here because the function can only be used
        // by unary resource for now.
        ASSERTD(cap <= 100);
        span->v1() = 2 * cap;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
DiscreteResource::halveProvidedCap()
{
    IntSpan* span;
    const IntSpan* tail = _timetable.tail();
    for (span = _timetable.head()->next(); span != tail; span = span->next())
    {
        uint_t cap = span->v1();
        // this check is here because the function can only be used
        // by unary resource whose provided cap has been doubled.
        ASSERTD(cap == 0 || cap == 200);
        span->v1() = cap / 2;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
DiscreteResource::init()
{
    _unary = false;
    _minReqCap = 0;
    _maxReqCap = 0;
    _timetableBounds.setOwner(false);
    _timetableBounds.setOrdering(new TimetableBoundOrderingDecCap());
    _calendar = nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
DiscreteResource::deInit()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_END;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(cls::TimetableBoundOrderingDecCap);
UTL_CLASS_IMPL(cls::DiscreteResource);
