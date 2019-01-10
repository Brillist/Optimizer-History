#include "libcls.h"
#include "LFbound.h"
#include "LSbound.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

LUT_NS_USE;
UTL_NS_USE;
CLP_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(cls::LSbound, clp::ConstrainedBound);

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

LSbound::LSbound(LFbound* lfBound, int ub)
    : clp::ConstrainedBound(lfBound->manager(), clp::bound_ub, ub)
{
    _lfBound = lfBound;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int
LSbound::find()
{
    saveState();
    _findPoint = tightest();
    _lfBound->queueFind();
    //_lfBound->find();
    return _bound;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_END;
