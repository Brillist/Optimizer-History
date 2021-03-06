#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////
// InterestPeriodInfo //////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

class InterestPeriodInfo : public utl::Object
{
    UTL_CLASS_DECL(InterestPeriodInfo, utl::Object);
    UTL_CLASS_DEFID;

public:
    virtual void copy(const utl::Object& rhs);

    virtual void serialize(utl::Stream& stream, uint_t io, uint_t mode = utl::ser_default);

public:
    uint_t idx;
    time_t begin;
    time_t end;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

typedef std::vector<InterestPeriodInfo*> ipInfos_t;

////////////////////////////////////////////////////////////////////////////////////////////////////
// ResourceInfo ////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

struct ResourceInfo : public utl::Object
{
    UTL_CLASS_DECL(ResourceInfo, utl::Object);
    UTL_CLASS_DEFID;

public:
    virtual void copy(const utl::Object& rhs);

    virtual void serialize(utl::Stream& stream, uint_t io, uint_t mode = utl::ser_default);

public:
    uint_t id;
    std::string name;
    uint_t resolutionSlots;
    uint_t maxIdleSlots;
    uint_t minEmploySlots;
    double costPerUnitHired;
    double costPerHour;
    double costPerDay;
    double costPerWeek;
    double costPerMonth;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

typedef std::map<uint_t, ResourceInfo*> resInfos_t;

////////////////////////////////////////////////////////////////////////////////////////////////////
// FixedCostInfo ///////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

struct FixedCostInfo : public utl::Object
{
    UTL_CLASS_DECL(FixedCostInfo, utl::Object);
    UTL_CLASS_DEFID;

public:
    virtual void copy(const utl::Object& rhs);

    virtual void serialize(utl::Stream& stream, uint_t io, uint_t mode = utl::ser_default);

public:
    uint_t opId;
    uint_t interestPeriod;
    double cost;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

typedef std::map<uint_t, FixedCostInfo*> fixedInfos_t;

////////////////////////////////////////////////////////////////////////////////////////////////////
// OverheadCostInfo ////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

struct OverheadCostInfo : public utl::Object
{
    UTL_CLASS_DECL(OverheadCostInfo, utl::Object);
    UTL_CLASS_DEFID;

public:
    virtual void copy(const utl::Object& rhs);

    virtual void serialize(utl::Stream& stream, uint_t io, uint_t mode = utl::ser_default);

public:
    uint_t interestPeriod;
    double cost;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

using overheadInfos_t = std::vector<OverheadCostInfo*>;

////////////////////////////////////////////////////////////////////////////////////////////////////
// InterestCostInfo ////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

struct InterestCostInfo : public utl::Object
{
    UTL_CLASS_DECL(InterestCostInfo, utl::Object);
    UTL_CLASS_DEFID;

public:
    virtual void copy(const utl::Object& rhs);

    virtual void serialize(utl::Stream& stream, uint_t io, uint_t mode = utl::ser_default);

public:
    uint_t interestPeriod;
    double cost;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

typedef std::vector<InterestCostInfo*> interestInfos_t;

////////////////////////////////////////////////////////////////////////////////////////////////////
// ResourceUsageInfo ///////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

struct ResourceUsageInfo : public utl::Object
{
    UTL_CLASS_DECL(ResourceUsageInfo, utl::Object);
    UTL_CLASS_DEFID;

public:
    virtual void copy(const utl::Object& rhs);

    virtual void serialize(utl::Stream& stream, uint_t io, uint_t mode = utl::ser_default);

public:
    uint_t id;
    uint_t cap;
    time_t begin;
    time_t end;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

typedef std::vector<ResourceUsageInfo*> usageInfos_t;

////////////////////////////////////////////////////////////////////////////////////////////////////
// ComponentScoreInfo //////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

// FIXME:  Better design is to make this a report and hook up the
// appropriate containers.
struct ComponentScoreInfo : public utl::Object
{
    UTL_CLASS_DECL(ComponentScoreInfo, utl::Object);
    UTL_CLASS_DEFID;

public:
    virtual void copy(const utl::Object& rhs);

