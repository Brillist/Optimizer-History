#include "libclp.h"
#include "Goal.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL_ABC(clp::Goal);

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Goal::copy(const Object& rhs)
{
    auto& goal = utl::cast<Goal>(rhs);
    _mgr = goal._mgr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_END;
