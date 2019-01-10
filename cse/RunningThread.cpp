#include "libcse.h"
#include <libutl/Thread.h>
#include "SEclient.h"
#include "SchedulingRun.h"
#include "RunningThread.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;

/////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(cse::RunningThread, utl::Thread);

/////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

/////////////////////////////////////////////////////////////////////////////////////////////////////

void*
RunningThread::run(void* arg)
{
    ASSERT(arg != nullptr);
    //     ASSERT(dynamic_cast<SchedulingRun*>(arg) != nullptr);
    SchedulingRun* schedulingRun = (SchedulingRun*)arg;
    schedulingRun->run();
    return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;