    virtual void serialize(utl::Stream& stream, uint_t io, uint_t mode = utl::ser_default);

public:
    std::string name;
    double score;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

typedef std::vector<ComponentScoreInfo*> componentInfos_t;

////////////////////////////////////////////////////////////////////////////////////////////////////
// ResourceSequenceCostInfo ////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

struct ResourceSequenceCostInfo : public utl::Object
{
    UTL_CLASS_DECL(ResourceSequenceCostInfo, utl::Object);
    UTL_CLASS_DEFID;

public:
    virtual void copy(const utl::Object& rhs);

    virtual void serialize(utl::Stream& stream, uint_t io, uint_t mode = utl::ser_default);

public:
    uint_t id;
    uint_t lhsId;
    uint_t rhsId;
    double cost;
    uint_t ip;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

typedef std::vector<ResourceSequenceCostInfo*> resourceSequenceCostInfos_t;

////////////////////////////////////////////////////////////////////////////////////////////////////
// OperationInfo ///////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

struct OperationInfo : public utl::Object
{
    UTL_CLASS_DECL(OperationInfo, utl::Object);
    UTL_CLASS_DEFID;

public:
    virtual void copy(const utl::Object& rhs);

    virtual void serialize(utl::Stream& stream, uint_t io, uint_t mode = utl::ser_default);

public:
    uint_t id;
    std::string name;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

typedef std::map<uint_t, OperationInfo*> opInfos_t;

////////////////////////////////////////////////////////////////////////////////////////////////////
// ResourceCostInfo ////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

struct ResourceCostInfo : public utl::Object
{
    UTL_CLASS_DECL(ResourceCostInfo, utl::Object);
    UTL_CLASS_DEFID;

public:
    virtual void copy(const utl::Object& rhs);

    virtual void serialize(utl::Stream& stream, uint_t io, uint_t mode = utl::ser_default);

public:
    uint_t cap;
    time_t date;
    uint_t hiredCap;
    double hireCost;
    lut::period_t bestRate;
    double workCost;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

typedef std::vector<ResourceCostInfo*> rcInfos_t;

////////////////////////////////////////////////////////////////////////////////////////////////////
// ResourceWorkHoursInfo ///////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

struct ResourceWorkHoursInfo : public utl::Object
{
    UTL_CLASS_DECL(ResourceWorkHoursInfo, utl::Object);
    UTL_CLASS_DEFID;

public:
    virtual void copy(const utl::Object& rhs);

    virtual void serialize(utl::Stream& stream, uint_t io, uint_t mode = utl::ser_default);

public:
    uint_t cap;
    time_t date;
    uint_t minutes;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

typedef std::vector<ResourceWorkHoursInfo*> wkhrsInfos_t;

////////////////////////////////////////////////////////////////////////////////////////////////////
// LatenessCostInfo ////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

struct LatenessCostInfo : public utl::Object
{
    UTL_CLASS_DECL(LatenessCostInfo, utl::Object);
    UTL_CLASS_DEFID;

public:
    virtual void copy(const utl::Object& rhs);

    virtual void serialize(utl::Stream& stream, uint_t io, uint_t mode = utl::ser_default);

public:
    uint_t interestPeriod;
    double opportunityCost;
    double latenessCost;
    double inventoryCost;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

typedef std::vector<LatenessCostInfo*> lcInfos_t;

////////////////////////////////////////////////////////////////////////////////////////////////////
// JobOverheadCostInfo /////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

struct JobOverheadCostInfo : public utl::Object
{
    UTL_CLASS_DECL(JobOverheadCostInfo, utl::Object);
    UTL_CLASS_DEFID;

public:
    virtual void copy(const utl::Object& rhs);

