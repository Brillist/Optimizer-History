#ifndef CSE_CLEVORDATASET_H
#define CSE_CLEVORDATASET_H

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <gop/DataSet.h>
#include <cse/Job.h>
#include <cse/JobGroup.h>
#include <cse/PrecedenceCt.h>
#include <cse/Resource.h>
#include <cse/ResourceGroup.h>
#include <cse/ResourceSequenceList.h>
#include <cse/SchedulerConfiguration.h>
#include <cse/MinCostHeuristics.h>
#include <cse/JobGroup.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Store a description of a scheduling problem.

   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class ClevorDataSet : public gop::DataSet
{
    UTL_CLASS_DECL(ClevorDataSet, gop::DataSet);

public:
    /** Copy another instance. */
    virtual void copy(const utl::Object& rhs);

    /// \name Propagation Model
    //@{
    /** Initialize the model (phase 0). */
    void modelBuild_0(cls::Schedule* schedule);

    /** Initialize the model (phase 1). */
    void modelBuild_1();

    /** Get the hard constraint score. */
    uint_t hardCtScore() const;

    /** Get the manager. */
    clp::Manager*
    manager() const
    {
        return _mgr;
    }

    /** Get the schedule. */
    cls::Schedule*
    schedule() const
    {
        return _schedule;
    }
    //@}

    /// \name Add objects
    //@{
    /** Set configuration. */
    void set(SchedulerConfiguration* config);

    /** Add a job. */
    void add(Job* job);

    /** Add a resource. */
    void add(Resource* res);

    /** Add a resource-group. */
    void add(ResourceGroup* resGroup);

    /** Add a resource-sequence-list. */
    void add(ResourceSequenceList* rsl);

    /** Add a precedence-ct. */
    void add(PrecedenceCt* pct);

    /** Add a job group. */
    void add(JobGroup* jobGroup);
    //@}

    /// \name Access collections
    //@{
    /** Get the list of jobs. */
    const job_set_id_t&
    jobs() const
    {
        return _jobs;
    }

    job_set_id_t&
    jobs()
    {
        return _jobs;
    }

    /** Get list of alternative jobs. */
    const jobgroup_set_id_t&
    jobGroups() const
    {
        return _jobGroups;
    }

    jobgroup_set_id_t&
    jobGroups()
    {
        return _jobGroups;
    }

    /** Get the list of ops. */
    const jobop_set_id_t&
    ops() const
    {
        return _ops;
    }

    /** Get the list of resources. */
    const res_set_id_t&
    resources() const
    {
        return _resources;
    }

    res_set_id_t&
    resources()
    {
        return _resources;
    }

    /** Get the list of resource-groups. */
    const resgroup_set_t&
    resourceGroups() const
    {
        return _resGroups;
    }

    resgroup_set_t&
    resourceGroups()
    {
        return _resGroups;
    }

    /** Get the list of resource-sequence-lists. */
    const rsl_set_t&
    resourceSequenceLists() const
    {
        return _rsls;
    }

    rsl_set_t&
    resourceSequenceLists()
    {
        return _rsls;
    }

    /** Get list of precedence-cts. */
    const pct_vector_t&
    precedenceCts() const
    {
        return _pcts;
    }

    pct_vector_t&
    precedenceCts()
    {
        return _pcts;
    }

    /** Get the list of schedulable ops. */
    const jobop_set_id_t&
    sops() const
    {
        return _sops;
    }

    /** Get list of ops ordered by decreasing successor-id. */
    const utl::TRBtree<JobOp>&
    opsDecSD() const
    {
        return _opsDecSD;
    }

    /** Get list of summary ops ordered by increasing successor-id. */
    const utl::TRBtree<JobOp>&
    summaryOpsIncSD() const
    {
        return _summaryOpsIncSD;
    }

    /** Get list of ops with hard constraints. */
    const utl::Array&
    hardCtOps() const
    {
        return _hardCtOps;
    }

    /** Get MinCostHeuristics. */
    const MinCostHeuristics*
    minCostHeuristics() const
    {
        return _minCostHeuristics;
    }
    //@}

    /// \name Access objects
    //@{
    /** Get scheduler configuration. */
    const SchedulerConfiguration*
    schedulerConfig() const
    {
        return _config;
    }

    /** Find a job by unique id. */
    Job* findJob(uint_t id) const;

    /** Find a job group by unique id. */
    JobGroup* findJobGroup(uint_t id) const;

    /** Find a job-op by unique id. */
    JobOp* findOp(uint_t id) const;

    /** Find a resource by unique id. */
    Resource* findResource(uint_t id) const;

    /** Find a resource-group by unique id. */
    ResourceGroup* findResourceGroup(uint_t id) const;

    /** Find a resource-sequence-list by unique id. */
    ResourceSequenceList* findResourceSequenceList(uint_t id) const;
    //@}

    void clearProblemData();

private:
    void init();
    void deInit();

    int timeToTimeSlot(time_t t) const;

    void createViews_0_0();
    void createViews_0_1();
    void createViews_1();
    void setRSLs();

    void propagate();

    // model: phase 0
    void modelBuildActivities_0();
    void modelBuildPrecedenceCts_0();
    // model: phase 1
    void modelBuildPrecedenceCts_1();
    void modelBuildActivities_1();
    void modelBuildUnaryCts();
    void modelBuildResources();
    void modelBuildDiscreteResources();
    void modelBuildDiscreteResourceCalendars();
    void modelBuildCompositeResources();
    void modelBuildResourceCts();
    void modelBuildDiscreteResourceCts();
    void modelBuildResourceGroupCts();
    void modelBuildCompositeResourceCts();
    void modelBuildHeuristics();

private:
    clp::Manager* _mgr;
    cls::Schedule* _schedule;
    clp::BoundPropagator* _bp;
    int _originTS;
    int _schedulingOriginTS;
    int _horizonTS;

    // problem data
    SchedulerConfiguration* _config;
    job_set_id_t _jobs;
    jobgroup_set_id_t _jobGroups;
    res_set_id_t _resources;
    resgroup_set_t _resGroups;
    rsl_set_t _rsls;
    pct_vector_t _pcts;

    // views
    jobop_set_id_t _ops;  //static list of all ops in _jobs
    jobop_set_id_t _sops; //static list of all sops in _jobs
    utl::Array _hardCtOps;
    utl::TRBtree<JobOp> _opsDecSD;
    utl::TRBtree<JobOp> _summaryOpsIncSD;

    // heuristics
    MinCostHeuristics* _minCostHeuristics;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
