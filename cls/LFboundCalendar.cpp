#include "libcls.h"
#include "LFboundCalendar.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;
CLP_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(cls::LFboundCalendar);

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

LFboundCalendar::LFboundCalendar(BrkActivity* act, int ub)
    : clp::Bound(act->manager(), clp::bound_ub, ub)
{
    _act = act;
    ASSERTD(dynamic_cast<LSbound*>(&act->lsBound()) != nullptr);
    _lsBound = (LSbound*)&act->lsBound();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int
LFboundCalendar::find()
{
    // get calendar and pt-exp
    auto cal = _act->calendar();
    const IntExp* ptExp = _act->possiblePts();

    int ls;
    uint_t pt;

    // do nothing if calendar or pt are unknown
    if ((cal == nullptr) || !ptExp->isBound())
    {
        ls = _lsBound->get();
        goto succeed;
    }
    pt = (uint_t)ptExp->value();

    // handle (pt == 0) as special case
    if (pt == 0)
    {
        ls = _bound + 1;
        goto succeed;
    }

    // let calendar find valid es,ef bounds
    cal->findBackward(_bound, ls, pt);

    // cannot execute
    if (_bound == int_t_min)
    {
        goto fail;
    }

    // LF may need to move backward to match LS
    if (ls > _lsBound->get())
    {
        ls = _lsBound->get();
        ls = cal->getNonBreakTimePrev(ls + 1);
        if (ls == int_t_min)
        {
            goto fail;
        }
        _bound = cal->getEndTimeForStartTime(ls, pt);
    }
    goto succeed;

fail:
    _bound = _findPoint = ls = int_t_min;
succeed:
    _findPoint = _bound;
    _lsBound->setFindPoint(ls);
    _lsBound->setLB(ls);
    return _bound;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_END;
