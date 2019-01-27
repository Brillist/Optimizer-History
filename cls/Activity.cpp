#include "libcls.h"
#include <clp/Manager.h>
#include <cls/Schedule.h>
#include "Activity.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;
CLP_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(cls::Activity);

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

Activity::Activity(Schedule* schedule)
{
    init();
    _schedule = schedule;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

String
Activity::toString() const
{
    std::ostringstream ss;
    ss << "id: " << id() << ", ";
    if (!name().empty())
    {
        ss << "name: '" << name() << "', ";
    }
    ss << "start: " << start().toString().get() << ", "
       << "end: " << end().toString().get();
    return ss.str().c_str();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Manager*
Activity::manager()
{
    if (_schedule == nullptr)
        return nullptr;
    return _schedule->manager();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Activity::setDebugFlag(bool debugFlag)
{
    _start.setDebugFlag(debugFlag);
    _end.setDebugFlag(debugFlag);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Activity::init()
{
    _id = uint_t_max;
    _serialId = uint_t_max;
    _schedule = nullptr;
    _owner = nullptr;
    _allocated = false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Activity::deInit()
{
    _schedule = nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_END;
