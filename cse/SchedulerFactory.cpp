#include "libcse.h"
#include "AltResSelector.h"
#include "ForwardScheduler.h"
#include "MinCostResourcePtSelector.h"
#include "PtSelector.h"
#include "CapSelector.h"
#include "ResCapSelector.h"
#include "JobSeqSelector.h"
#include "JobOpSeqSelector.h"
#include "OpSeqSelector.h"
#include "SchedulerFactory.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

LUT_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(cse::SchedulerFactory);

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

Scheduler*
SchedulerFactory::make(void* param, void*) const
{
    ASSERTD(param != nullptr);
    Scheduler* scheduler = nullptr;
    std::string name((const char*)param);
    if (name == "AltResSelector")
    {
        scheduler = new AltResSelector();
    }
    else if (name == "ForwardScheduler")
    {
        scheduler = new ForwardScheduler();
    }
    else if (name == "MinCostResourcePtSelector")
    {
        scheduler = new MinCostResourcePtSelector();
    }
    else if (name == "PtSelector")
    {
        scheduler = new PtSelector();
    }
    else if (name == "CapSelector")
    {
        scheduler = new CapSelector();
    }
    else if (name == "ResCapSelector")
    {
        scheduler = new ResCapSelector();
    }
    else if (name == "JobSeqSelector")
    {
        scheduler = new JobSeqSelector();
    }
    else if (name == "JobOpSeqSelector")
    {
        scheduler = new JobOpSeqSelector();
    }
    else if (name == "OpSeqSelector")
    {
        scheduler = new OpSeqSelector();
    }
    return scheduler;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;
