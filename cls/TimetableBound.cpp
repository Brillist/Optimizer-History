#include "libcls.h"
#include <clp/IntExpDomainRISC.h>
#include "TimetableBound.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;
CLP_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL_ABC(cls::TimetableBound);

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

TimetableBound::TimetableBound(
    BrkActivity* act, ResourceCapPts* rcp, const CapPt* capPt, bound_t type, int bound)
    : clp::Bound(act->manager(), type, bound)
{
    _possible = true;
    _act = act;
    _res = utl::cast<DiscreteResource>(rcp->resource());
    _timetable = _res->timetable();
    _rcp = rcp;
    _capPt = capPt;

    uint_t cap = capacity();

    // unary resource: use timetable domain directly (efficient special case)
    if (_res->isUnary() && (cap == 100))
    {
        _capExp = nullptr;
        _tt = _timetable->domain();
        _ttv0 = 0;
        _ttv1 = 100;
    }
    else // non-unary resource: use a capacity expression
    {
        _capExp = _timetable->addCapExp(cap);
        _tt = _capExp->domainRISC();
        _ttv0 = 1;
        _ttv1 = 0;
    }

    // resource book-keeping
    if (_res->minReqCap() == 0)
        _res->setMinReqCap(cap);
    else
        _res->setMinReqCap(utl::min(_res->minReqCap(), cap));
    _res->addTimetableBound(this);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
TimetableBound::exclude()
{
    // already excluded?
    if (!_possible)
    {
        return;
    }

    // set _possible = false to note this exclusion
    auto mgr = manager();
    mgr->revToggle(_possible);

    // decrement number of possible cap/pt pairs
    _rcp->decNumPossible();

    // exclude this processing time for the activity
    _act->excludePt(_capPt->processingTime());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
TimetableBound::registerEvents(ConstrainedBound* bound)
{
    if (_capExp == nullptr)
    {
        _timetable->addRangeBound(bound);
    }
    else
    {
        _capExp->addDomainBound(bound);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
TimetableBound::deregisterEvents(ConstrainedBound* bound)
{
    if (_capExp == nullptr)
    {
        _timetable->removeRangeBound(bound);
    }
    else
    {
        _capExp->removeDomainBound(bound);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_END;
