#pragma once

/////////////////////////////////////////////////////////////////////////////////////////////////////

#include <libutl/Thread.h>

/////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

/////////////////////////////////////////////////////////////////////////////////////////////////////

class RunningThread : public utl::Thread
{
    UTL_CLASS_DECL(RunningThread, utl::Thread);
    UTL_CLASS_DEFID;

public:
    virtual void* run(void* arg = nullptr);
};

/////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;
