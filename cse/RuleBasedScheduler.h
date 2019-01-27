#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <cse/Scheduler.h>
#include <cse/SchedulingContext.h>
#include <cse/JobOpOrdering.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////
// RuleBasedScheduler //////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Choose operations for scheduling based on simple rules.

   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class RuleBasedScheduler : public Scheduler
{
    UTL_CLASS_DECL_ABC(RuleBasedScheduler, Scheduler);

public:
    virtual void copy(const utl::Object& rhs);

    virtual void
    run(gop::Ind* ind, gop::IndBuilderContext* context) const
    {
    }

    virtual void run(SchedulingContext* context) const;

protected:
    /** Initialize a run. */
    virtual void initRun(SchedulingContext* context) const;

    /** Select an op for scheduling. */
    virtual JobOp*
    selectOp(SchedulingContext* context) const
    {
        ABORT();
        return nullptr;
    }

    /** Set the op ordering. */
    void setOpOrdering(OpOrdering* opOrdering);

protected:
    mutable uint_t _jobSid;
    mutable uint_t _opSid;

    OpOrdering* _opOrdering;
    bool _setSid;

private:
    void init();
    void deInit();
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// JobLevelScheduler ///////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

class JobLevelScheduler : public RuleBasedScheduler
{
    UTL_CLASS_DECL(JobLevelScheduler, RuleBasedScheduler);

public:
    /** Copy another instance. */
    void copy(const utl::Object& rhs);

protected:
    /** Set the job ordering. */
    void setJobOrdering(JobOrdering* jobOrdering);

    /** Initialize a run. */
    virtual void initRun(SchedulingContext* context) const;

    /** Select an op for scheduling. */
    virtual JobOp* selectOp(SchedulingContext* context) const;

    /** Select a job for scheduling. */
    virtual void setSchedulableJobs(SchedulingContext* context) const;

    /** Add a schedulable object. */
    void sjobsAdd(utl::Object* obj) const;

    /** Release a job for scheduling. */
    virtual void releaseJob(Job* job, SchedulingContext* context) const;

protected:
    JobOrdering* _jobOrdering;
    mutable utl::RBtree _unreleasedJobs;
    bool _releaseJobs;

    // a dynamic list of schedulable jobs
    // controlled by JobLevelScheduler.
    // another similar list is in SchedulingContext.
    mutable utl::Object** _sjobs;
    mutable utl::Object** _sjobsPtr;
    mutable utl::Object** _sjobsLim;
    mutable size_t _sjobsSize;

private:
    void init();
    void deInit();
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// FrozenOpScheduler ///////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

class FrozenOpScheduler : public JobLevelScheduler
{
    UTL_CLASS_DECL(FrozenOpScheduler, JobLevelScheduler);

private:
    void init();
    void
    deInit()
    {
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// JobSequenceScheduler ////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

class JobSequenceScheduler : public JobLevelScheduler
{
    UTL_CLASS_DECL(JobSequenceScheduler, JobLevelScheduler);

private:
    void init();
    void
    deInit()
    {
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// OpSequenceScheduler /////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

class OpSequenceScheduler : public RuleBasedScheduler
{
    UTL_CLASS_DECL(OpSequenceScheduler, RuleBasedScheduler);

public:
    virtual void initRun(SchedulingContext* context) const;

    virtual JobOp* selectOp(SchedulingContext* context) const;

private:
    void init();
    void
    deInit()
    {
    }

private:
    mutable uint_t _idx;
    mutable utl::Array _ops;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// FwdScheduler ////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

class FwdScheduler : public JobLevelScheduler
{
    UTL_CLASS_DECL(FwdScheduler, JobLevelScheduler);

protected:
    virtual void initRun(SchedulingContext* context) const;
    virtual void releaseJob(Job* job, SchedulingContext* context) const;

private:
    void init();
    void
    deInit()
    {
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;
