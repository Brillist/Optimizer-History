#ifndef CSE_JOBOP_H
#define CSE_JOBOP_H

//////////////////////////////////////////////////////////////////////////////

#include <clp/CycleGroup.h>
#include <cls/BrkActivity.h>
#include <cls/IntActivity.h>
#include <cls/ResourceCapPts.h>
#include <cse/ResourceGroup.h>
#include <cse/ResourceRequirement.h>
#include <cse/ResourceGroupRequirement.h>
#include <cse/UnaryCt.h>
#include <cse/ItemRequirement.h>

//////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////

class ClevorDataSet;
class Job;
class JobOp;

//////////////////////////////////////////////////////////////////////////////

/** Scheduling agent. */
enum scheduling_agent_t
{
    sa_clevor = 0,    /**< Clevor scheduler */
    sa_external = 1,  /**< external agent */
    sa_undefined      /**< undefined/null */
};

//////////////////////////////////////////////////////////////////////////////

/** Operation type. */
enum operation_t
{
    op_normal = 0,          /**< normal op */
    op_summary = 1,         /**< summary op */
    op_milestone = 2,       /**< milestone op */
    op_precedenceLag = 3,   /**< precedence ct lag */
    op_interruptible = 4,   /**< interruptible op */
    op_undefined            /**< undefined/null */
};

//////////////////////////////////////////////////////////////////////////////

/** Operation status. */
enum operation_status_t
{
    opstatus_unstarted = 0,    /**< not started */
    opstatus_started = 1,      /**< started but not complete */
    opstatus_complete = 2,     /**< complete */
    opstatus_undefined,        /**< undefined/null */
};

//////////////////////////////////////////////////////////////////////////////

/**
   Operation belonging to a Job.

   \author Adam McKee
*/

//////////////////////////////////////////////////////////////////////////////

class JobOp : public utl::Object
{
    UTL_CLASS_DECL(JobOp);
public:
    /** Copy another instance. */
    virtual void copy(const utl::Object& rhs);

    virtual void serialize(
        utl::Stream& stream,
        utl::uint_t io,
        utl::uint_t mode = utl::ser_default);

    /// \name Accessors
    //@{
    /** Get the id. */
    utl::uint_t id() const
    { return _id; }

    /** Get the id. */
    utl::uint_t& id()
    { return _id; }

    /** Get the serial id. */
    utl::uint_t serialId() const
    { return _serialId; }

    /** Get the serial id. */
    utl::uint_t& serialId()
    { return _serialId; }

    /** Get the sequence id. */
    utl::uint_t sequenceId() const
    { return _sequenceId; }

    /** Get the serial id. */
    utl::uint_t& sequenceId()
    { return _sequenceId; }

    /** Get the name. */
    const std::string& name() const
    { return _name; }

    /** Get the name. */
    std::string& name()
    { return _name; }

    /** Get the job. */
    Job* job() const
    { return _job; }

    /** Get the job. */
    Job*& job()
    { return _job; }

    /** Get the cost. */
    double cost() const
    { return _cost; }

    /** Set the cost. */
    double& cost()
    { return _cost; }

    /** Get the processing-time. */
    utl::uint_t processingTime() const
    { return _processingTime; }

    /** Get the processing-time. */
    utl::uint_t& processingTime()
    { return _processingTime; }

    /** Get the type. */
    operation_t type() const
    { return _type; }

    /** Get the type. */
    operation_t& type()
    { return _type; }

    /** Get the status. */
    operation_status_t status() const
    { return _status; }

    /** Get the status. */
    operation_status_t& status()
    { return _status; }

    /** Get has-hard-ct flag. */
    bool hasHardCt() const
    { return _hasHardCt; }

    /** Get has-hard-ct flag. */
    bool& hasHardCt()
    { return _hasHardCt; }

    /** Get frozen flag. */
    bool frozen() const
    { return _frozen; }

    /** Get frozen flag. */
    bool& frozen()
    { return _frozen; }

    /** Get manually-frozen flag. */
    bool manuallyFrozen() const
    { return _manuallyFrozen; }

