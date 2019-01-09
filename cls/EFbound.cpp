#include "libcls.h"
#include <libutl/BufferedFDstream.h>
#include "ESbound.h"
#include "EFbound.h"

//////////////////////////////////////////////////////////////////////////////

LUT_NS_USE;
UTL_NS_USE;
CLP_NS_USE;

//////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(cls::EFbound, clp::ConstrainedBound);

//////////////////////////////////////////////////////////////////////////////

CLS_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////

EFbound::EFbound(ESbound* esBound, int lb)
   : clp::ConstrainedBound(esBound->manager(), clp::bound_lb, lb)
{
    _esBound = esBound;
}

//////////////////////////////////////////////////////////////////////////////

int
EFbound::find()
{
    saveState();
    _findPoint = tightest();
    _esBound->queueFind();
    return _bound;
}

//////////////////////////////////////////////////////////////////////////////

void
EFbound::setLB(int lb)
{
    if (lb <= _bound) return;
    if (_esBound->act()->allocated())
    {
        utl::cout << "WARNING: moving a scheduled activity:"
                  << _esBound->act()->id() << ", ef:"
                  << _bound << "->" << lb
                  << utl::endlf;
        _esBound->allocateCapacity(max(_esBound->get(), _bound + 1), lb);
    }
    ConstrainedBound::setLB(lb);
}

//////////////////////////////////////////////////////////////////////////////

CLS_NS_END;
