#include "libcls.h"
#include "ESboundInt.h"
#include "EFboundInt.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

LUT_NS_USE;
UTL_NS_USE;
CLP_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(cls::EFboundInt);

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

EFboundInt::EFboundInt(ESboundInt* esBound, int lb)
    : clp::ConstrainedBound(esBound->manager(), clp::bound_lb, lb)
{
    _esBound = esBound;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int
EFboundInt::find()
{
    saveState();
    _findPoint = tightest();
    _esBound->queueFind();
    return _bound;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_END;