    /** Get manually-frozen flag. */
    bool& manuallyFrozen()
    { return _manuallyFrozen; }

    /** Get scheduling agent. */
    scheduling_agent_t scheduledBy() const
    { return _scheduledBy; }
    //@}

    /// \name previous scheduling result
    //@{
    /** Get scheduling agent. */
    scheduling_agent_t& scheduledBy()
    { return _scheduledBy; }

    /** Get scheduled processing-time. */
    utl::uint_t scheduledProcessingTime() const
    { return _scheduledProcessingTime; }

    /** Get scheduled processing-time. */
    utl::uint_t& scheduledProcessingTime()
    { return _scheduledProcessingTime; }

    /** Get scheduled remaining processing-time. */
    utl::uint_t scheduledRemainingPt() const
    { return _scheduledRemainingPt; }

    /** Get scheduled remaining processing-time. */
    utl::uint_t& scheduledRemainingPt()
    { return _scheduledRemainingPt; }

    /** Get scheduled start-time. */
    time_t scheduledStartTime() const
    { return _scheduledStartTime; }

    /** Get scheduled start-time. */
    time_t& scheduledStartTime()
    { return _scheduledStartTime; }

    /** Get scheduled end-time. */
    time_t scheduledEndTime() const
    { return _scheduledEndTime; }

    /** Get scheduled end-time. */
    time_t& scheduledEndTime()
    { return _scheduledEndTime; }

    /** Get scheduled resume-time. */
    time_t scheduledResumeTime() const
    { return _scheduledResumeTime; }

    /** Get scheduled resume-time. */
    time_t& scheduledResumeTime()
    { return _scheduledResumeTime; }
    //@}

    /// \name Unary constraints
    //@{
    /** Add a unaryCt. */ 
    void add(UnaryCt* uct)
    { _unaryCts.push_back(uct); }

    /** Get unaryCts. */
    const unaryct_vect_t& unaryCts() const
    { return _unaryCts; }

    /** Get min-start-time. */
    time_t minStartTime() const
    { return _minStartTime; }

    /** Get max-start-time. */
    time_t maxStartTime() const
    { return _maxStartTime; }

    /** Get min-end-time. */
    time_t minEndTime() const
    { return _minEndTime; }

    /** Get max-end-time. */
    time_t maxEndTime() const
    { return _maxEndTime; }

    void processUnaryCts();
    //@}

    /// \name Resource requirements
    //@{
    /** Owner of resource requirements? */
    bool isResReqOwner() const
    { return _resReqOwner; }

    /** Set resource requirement ownership flag. */
    void setResReqOwner(bool resReqOwner)
    { _resReqOwner = resReqOwner; }

    /** Clear the resource requirements. */
    void clearResReqs();

    /** Get the number of resource requirements. */
    utl::uint_t numResReqs() const
    { return _resReqs.size(); }

    /** Get a resource requirement by index. */
    const cse::ResourceRequirement* getResReq(utl::uint_t idx) const
    { return _resReqs[idx]; }

    /** Get a resource requirement by index. */
    cse::ResourceRequirement* getResReq(utl::uint_t idx)
    { return _resReqs[idx]; }

    /** Add a resource requirement. */
    void addResReq(cse::ResourceRequirement* resReq)
    { _resReqs.push_back(resReq); }

    /** Get the number of resource group requirements. */
    utl::uint_t numResGroupReqs() const
    { return _frozen ? 0 : _resGroupReqs.size(); }

    /** Get a resource group requirement by index. */
    const cse::ResourceGroupRequirement* getResGroupReq(
        utl::uint_t idx) const
    { return _resGroupReqs[idx]; }

    /** Get a resource group requirement by index. */
    cse::ResourceGroupRequirement* getResGroupReq(utl::uint_t idx)
    { return _resGroupReqs[idx]; }

    /** Add a resource group requirement. */
    void addResGroupReq(cse::ResourceGroupRequirement* resGroupReq)
    { _resGroupReqs.push_back(resGroupReq); }

