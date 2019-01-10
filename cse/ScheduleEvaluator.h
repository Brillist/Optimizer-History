#ifndef CSE_SCHEDULEEVALUATOR_H
#define CSE_SCHEDULEEVALUATOR_H

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <gop/IndEvaluator.h>
#include <gop/Score.h>
#include <cse/SchedulerConfiguration.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Schedule evaluation strategy (abstract).

   ScheduleEvaluator provides a standard interface for different schedule
   evaluation strategies.

   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class ScheduleEvaluator : public gop::IndEvaluator
{
    UTL_CLASS_DECL_ABC(ScheduleEvaluator);

public:
    /** Copy another instance. */
    virtual void copy(const utl::Object& rhs);

    /** Initialize. */
    virtual void initialize(const gop::IndEvaluatorConfiguration* cf);

    virtual gop::Score* eval(const gop::IndBuilderContext* context) const;

    virtual double calcScore(const gop::IndBuilderContext* context) const = 0;

    /*     virtual double bestFailScore( */
    /*         const gop::IndBuilderContext* context) const; */

    /*     virtual double failScore( */
    /*         const gop::IndBuilderContext* context) const; */
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

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
