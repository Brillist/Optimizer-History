#include "libcls.h"
#include <clp/IntExpDomainRISC.h>
#include "ESbound.h"
#include "ESboundTimetable.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;
CLP_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(cls::ESboundTimetable, cls::TimetableBound);

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ESboundTimetable::allocateCapacity()
{
    int es = _act->es();
    int ef = _act->ef();
    ASSERTD(es == _bound);
    uint_t cap = capacity();
    _res->allocate(es, ef, cap, _act);
    if (_res->isUnary() && ((cap == 0) || (cap == 100)))
    {
        _res->actsByStartTime().insert(_act);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ESboundTimetable::allocateCapacity(int t1, int t2)
{
    uint_t cap = capacity();
    //_act is only usefull for composite resource,
    // so it will be ignored later anyway.
    _res->allocate(t1, t2, cap, _act);
    // no need to insert _act again into _res->actsByStartTime
    // because this function should only be used for shifting ef
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ESboundTimetable::deallocateCapacity()
{
    int es = _act->es();
    int ef = _act->ef();
    ASSERTD(es == _bound);
    uint_t cap = capacity();
    _res->deallocate(es, ef, cap, _act);
    if (_res->isUnary() && ((cap == 0) || (cap == 100)))
    {
        _res->actsByStartTime().erase(_act);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ESboundTimetable::deallocateCapacity(int t1, int t2)
{
    uint_t cap = capacity();
    _res->deallocate(t1, t2, cap, _act);
    // to erase _act from and re-insert it into
    // _res->actsByStartTime() is necessary, because
    // this function is used for shifting es
    if (_res->isUnary() && ((cap == 0) || (cap == 100)))
    {
        _res->actsByStartTime().erase(_act);
        _res->actsByStartTime().insert(_act);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int
ESboundTimetable::find()
{
    saveState();

    // get calendar and pt-exp
    const ResourceCalendar* cal = _act->calendar();
    const IntExp* ptExp = _act->possiblePts();

    int ef = _act->ef();
    uint_t pt;
    const IntSpan* span;

    // do nothing if calendar or pt are unknown
    if ((cal == nullptr) || !ptExp->isBound())
    {
        goto succeed;
    }
    pt = (uint_t)ptExp->getValue();

    // handle (pt == 0) as special case
    if (pt == 0)
    {
        goto succeed;
    }

    // note: es,ef are valid for the calendar b/c ESboundCalendar ran first
    span = _tt->find(_bound);
    while (!span->isTail())
    {
        int ttmax = span->max();

        if (_bound > ttmax)
        {
            span = span->next();
            continue;
        }

        bool capacityOK = ((span->v0() == _ttv0) && (span->v1() == _ttv1));

        // found workable es,ef ?
        if (capacityOK && (ef <= ttmax))
        {
            goto succeed;
        }

        _bound = ttmax + 1;
        span = span->next();
        cal->findForward(_bound, ef, pt);
    }

    _bound = int_t_max;
    return _bound;
succeed:
    return _bound;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_END;