    /** Add a resource-cap-pts object. */
    void add(cls::ResourceCapPts* resCapPts)
    { _resCapPts += resCapPts; }

    /** Get resource-cap-pts. */
    const cls::resCapPts_set_t& resCapPts() const
    { return _resCapPts; }

    /** Get adjusted resource-cap-pts. */
    const cls::resCapPts_set_t& resCapPtsAdj() const
    { return _resCapPtsAdj; }

    /** Get adjusted resource-cap-pts. */
    cls::resCapPts_set_t& resCapPtsAdj()
    { return _resCapPtsAdj; }

    /** Get resource-cap-pts for a given resource. */
    const cls::ResourceCapPts* resCapPts(utl::uint_t resId) const;

    /** Get adjusted resource-cap-pts for a given resource. */
    cls::ResourceCapPts* resCapPtsAdj(utl::uint_t resId) const;

    /** Make adjusted resource-cap-pts. */
    void setResCapPtsAdj(utl::uint_t timeStep);

    /** Remove adjusted resource-cap-pts for a given resource. */
    void removeResCapPtsAdj(utl::uint_t resId);

    /** Get resource-ids for all (possibly) required resources. */
    void getAllResIds(std::set<utl::uint_t>& resIds) const;
    //@}

    /// \name Item requirement
    //@{
    /** Get item requirements. */
    itemreq_vector_t& itemReqs()
    { return _itemReqs; }

    /** Add a item requirement. */
    void addItemReq(ItemRequirement* itemReq)
    { _itemReqs.push_back(itemReq); }
    //@}

    /// \name Predecence relationships
    //@{
    /** Get the ES-bound cycle-group. */
    const clp::CycleGroup* esCG() const
    { ASSERTD(_act != nullptr); return _act->esBound().cycleGroup(); }

    /** Get the ES-bound cycle-group. */
    clp::CycleGroup* esCG()
    { ASSERTD(_act != nullptr); return _act->esBound().cycleGroup(); }

    /** Get the LF-bound cycle-group. */
    const clp::CycleGroup* lfCG() const
    { ASSERTD(_act != nullptr); return _act->lfBound().cycleGroup(); }

    /** Get the LF-bound cycle-group. */
    clp::CycleGroup* lfCG()
    { ASSERTD(_act != nullptr); return _act->lfBound().cycleGroup(); }

    /** Get schedulable-ops index. */
    utl::uint_t& schedulableOpsIdx()
    { return _schedulableOpsIdx; }
    //@}

    /// \name Activity
    //@{
    /** Breakable? */
    bool breakable() const
    { return (dynamic_cast<cls::BrkActivity*>(_act) != nullptr); }

    /** Interruptible? */
    bool interruptible() const
    { return (dynamic_cast<cls::IntActivity*>(_act) != nullptr); }

    /** Get the activity object. */
    cls::Activity* activity() const
    { return _act; }

    /** Get the activity object. */
    cls::Activity*& activity()
    { return _act; }

    /** Get the breakable-activity object. */
    cls::BrkActivity* brkact() const
    {
        ASSERTD(breakable());
        return (cls::BrkActivity*)_act;
    }

    /** Get the interruptible-activity object. */
    cls::IntActivity* intact() const
    {
        ASSERTD(interruptible());
        return (cls::IntActivity*)_act;
    }
    //@}

    /** Get the data-set. */
    const ClevorDataSet* dataSet() const;

    /** Get schedulable flag (has res-req and not summary). */
    bool schedulable() const;

    /** Get ignorable flag (scheduled - but not within scheduling window). */
    bool ignorable() const;
    /** Remove scheduling information. */
    void unschedule();

    /** Unschedule res-reqs. */
    void unscheduleResReqs();

    /** Remove unsupported resource-requirements. */
    void removeImproperResReqs();

    /** Remove resource-requirements created by system. */
    void removeSystemResReqs();

    /** Is scheduled? */
    bool isScheduled() const;

    /** Resource requirements scheduled? */
    bool resReqsScheduled() const;

    /** Has resource requirements? */
    bool hasRequirements() const;

    /** Has requirement for resource capacity (non-zero cap & pt)? */
    bool hasCapacityRequirement() const;

