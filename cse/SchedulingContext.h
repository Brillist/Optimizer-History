#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <gop/IndBuilderContext.h>
#include <cls/Schedule.h>
#include <cse/ClevorDataSet.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Maintain state for schedule-building process.

   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class SchedulingContext : public gop::IndBuilderContext
{
    UTL_CLASS_DECL(SchedulingContext, gop::IndBuilderContext);
    UTL_CLASS_NO_COPY;

public:
    /** Get the data-set. */
    const gop::DataSet*
    dataSet() const
    {
        return _dataSet;
    }

    /** Get the data-set. */
    const cse::ClevorDataSet*
    clevorDataSet() const
    {
        return _dataSet;
    }

    cse::ClevorDataSet*
    clevorDataSet()
    {
        return _dataSet;
    }

    /** Get the manager. */
    const clp::Manager*
    manager() const
    {
        return _mgr;
    }

    /** Get the manager. */
    clp::Manager*
    manager()
    {
        return _mgr;
    }

    /** Get the schedule. */
    const cls::Schedule*
    schedule() const
    {
        return _schedule;
    }

    /** Get the schedule. */
    cls::Schedule*
    schedule()
    {
        return _schedule;
    }

    /** Initialize. */
    void initialize(ClevorDataSet* dataSet);

    /** Clear (prepare to generate new schedule). */
    virtual void clear();

    /** Schedule the given operation. */
    void schedule(JobOp* op);

    /** Scheduling complete? */
    bool
    complete() const
    {
        return _complete;
    } //(_sjobs.size() == 0); }

    /** Set scheduling complete status. */
    void setComplete(bool complete);
    /*     { _complete = complete; } */

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

    /** Store the schedule into the data-set. */
    void store();

    /** Get the makespan. */
    time_t
    makespan() const
    {
        ASSERTD(_config != nullptr);
        return _config->timeSlotToTime(_makespan);
    }

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

    /** Get the makespan time-slot. */
    int
    makespanTimeSlot() const
    {
        return _makespan;
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

    void
    setDataSetOwner(bool owner)
    {
        _dataSetOwner = owner;
    }

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
    bool _complete;
    bool _afterInitialization;

    // scheduler configuration
    const SchedulerConfiguration* _config;

    // list of all schedulable ops
    // This seems obsolete.
    // Joe, Oct 10, 2006
    /*     jobop_set_id_t _allSops; */

    // a dynamic list of schedulable jobs
    // controlled by cse::Propagator::unsuspend().
    clp::RevArray<Job*> _sjobs;

    // misc
    int _makespan;
    int _frozenMakespan;
    uint_t _hardCtScore;
    uint_t _numScheduledOps;

    // control who will be responsible for
    // deleting _dataSet, SchedulingContext or MRPrun.
    bool _dataSetOwner;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;
