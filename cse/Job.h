#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <clp/CycleGroup.h>
#include <clp/RevArray.h>
#include <cse/JobOp.h>
#include <cse/SummaryOp.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

class ClevorDataSet;

////////////////////////////////////////////////////////////////////////////////////////////////////

/** Job status. */
enum job_status_t
{
    jobstatus_inactive = 0,  /**< not active / selected */
    jobstatus_planned = 1,   /**< active / selected */
    jobstatus_confirmed = 2, /**< confirmed */
    jobstatus_started = 3,   /** < started */
    jobstatus_complete = 4,  /** < complete */
    jobstatus_undefined,     /**< undefined/null */
};

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Job within a Schedule.

   A job is a grouping of operations (instances of JobOp).

   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class Job : public utl::Object
{
    UTL_CLASS_DECL(Job, utl::Object);

public:
    // typedefs
    typedef jobop_set_id_t::iterator iterator;
    typedef jobop_set_id_t::const_iterator const_iterator;

public:
    virtual void copy(const utl::Object& rhs);

    virtual int compare(const utl::Object& rhs) const;

    virtual void serialize(utl::Stream& stream, uint_t io, uint_t mode = utl::ser_default);

    /** Remove all operations. */
    void clear();

    /** Get the data-set. */
    const ClevorDataSet*
    dataSet() const
    {
        return _dataSet;
    }

    /** Get the data-set. */
    const ClevorDataSet*&
    dataSet()
    {
        return (const ClevorDataSet*&)_dataSet;
    }

    /** Get the id. */
    uint_t
    id() const
    {
        return _id;
    }

    /** Get the id. */
    uint_t&
    id()
    {
        return _id;
    }

    /** Get the name. */
    const std::string&
    name() const
    {
        return _name;
    }

    /** Get the name */
    std::string&
    name()
    {
        return _name;
    }

    /** Get the serial-id. */
    uint_t
    serialId() const
    {
        return _serialId;
    }

    /** Get the serial-id. */
    uint_t&
    serialId()
    {
        return _serialId;
    }

    /** Get the preference. */
    uint_t
    preference() const
    {
        return _preference;
    }

    /** Get the preference. */
    uint_t&
    preference()
    {
        return _preference;
    }

    /** Get the active bool. */
    const bool
    active() const
    {
        return _active;
    }

    /** Get the active bool. */
    bool&
    active()
    {
        return _active;
    }

    /** the point to _active. */
    bool*
    activeP()
    {
        return &_active;
    }

    /** Get job group id. */
    uint_t
    groupId() const
    {
        return _groupId;
    }

    /** Get job group id. */
    uint_t&
    groupId()
    {
        return _groupId;
    }

    /** Get a set of workorder ids. */
    lut::uint_set_t
    workOrderIds() const
    {
        return _workOrderIds;
    }

    /** Get a set of workorder ids. */
    lut::uint_set_t&
    workOrderIds()
    {
        return _workOrderIds;
    }

    /** Get the root summary op. */
    SummaryOp*
    rootSummaryOp() const
    {
        return _rootSummaryOp;
    }

    /** Get the root summary op. */
    SummaryOp*&
    rootSummaryOp()
    {
        return _rootSummaryOp;
    }

    /// \name Schedulable Ops
    //@{
    /** Get list of all schedulable ops. */
    const jobop_set_id_t&
    allSops() const
    {
        return _allSops;
    }

    /** Job is complete? */
    /*     bool complete() const */
    /*     { return (_sops.size() == 0); } */

    /** Initialize schedulable-ops lists. */
    void sopsInit(clp::Manager* mgr);

    /** Add to the list of schedulable-ops. */
    void sopsAdd(JobOp* op);

    /** Remove from the list of schedulable-ops. */
    void sopsRemove(JobOp* op);

    /** Prepare to generate new schedule. */
    void scheduleClear();

    /** Schedule an operation. */
    /*     void schedule(JobOp* op, bool forward); */

    /** Calculate _makespan and _frozenMakespan. */
    void calculateMakespan(bool forward);

    /** Schedulable-jobs empty? */
    bool
    sopsEmpty() const
    {
        return (_sops.begin() == _sops.end());
    }

    /** Get begin iterator for schedulable ops. */
    JobOp**
    sopsBegin() const
    {
        return (JobOp**)_sops.begin();
    }

    /** Get end iterator for schedulable ops. */
    JobOp**
    sopsEnd() const
    {
        return (JobOp**)_sops.end();
    }
    //@}

    /** Get item id. */
    uint_t
    itemId() const
    {
        return _itemId;
    }

    /** Get item id. */
    uint_t&
    itemId()
    {
        return _itemId;
    }

    /** Get item quantity. */
    uint_t
    itemQuantity() const
    {
        return _itemQuantity;
    }

    /** Get item quantity. */
    uint_t&
    itemQuantity()
    {
        return _itemQuantity;
    }

    /** Get the completion time of this job. */
    int
    makespan() const
    {
        return _makespan;
    }

    /** Get _frozenMakespan. */
    int
    frozenMakespan() const
    {
        return _frozenMakespan;
    }

    /** Get the tardiness. */
    uint_t tardiness() const;

    /** Get the release time. */
    int
    releaseTime() const
    {
        return _releaseTime;
    }

    /** Get the release time. */
    int&
    releaseTime()
    {
        return _releaseTime;
    }

    /** Get the due time. */
    int
    dueTime() const
    {
        return _dueTime;
    }

    /** Get the due time. */
    int&
    dueTime()
    {
        return _dueTime;
    }

    /** Get the status. */
    job_status_t
    status() const
    {
        return _status;
    }

    /** Get the status. */
    job_status_t&
    status()
    {
        return _status;
    }

    /** Get the opportunity cost. */
    double
    opportunityCost() const
    {
        return _opportunityCost;
    }

    /** Get the opportunity cost. */
    double&
    opportunityCost()
    {
        return _opportunityCost;
    }

    /** Get the opportunity cost period. */
    lut::period_t
    opportunityCostPeriod() const
    {
        return _opportunityCostPeriod;
    }

    /** Get the opportunity cost period. */
    lut::period_t&
    opportunityCostPeriod()
    {
        return _opportunityCostPeriod;
    }

    /** Get the lateness cost. */
    double
    latenessCost() const
    {
        return _latenessCost;
    }

    /** Get the lateness cost. */
    double&
    latenessCost()
    {
        return _latenessCost;
    }

    /** November 21, 2013 (Elisa)*/
    /** Get the lateness cost increment. */
    double
    latenessIncrement() const
    {
        return _latenessIncrement;
    }

    /** November 21, 2013 (Elisa) */
    /*  Get the lateness cost increment. */
    double&
    latenessIncrement()
    {
        return _latenessIncrement;
    }

    /** Get the lateness cost period. */
    lut::period_t
    latenessCostPeriod() const
    {
        return _latenessCostPeriod;
    }

    /** Get the lateness cost period. */
    lut::period_t&
    latenessCostPeriod()
    {
        return _latenessCostPeriod;
    }

    /** January 2, 2014 (Elisa) */
    /* Get the overhead cost */
    double
    overheadCost() const
    {
        return _overheadCost;
    }

    /* Get the overhead cost */
    double&
    overheadCost()
    {
        return _overheadCost;
    }

    /** January 2, 2014 (Elisa) */
    /* Get the overhead cost period */
    lut::period_t
    overheadCostPeriod() const
    {
        return _overheadCostPeriod;
    }

    /* Get the overhead cost period */
    lut::period_t&
    overheadCostPeriod()
    {
        return _overheadCostPeriod;
    }
    /** Get the inventory cost. */
    double
    inventoryCost() const
    {
        return _inventoryCost;
    }

    /** Get the inventory cost. */
    double&
    inventoryCost()
    {
        return _inventoryCost;
    }

    /** Get the inventory cost period. */
    lut::period_t
    inventoryCostPeriod() const
    {
        return _inventoryCostPeriod;
    }

    /** Get the inventory cost period. */
    lut::period_t&
    inventoryCostPeriod()
    {
        return _inventoryCostPeriod;
    }

    /** Add an operation. */
    void
    add(const JobOp& op)
    {
        add((JobOp&)op);
    }

    /** Add an operation. */
    void
    add(JobOp& op)
    {
        add(op.clone());
    }

    /** Add an operation. */
    void add(JobOp* op);

    /** Get begin iterator (const). */
    const_iterator
    begin() const
    {
        return _ops.begin();
    }

    /** Get end iterator (const). */
    const_iterator
    end() const
    {
        return _ops.end();
    }

    /** Get begin iterator. */
    iterator
    begin()
    {
        return _ops.begin();
    }

    /** Get end iterator. */
    iterator
    end()
    {
        return _ops.end();
    }

    /// \name Precedence relationships
    //@{
    /** Initialize precedence-op. */
    void initPrecedenceBound(clp::Manager* mgr);

    /** Initialize release-op. */
    void initReleaseBound(clp::Manager* mgr);

    /** Release this job. */
    void release(clp::Manager* mgr);

    /** Is self released? */
    bool released() const;

    /** Get cycle-group. */
    const clp::CycleGroup*
    cycleGroup() const
    {
        return _precedenceBound->cycleGroup();
    }

    /** Get cycle-group. */
    clp::CycleGroup*
    cycleGroup()
    {
        return _precedenceBound->cycleGroup();
    }

    /** Determine the relationship between self and the given job. */
    clp::cg_precedence_rel_t
    relationship(const Job* job) const
    {
        return cycleGroup()->relationship(job->cycleGroup());
    }

    /** Get the successor depth. */
    uint_t
    successorDepth() const
    {
        return cycleGroup()->successorDepth();
    }

    /** Get schedulable-jobs index. */
    uint_t&
    schedulableJobsIdx()
    {
        return _schedulableJobsIdx;
    }
    //@}

    /** finalize all schedulable constrained bounds of the job. */
    void finalize(clp::Manager* mgr);

    String toString() const;

protected:
    ClevorDataSet* _dataSet;
    uint_t _id;
    uint_t _serialId;
    uint_t _preference; //for alt jobs
    bool _active;       //indicate the selected job from alt jobs
    std::string _name;
    uint_t _groupId;
    lut::uint_set_t _workOrderIds; //ids of mrp::workorders
    jobop_set_id_t _ops;
    jobop_set_id_t _allSops;   // a static list of schedulable ops
    SummaryOp* _rootSummaryOp; //at least MRP uses it.
    clp::ConstrainedBound* _precedenceBound;
    clp::ConstrainedBound* _releaseBound;

    // a dynamic list of schedulable ops
    // controlled by cse::Propagator::unsuspend()
    clp::RevArray<JobOp*> _sops;

    uint_t _itemId;       // for MRP GUI
    uint_t _itemQuantity; // for MRP GUI
    int _makespan;
    int _frozenMakespan;
    int _releaseTime; // for MRP GUI and more
    int _dueTime;
    job_status_t _status;
    double _opportunityCost;
    lut::period_t _opportunityCostPeriod;
    double _latenessCost;
    lut::period_t _latenessCostPeriod;
    // November 21, 2013 (Elisa)
    // add variable to hold the lateness cost increment
    double _latenessIncrement;
    double _inventoryCost;
    lut::period_t _inventoryCostPeriod;
    // January 2, 2014 (Elisa)
    // add variables to hold the overhead dost and period
    double _overheadCost;
    lut::period_t _overheadCostPeriod;

    // predecessors, successors
    uint_t _schedulableJobsIdx;

private:
    void init();
    void deInit();
};

