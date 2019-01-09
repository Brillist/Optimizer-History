#ifndef CSE_INDEVALUATORCONFIGURATION_H
#define CSE_INDEVALUATORCONFIGURATION_H

//////////////////////////////////////////////////////////////////////////////

#include <gop/IndEvaluatorConfiguration.h>
#include <cse/SchedulerConfiguration.h>

//////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////

/**
   Schedule evaluator configuration (abstract).

   ScheduleEvaluatorConfiguration serves as an abstract base for classes
   that store configuration options for ScheduleEvaluator-derived classes.

   \author Adam McKee
*/

//////////////////////////////////////////////////////////////////////////////

class ScheduleEvaluatorConfiguration : public gop::IndEvaluatorConfiguration
{
    UTL_CLASS_DECL_ABC(ScheduleEvaluatorConfiguration);
public:
   /** Copy another instance. */
    virtual void copy(const utl::Object& rhs);

    /** Get the scheduler-configuration. */
    SchedulerConfiguration* getSchedulerConfig() const
    { return _schedulerConfig; }

    /** Set the scheduler-configuration. */
    void setSchedulerConfig(SchedulerConfiguration* schedulerConfig)
    { delete _schedulerConfig; _schedulerConfig = schedulerConfig; }
private:
    void init()
    { _schedulerConfig = nullptr; }
    void deInit()
    { delete _schedulerConfig; }
private:
    SchedulerConfiguration* _schedulerConfig;
};

//////////////////////////////////////////////////////////////////////////////

CSE_NS_END;

//////////////////////////////////////////////////////////////////////////////

#endif
