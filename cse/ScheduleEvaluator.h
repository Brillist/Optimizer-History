#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <gop/IndEvaluator.h>
#include <gop/Score.h>
#include <cse/SchedulerConfiguration.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Schedule evaluation strategy (abstract).

   ScheduleEvaluator provides a standard interface for different schedule evaluation strategies.

   \ingroup cse
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class ScheduleEvaluator : public gop::IndEvaluator
{
    UTL_CLASS_DECL_ABC(ScheduleEvaluator, gop::IndEvaluator);

public:
    /** Copy another instance. */
    virtual void copy(const utl::Object& rhs);

    /** Initialize. */
    virtual void initialize(const gop::IndEvaluatorConfiguration* cf);

    virtual gop::Score* eval(const gop::IndBuilderContext* context) const;

    virtual double calcScore(const gop::IndBuilderContext* context) const = 0;

protected:
    SchedulerConfiguration* _schedulerConfig;

private:
    void
    init()
    {
        _schedulerConfig = nullptr;
    }
    void
    deInit()
    {
        delete _schedulerConfig;
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;