    /** Start-time fixed? */
    bool isFixed() const;

    /** Get the successor depth for forward scheduling. */
    utl::uint_t FDsuccessorDepth() const
    { return esCG()->successorDepth(); }

    /** Get the successor depth for backward scheduling. */
    utl::uint_t BDsuccessorDepth() const
    { return lfCG()->successorDepth(); }

    utl::String toString() const;
private:
/*     typedef std::vector<UnaryCt*> unaryct_vector_t; */
    typedef std::set<utl::uint_t> uint_set_t;
private:
    void init();
    void deInit();

    void removeResReqs(const uint_set_t& resIds);

    void getSystemResIds(uint_set_t& resIds) const;

    void getCompositeResIds(uint_set_t& resIds) const;

    void getDiscreteResIds(uint_set_t& resIds) const;

    void getResGroupResIds(uint_set_t& resIds) const;
private:
    utl::uint_t _id;
    utl::uint_t _serialId;
    utl::uint_t _sequenceId;
    std::string _name;
    Job* _job;
    double _cost;
    utl::uint_t _processingTime;

    operation_t _type;
    operation_status_t _status;

    bool _hasHardCt;
    bool _frozen;
    bool _manuallyFrozen;

    // previous scheduling result
    scheduling_agent_t _scheduledBy;
    utl::uint_t _scheduledProcessingTime;
    utl::uint_t _scheduledRemainingPt;
    time_t _scheduledStartTime;
    time_t _scheduledEndTime;
    time_t _scheduledResumeTime;

    // unary constraints
    unaryct_vect_t _unaryCts;
    time_t _minStartTime;
    time_t _maxStartTime;
    time_t _minEndTime;
    time_t _maxEndTime;

    // resource requirements
    bool _resReqOwner;
    resReq_vector_t _resReqs;
    resGroupReq_vector_t _resGroupReqs;
    cls::resCapPts_set_t _resCapPts;
    mutable cls::resCapPts_set_t _resCapPtsAdj;

    // item requirements
    // note: _itemReqs is added for inventory management in mrp.
    itemreq_vector_t _itemReqs;

    // schedulable-ops list index
    utl::uint_t _schedulableOpsIdx;

    // activity, start & end times
    cls::Activity* _act;
};

//////////////////////////////////////////////////////////////////////////////

struct JobOpIdOrdering
    : public std::binary_function<JobOp*,JobOp*,bool>
{
    bool operator()(const JobOp* lhs, const JobOp* rhs) const
    {
        return (lhs->id() < rhs->id());
    }
};

//////////////////////////////////////////////////////////////////////////////

struct JobOpSerialIdOrdering
    : public std::binary_function<JobOp*,JobOp*,bool>
{
    bool operator()(const JobOp* lhs, const JobOp* rhs) const
    {
        return (lhs->serialId() < rhs->serialId());
    }
};

//////////////////////////////////////////////////////////////////////////////

struct JobOpFDsuccessorDepthDecOrdering
   : public std::binary_function<JobOp*,JobOp*,bool>
{
   bool operator()(const JobOp* lhs, const JobOp* rhs) const
   {
      return (lhs->FDsuccessorDepth() > rhs->FDsuccessorDepth());
   }
};

//////////////////////////////////////////////////////////////////////////////

struct JobOpBDsuccessorDepthDecOrdering
   : public std::binary_function<JobOp*,JobOp*,bool>
{
   bool operator()(const JobOp* lhs, const JobOp* rhs) const
   {
      return (lhs->BDsuccessorDepth() > rhs->BDsuccessorDepth());
   }
};

//////////////////////////////////////////////////////////////////////////////

typedef std::vector<JobOp*> jobop_vector_t;
typedef std::set<JobOp*, JobOpIdOrdering> jobop_set_id_t;
typedef std::set<JobOp*, JobOpSerialIdOrdering> jobop_set_sid_t;

//////////////////////////////////////////////////////////////////////////////

CSE_NS_END;

//////////////////////////////////////////////////////////////////////////////

#endif
