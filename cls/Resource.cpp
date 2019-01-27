#include "libcls.h"
#include "Resource.h"
#include "Schedule.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;
CLP_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(cls::Resource);

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

clp::Manager*
Resource::manager() const
{
    ASSERTD(_schedule != nullptr);
    return _schedule->manager();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Resource::init()
{
    _id = uint_t_max;
    _serialId = uint_t_max;
    _schedule = nullptr;
    _object = nullptr;
    _visited = false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Resource::deInit()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_END;
