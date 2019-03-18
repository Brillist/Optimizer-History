#include "libcls.h"
#include <clp/IntExpDomainRISC.h>
#include "LFbound.h"
#include "LFboundTimetable.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;
CLP_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(cls::LFboundTimetable);

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

void
LFboundTimetable::allocateCapacity()
{
    int ls = _act->ls();
    int lf = _act->lf();
    ASSERTD(lf == _bound);
    uint_t cap = capacity();
    _res->allocate(ls, lf, cap, _act);
    if (_res->isUnary() && ((cap == 0) || (cap == 100)))
    {
        _res->actsByStartTime().insert(_act);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
LFboundTimetable::deallocateCapacity()
{
    int ls = _act->ls();
    int lf = _act->lf();
    ASSERTD(lf == _bound);
    uint_t cap = capacity();
    _res->deallocate(ls, lf, cap, _act);
    if (_res->isUnary() && ((cap == 0) || (cap == 100)))
    {
        _res->actsByStartTime().erase(_act);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int
LFboundTimetable::find()
{
    // get calendar and pt-exp
    auto cal = _act->calendar();
    const IntExp* ptExp = _act->possiblePts();

    int ls = _act->ls();
    uint_t pt;
    const IntSpan* span;

    // do nothing if calendar or pt are unknown
    if ((cal == nullptr) || !ptExp->isBound())
    {
        goto succeed;
    }
    pt = (uint_t)ptExp->value();

    // handle (pt == 0) as special case
    if (pt == 0)
    {
        goto succeed;
    }

    // note: lf is valid for the calendar b/c LFboundCalendar ran first
    span = _tt->find(_bound);
    while (!span->isHead())
    {
        int ttmin = span->min();

        if (_bound < ttmin)
        {
            span = span->prev();
            continue;
        }

        bool capacityOK = ((span->v0() == _ttv0) && (span->v1() == _ttv1));

        // found workable es,ef ?
        if (capacityOK && (ls >= ttmin))
        {
            goto succeed;
        }

        _bound = ttmin - 1;
        span = span->prev();
        cal->findBackward(_bound, ls, pt);
    }

    _bound = _findPoint = int_t_min;
    return _bound;
succeed:
    _findPoint = _bound;
    return _bound;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_END;