    virtual void serialize(utl::Stream& steam, uint_t io, uint_t mode = utl::ser_default);

public:
    uint_t interestPeriod;
    double cost;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

typedef std::vector<JobOverheadCostInfo*> joboverheadInfos_t;

////////////////////////////////////////////////////////////////////////////////////////////////////
// ResourceCostReport //////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

class ResourceCostReport : public utl::Object
{
    UTL_CLASS_DECL(ResourceCostReport, utl::Object);

public:
    virtual void copy(const utl::Object& rhs);

    virtual void serialize(utl::Stream& stream, uint_t io, uint_t mode = utl::ser_default);

public:
    uint_t
    getId() const
    {
        return _id;
    }

    void setId(uint_t id);

    rcInfos_t*
    costs()
    {
        return &_costs;
    }

    const rcInfos_t*
    costs() const
    {
        return &_costs;
    }

    wkhrsInfos_t*
    workHours()
    {
        return &_workHours;
    }

    const wkhrsInfos_t*
    workHours() const
    {
        return &_workHours;
    }

private:
    void init();
    void deInit();

private:
    uint_t _id;              // resource id
    rcInfos_t _costs;        // cost info for this resource
    wkhrsInfos_t _workHours; // work hours for this reosurce
};

////////////////////////////////////////////////////////////////////////////////////////////////////

typedef std::map<uint_t, ResourceCostReport*> resourceReports_t;

////////////////////////////////////////////////////////////////////////////////////////////////////
// LatenessCostReport //////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

class LatenessCostReport : public utl::Object
{
    UTL_CLASS_DECL(LatenessCostReport, utl::Object);

public:
    virtual void copy(const utl::Object& rhs);

    virtual void serialize(utl::Stream& stream, uint_t io, uint_t mode = utl::ser_default);

public:
    uint_t
    getId() const
    {
        return _id;
    }

    void
    setId(uint_t id)
    {
        _id = id;
    }

    std::string
    getName() const
    {
        return _name;
    }

    void
    setName(std::string name)
    {
        _name = name;
    }

    lcInfos_t*
    costs()
    {
        return &_costs;
    }

    const lcInfos_t*
    costs() const
    {
        return &_costs;
    }

private:
    void init();
    void deInit();

private:
    // work order id;
    uint_t _id;
    // work order name;
    std::string _name;
    // lateness cost for each ip
    lcInfos_t _costs;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

typedef std::map<uint_t, LatenessCostReport*> latenessReports_t;

////////////////////////////////////////////////////////////////////////////////////////////////////
// JobOverheadCostReport ///////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

class JobOverheadCostReport : public utl::Object
{
    UTL_CLASS_DECL(JobOverheadCostReport, utl::Object);

public:
    virtual void copy(const utl::Object& rhs);

    virtual void serialize(utl::Stream& stream, uint_t io, uint_t mode = utl::ser_default);

public:
    uint_t
    getId() const
    {
        return _id;
    }

    void
    setId(uint_t id)
    {
        _id = id;
    }

    std::string
    getName() const
    {
        return _name;
    }

    void
    setName(std::string name)
    {
        _name = name;
    }

    joboverheadInfos_t*
    costs()
    {
        return &_costs;
    }

    const joboverheadInfos_t*
    costs() const
    {
        return &_costs;
    }

private:
    void init();
    void deInit();

private:
    // work order id;
    uint_t _id;
    // work order name;
    std::string _name;
    // job overhead cost for each ip
    joboverheadInfos_t _costs;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

typedef std::map<uint_t, JobOverheadCostReport*> joboverheadReports_t;

////////////////////////////////////////////////////////////////////////////////////////////////////
// AuditReport /////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

class AuditReport : public utl::Object
{
    UTL_CLASS_DECL(AuditReport, utl::Object);

public:
    virtual void copy(const utl::Object& rhs);

    virtual void serialize(utl::Stream& stream, uint_t io, uint_t mode = utl::ser_default);

    std::string
    getName() const
    {
        return _name;
    }

    void setName(std::string name);

