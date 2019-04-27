#pragma once

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
   Perform a scheduling run.

   SchedulingRun builds on top of SchedulingContext to provide a simplified interface for
   both generating unoptimized schedules and performing optimization runs.

   Use of SchedulingRun initially follows these steps:

   - call the appropriate overload of \ref initialize to prepare for the run
   - call \ref run to execute the run
   - call \ref audit to audit the best schedule

   After the run is complete, other functions such as \ref bestScore and \ref bestScoreAuditReport
   can be called to retrieve information about the best schedule that was found in the run.
     
   \see SchedulingContext
   \ingroup cse
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class SchedulingRun : public utl::Object
{
    UTL_CLASS_DECL(SchedulingRun, utl::Object);
    UTL_CLASS_NO_COPY;

public:
    /// \name Run Control
    //@{
    /**
       Initialize a simple/unoptimized run.
       \param dataSet related ClevorDataSet
       \param scheduler agent that will perform scheduling
       \param objectives Objective%s that will measure the resulting schedule's performance
    */
    void initialize(ClevorDataSet* dataSet,
                    cse::Scheduler* scheduler,
                    const gop::objective_vector_t& objectives);

    /**
       Initialize an optimization run.
       \param dataSet related ClevorDataSet
       \param optimizer gop::Optimizer that will guide the search
       \param optimizerConfig configuration for the provided optimizer
    */
    void initialize(ClevorDataSet* dataSet,
                    gop::Optimizer* optimizer,
                    gop::OptimizerConfiguration* optimizerConfig);

    /** Run. */
    bool run();

    /** Stop optimization run (this would be called from another thread). */
    void stop();

    /** Audit the best result. */
    void audit();
    //@}

    /// \name Conversion between time_t and time-slot
    //@{
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

    /// \name Accessors (const)
    //@{
    /** Get scheduling context. */
    const SchedulingContext*
    context() const
    {
        return _context;
    }

    /** Get the optimizer. */
    const gop::Optimizer*
    optimizer() const
    {
        return _optimizer;
    }

    /** Get number of objectives. */
    uint_t numObjectives() const;

    /** Get makespan. */
    time_t makespan() const;
    //@}

    /// \name Accessors (non-const)
    //@{
    SchedulingContext*
    context()
    {
        return _context;
    }

    /** Get the optimizer. */
    gop::Optimizer*
    optimizer()
    {
        return _optimizer;
    }
    //@}

    /// \name Queries about best result found
    //@{
    /** Get best score. */
    gop::Score* bestScore(uint_t objectiveIdx = 0) const;

    /** Get best score for named objective. */
    gop::Score* bestScore(const std::string& objectiveName) const;

    /** Get best score for named component of named objective. */
    int bestScoreComponent(const std::string& objectiveName,
                           const std::string& componentName) const;

    /** Get audit text for the best result. */
    const std::string& bestScoreAudit() const;

    /** Get audit report for the best result. */
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