////////////////////////////////////////////////////////////////////////////////////////////////////

struct JobIdOrdering
{
    bool
    operator()(const Job* lhs, const Job* rhs) const
    {
        return (lhs->id() < rhs->id());
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////

struct JobSerialIdOrdering
{
    bool
    operator()(const Job* lhs, const Job* rhs) const
    {
        return (lhs->serialId() < rhs->serialId());
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////

struct JobPreferenceOrdering
{
    bool
    operator()(const Job* lhs, const Job* rhs) const
    {
        return (lhs->preference() > rhs->preference());
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////

struct JobDueTimeOrdering
{
    bool
    operator()(const Job* lhs, const Job* rhs) const
    {
        return (lhs->dueTime() < rhs->dueTime());
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////

struct JobSuccessorDepthDecOrdering
{
    bool
    operator()(const Job* lhs, const Job* rhs) const
    {
        return (lhs->successorDepth() > rhs->successorDepth());
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////

struct JobLatenessCostDecOrdering
{
    bool
    operator()(const Job* lhs, const Job* rhs) const
    {
        return (lhs->latenessCost() > rhs->latenessCost());
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////

typedef std::vector<Job*> job_vector_t;
typedef std::set<Job*, JobIdOrdering> job_set_id_t;
typedef std::set<Job*, JobSerialIdOrdering> job_set_sid_t;
typedef std::set<Job*, JobPreferenceOrdering> job_set_pref_t;

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;