    double
    getScore() const
    {
        return _score;
    }

    void setScore(double score);

    time_t
    getOriginTime() const
    {
        return _originTime;
    }

    void setOriginTime(time_t t);

    time_t
    getHorizonTime() const
    {
        return _horizonTime;
    }

    void setHorizonTime(time_t t);

    uint_t
    getTimeStep() const
    {
        return _timeStep;
    }

    void setTimeStep(uint_t step);

    ipInfos_t*
    interestPeriods()
    {
        return &_interestPeriods;
    }

    const ipInfos_t*
    interestPeriods() const
    {
        return &_interestPeriods;
    }

    resInfos_t*
    resourceInfos()
    {
        return &_resInfos;
    }

    const resInfos_t*
    resourceInfos() const
    {
        return &_resInfos;
    }

    resourceReports_t*
    resourceCosts()
    {
        return &_resourceCosts;
    }

    const resourceReports_t*
    resourceCosts() const
    {
        return &_resourceCosts;
    }

    ResourceCostReport& resourceCost(uint_t);

    latenessReports_t*
    latenessCosts()
    {
        return &_latenessCosts;
    }

    const latenessReports_t*
    latenessCosts() const
    {
        return &_latenessCosts;
    }

    LatenessCostReport& latenessCost(uint_t);

    joboverheadReports_t*
    joboverheadCosts()
    {
        return &_joboverheadCosts;
    }

    const joboverheadReports_t*
    joboverheadCosts() const
    {
        return &_joboverheadCosts;
    }

    JobOverheadCostReport& joboverheadCost(uint_t);

    fixedInfos_t*
    fixedCosts()
    {
        return &_fixedCosts;
    }

    const fixedInfos_t*
    fixedCosts() const
    {
        return &_fixedCosts;
    }

    overheadInfos_t*
    overheadCosts()
    {
        return &_overheadCosts;
    }

    const overheadInfos_t*
    overheadCosts() const
    {
        return &_overheadCosts;
    }

    interestInfos_t*
    interestCosts()
    {
        return &_interestCosts;
    }

    const interestInfos_t*
    interestCosts() const
    {
        return &_interestCosts;
    }

    usageInfos_t*
    resourceUsages()
    {
        return &_resourceUsages;
    }

    const usageInfos_t*
    resourceUsages() const
    {
        return &_resourceUsages;
    }

    componentInfos_t*
    componentInfos()
    {
        return &_componentInfos;
    }

    const componentInfos_t*
    componentInfos() const
    {
        return &_componentInfos;
    }

    opInfos_t*
    operationInfos()
    {
        return &_operationInfos;
    }

    const opInfos_t*
    operationInfos() const
    {
        return &_operationInfos;
    }

    resourceSequenceCostInfos_t*
    resourceSequenceCosts()
    {
        return &_resourceSequenceCosts;
    }

    const resourceSequenceCostInfos_t*
    resourceSequenceCosts() const
    {
        return &_resourceSequenceCosts;
    }

private:
    void init();
    void deInit();

private:
    std::string _name;

    // scheduler config
    time_t _originTime;
    time_t _horizonTime;
    uint_t _timeStep;

    double _score;

    // interest period info
    ipInfos_t _interestPeriods;
    // info for each resource
    resInfos_t _resInfos;
    // resource costs
    resourceReports_t _resourceCosts;
    // lateness costs
    latenessReports_t _latenessCosts;
    // fixed costs
    fixedInfos_t _fixedCosts;
    // overhead costs for each period
    overheadInfos_t _overheadCosts;
    // job overhead costs
    joboverheadReports_t _joboverheadCosts;
    // interest costs for each period
    interestInfos_t _interestCosts;
    // resource utilization information
    usageInfos_t _resourceUsages;
    // component name and scores
    componentInfos_t _componentInfos;
    // operation information
    opInfos_t _operationInfos;
    // resource sequence costs
    resourceSequenceCostInfos_t _resourceSequenceCosts;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;
