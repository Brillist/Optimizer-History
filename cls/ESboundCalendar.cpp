#include "libcls.h"
#include "ESboundCalendar.h"

//////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;
CLP_NS_USE;

//////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(cls::ESboundCalendar, clp::Bound);

//////////////////////////////////////////////////////////////////////////////

CLS_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////

ESboundCalendar::ESboundCalendar(
    BrkActivity* act,
    int lb,
    EFbound* efBound)
    : clp::Bound(act->manager(), clp::bound_lb, lb)
{
    _act = act;
    _efBound = efBound;
}

//////////////////////////////////////////////////////////////////////////////

int
ESboundCalendar::find()
{
    saveState();

    // get calendar and pt-exp
    const ResourceCalendar* cal = _act->calendar();
    const IntExp* ptExp = _act->possiblePts();

    int ef;
    uint_t pt;

    // do nothing if calendar or pt are unknown
    if ((cal == nullptr) || !ptExp->isBound())
    {
        ef = _efBound->get();
        goto succeed;
    }
    pt = (uint_t)ptExp->getValue();

    // handle (pt == 0) as special case
    if (pt == 0)
    {
        ef = _efBound->get();
        if (_bound <= ef)
        {
            _bound = ef + 1;
        }
        else // _bound > ef
        {
            ef = _bound - 1;
        }
        goto succeed;
    }

    // let calendar find valid es,ef bounds
    cal->findForward(_bound, ef, pt);

    // cannot execute
    if (_bound == int_t_max)
    {
        goto fail;
    }

    // ES may need to move forward to match EF
    if (ef < _efBound->get())
    {
        ef = _efBound->get();
        ef = cal->getNonBreakTimeNext(ef - 1);
        if (ef == int_t_max)
        {
            goto fail;
        }
        _bound = cal->getStartTimeForEndTime(ef, pt);
    }
    goto succeed;

fail:
    _bound = _findPoint = ef = int_t_max;
succeed:
    _findPoint = _bound;
    _efBound->setFindPoint(ef);
    _efBound->setLB(ef);
    return _bound;
}

//////////////////////////////////////////////////////////////////////////////

CLS_NS_END;
