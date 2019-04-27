#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <gop/IndBuilderContext.h>
#include <cls/Schedule.h>
#include <cse/ClevorDataSet.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Context for scheduling.

   SchedulingContext provides a basis for schedule construction, supporting both optimization
   runs and "simple" runs (forward scheduling).

   Use of SchedulingContext begins with a call to \ref initialize to prepare for scheduling
   a ClevorDataSet.

   Then, for each scheduling run:

   - call \ref initialize to prepare for scheduling a ClevorDataSet
   - call \ref clear to reset state after a previous scheduling run (if any)
   - call \ref schedule to schedule JobOp%s until no more remain
   - call \ref setComplete to indicate that the schedule is fully constructed
   - call \ref store to record information about the generated scheduled in the ClevorDataSet

   \see SchedulingRun
   \ingroup cse
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class SchedulingContext : public gop::IndBuilderContext
{
    UTL_CLASS_DECL(SchedulingContext, gop::IndBuilderContext);
    UTL_CLASS_NO_COPY;

public:
    /**
       Initialize.
       \param dataSet associated ClevorDataSet
    */
    void initialize(ClevorDataSet* dataSet);

    /** Prepare to generate a new schedule. */
    virtual void clear();

    /** Schedule the given operation. */
    void schedule(JobOp* op);

    /** Store the schedule into the data-set. */
    void store();

    /** Set scheduling complete flag. */
    void setComplete(bool complete);

    /// \name Accessors (const)
    //@{
    /** Get the gop::DataSet. */
    const gop::DataSet*
    dataSet() const
    {
        return _dataSet;
    }

    /** Get the ClevorDataSet. */
    const cse::ClevorDataSet*
    clevorDataSet() const
    {
        return _dataSet;
    }

    /** Get the Manager. */
    const clp::Manager*
    manager() const
    {
        return _mgr;
    }

    /** Get the Schedule. */
    const cls::Schedule*
    schedule() const
    {
        return _schedule;
    }

    /** Scheduling complete? */
    bool
    complete() const
    {
        return _complete;
    }

    /** Get the hard constraint score. */
    uint_t
    hardCtScore() const
    {
        return _hardCtScore;
    }

    /** Get the number of scheduled operations. */
    uint_t
    numScheduledOps() const
    {
        return _numScheduledOps;
    }

    /** Get the makespan (as time_t). */
    time_t
    makespan() const
    {
        ASSERTD(_config != nullptr);
        return _config->timeSlotToTime(_makespan);
    }

    /** Get the makespan (as time-slot). */
    int
    makespanTimeSlot() const
    {
        return _makespan;
    }
    //@}

    /// \name Accessors (non-const)
    //@{
    /** Set CSE dataSet ownership flag. */
    void
    setDataSetOwner(bool owner)
    {
        _dataSetOwner = owner;
    }

    /** Get ClevorDataSet (non-const). */
    cse::ClevorDataSet*
    clevorDataSet()
    {
        return _dataSet;
    }

    /** Get the Manager (non-const). */
    clp::Manager*
    manager()
    {
        return _mgr;
    }

    /** Get the Schedule (non-const). */
    cls::Schedule*
    schedule()
    {
        return _schedule;
    }
    //@}

    /// \name Schedulable Jobs
    //@{
    /** Schedulable-jobs empty? */
    bool
    sjobsEmpty() const
    {
        return (_sjobs.begin() == _sjobs.end());
    }

    /** Add to the list of schedulable-jobs. */
    void sjobsAdd(Job* job);

    /** Remove from the list of schedulable-jobs. */
    void sjobsRemove(Job* job);

    /** Get begin iterator for schedulable-jobs. */
    Job**
    sjobsBegin() const
    {
        return (Job**)_sjobs.begin();
    }

    /** Get end iterator for schedulable-jobs. */
    Job**
    sjobsEnd() const
    {
        return (Job**)_sjobs.end();
    }
    //@}

    /// \name Conversion between time_t and time-slot
    //@{
    /** Convert time => time-slot. */
    uint_t
    timeToTimeSlot(time_t t) const
    {
        return _config->timeToTimeSlot(t);
    }

    /** Convert time-slot => time. */
    time_t
    timeSlotToTime(uint_t ts) const
    {
        return _config->timeSlotToTime(ts);
    }
    //@}

private:
    void init();
    void deInit();

    void setResCapPtsAdj();
    void initSummaryOps();
    void setCompletionStatus();
    void setFrozenStatus();
    void scheduleFrozenOps();
    void onComplete();
    void calculateMakespan();
    void postUnaryResourceFS();
    void findResourceSequenceRuleApplications();
    void postResourceSequenceDelays();
    void propagate();

private:
    // data-set, constraint engine
    ClevorDataSet* _dataSet;
    clp::Manager* _mgr;
    clp::BoundPropagator* _bp;
    cls::Schedule* _schedule;
    bool _dataSetOwner;
    bool _initialized;
    bool _complete;

    // scheduler configuration
    const SchedulerConfiguration* _config;

    // dynamic list of schedulable jobs (see cse::Propagator::unsuspend)
    clp::RevArray<Job*> _sjobs;

    // misc
    int _makespan;
    int _frozenMakespan;
    uint_t _hardCtScore;
    uint_t _numScheduledOps;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;
