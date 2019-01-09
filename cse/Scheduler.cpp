#include "libcse.h"
#include <gop/ConfigEx.h>
#include "Job.h"
#include "Scheduler.h"

//////////////////////////////////////////////////////////////////////////////

#ifdef DEBUG
//#define DEBUG_UNIT
#endif

//////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;
CLP_NS_USE;
CLS_NS_USE;
GOP_NS_USE;

//////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL_ABC(cse::Scheduler, gop::IndBuilder);

//////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////

void
Scheduler::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(Scheduler));
    const Scheduler& scheduler = (const Scheduler&)rhs;
    IndBuilder::copy(scheduler);
    _stringBase = scheduler._stringBase;
    delete _config;
    _config = lut::clone(scheduler._config);
    delete _nestedScheduler;
    _nestedScheduler = lut::clone(scheduler._nestedScheduler);
}

//////////////////////////////////////////////////////////////////////////////

void
Scheduler::serialize(Stream& stream, uint_t io, uint_t)
{
    IndBuilder::serialize(stream, io);
    utl::serializeNullable(_nestedScheduler, stream, io);
}

//////////////////////////////////////////////////////////////////////////////

void
Scheduler::setStringBase(gop::Operator* op) const
{
   if (_nestedScheduler == nullptr)
   {
      throw ConfigEx();
   }
   else
   {
      _nestedScheduler->setStringBase(op);
   }
}

//////////////////////////////////////////////////////////////////////////////

void
Scheduler::setConfig(SchedulerConfiguration* config)
{
   delete _config;
   _config = config;
   if (_nestedScheduler != nullptr)
   {
      _nestedScheduler->setConfig(lut::clone(config));
   }
}

//////////////////////////////////////////////////////////////////////////////

void
Scheduler::init()
{
   _stringBase = 0;
   _config = nullptr;
   _nestedScheduler = nullptr;
}

//////////////////////////////////////////////////////////////////////////////

void
Scheduler::deInit()
{
   delete _config;
   delete _nestedScheduler;
}

//////////////////////////////////////////////////////////////////////////////

CSE_NS_END;
