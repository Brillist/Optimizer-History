#include "libcse.h"
#include <libutl/Thread.h>
#include "SEclient.h"
#include "SchedulingRun.h"
#include "RunningThread.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;

/////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(cse::RunningThread);

/////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

/////////////////////////////////////////////////////////////////////////////////////////////////////

void*
RunningThread::run(void* arg)
{
    ASSERT(arg != nullptr);
    auto schedulingRun = static_cast<SchedulingRun*>(arg);
    schedulingRun->run();
    return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;
