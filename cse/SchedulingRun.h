#ifndef CSE_SCHEDULINGRUN_H
#define CSE_SCHEDULINGRUN_H

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <cse/AuditReport.h>
#include <cse/Scheduler.h>
#include <cse/SchedulingContext.h>
#include <cse/SEclient.h>
#include <gop/Objective.h>
#include <gop/Optimizer.h>
#include <gop/Score.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Manage a scheduling run.

   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class SchedulingRun : public utl::Object
{
    UTL_CLASS_DECL(SchedulingRun, utl::Object);
    UTL_CLASS_NO_COPY;

public:
    /// \name Run Control
    //@{
    /** Initialize a non-optimization run. */
    void initialize(ClevorDataSet* dataSet,
                    cse::Scheduler* scheduler,
                    const gop::objective_vector_t& objectives);

    /** Initialize an optimization run. */
    void initialize(ClevorDataSet* dataSet,
                    gop::Optimizer* optimizer,
                    gop::OptimizerConfiguration* optimizerConfig);

    /** Run. */
    bool run();

    /** Stop optimization run. */
    void stop();

    /** Audit the best result. */
    void audit();

    /** Convert a time-slot to a time. */
    time_t
    timeSlotToTime(int ts) const
    {
        return _context->timeSlotToTime(ts);
    }

    /** Convert a time to a time-slot. */
    int
    timeToTimeSlot(time_t t) const
    {
        return _context->timeToTimeSlot(t);
    }
    //@}

    //------------------------------------------------------------------------

    /// \name Accessors
    //@{
    /** Get scheduling context. */
    const SchedulingContext*
    context() const
    {
        return _context;
    }

    SchedulingContext*
    context()
    {
        return _context;
    }

    /** Get the optimizer. */
    const gop::Optimizer*
    optimizer() const
    {
        return _optimizer;
    }

    /** Get the optimizer. */
    gop::Optimizer*
    optimizer()
    {
        return _optimizer;
    }

    /** Get number of objectives. */
    uint_t numObjectives() const;

    /** Get makespan. */
    time_t makespan() const;

    /** Get best score. */
    gop::Score* bestScore(uint_t objectiveIdx = 0) const;

    /** Get best score for given objective. */
    gop::Score* bestScore(const std::string& objectiveName) const;

    /** Get best score component. */
    int bestScoreComponent(const std::string& objectiveName,
                           const std::string& componentName) const;

    /** Get best score audit. */
    const std::string& bestScoreAudit() const;

    /** Get best score audit report. */
    const AuditReport* bestScoreAuditReport() const;
    //@}
private:
    void init();
    void deInit();

private:
    SchedulingContext* _context;
    gop::Optimizer* _optimizer;
    cse::Scheduler* _scheduler;
    gop::objective_vector_t _objectives;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
