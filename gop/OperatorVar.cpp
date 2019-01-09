#include "libgop.h"
#include <libutl/Float.h>
#include <libutl/MemStream.h>
#include <gop/Optimizer.h>
#include "OperatorVar.h"

//////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;

//////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(gop::OperatorVar, utl::Object);

//////////////////////////////////////////////////////////////////////////////

GOP_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////
//// OperatorVar /////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

OperatorVar::OperatorVar(
    uint_t idx,
    bool* active,
    uint_t successIter,
    uint_t totalIter)
{
    ASSERTD(totalIter > 0);
    _idx = idx;
    _successIter = successIter;
    _totalIter = totalIter;
    _p = (double)successIter / (double)totalIter;
    _active = active;
}

//////////////////////////////////////////////////////////////////////////////

void
OperatorVar::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(OperatorVar));
    const OperatorVar& var = (const OperatorVar&)rhs;
    _idx = var._idx;
    _active = var._active;
    _successIter = var._successIter;
    _totalIter = var._totalIter;
    _p = var._p;
}

//////////////////////////////////////////////////////////////////////////////

bool
OperatorVarDecPordering::operator()(
    const OperatorVar* lhs,
    const OperatorVar* rhs) const
{
    if (lhs->p() == rhs->p())
    {
        return (lhs->idx() < rhs->idx());
    }
    return (lhs->p() > rhs->p());
}

//////////////////////////////////////////////////////////////////////////////

GOP_NS_END;
