#include "libcse.h"
#include "AuditReport.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(cse::InterestPeriodInfo);
UTL_CLASS_IMPL(cse::ResourceInfo);
UTL_CLASS_IMPL(cse::FixedCostInfo);
UTL_CLASS_IMPL(cse::OverheadCostInfo);
UTL_CLASS_IMPL(cse::ResourceUsageInfo);
UTL_CLASS_IMPL(cse::InterestCostInfo);
UTL_CLASS_IMPL(cse::ResourceSequenceCostInfo);
UTL_CLASS_IMPL(cse::ComponentScoreInfo);
UTL_CLASS_IMPL(cse::OperationInfo);
UTL_CLASS_IMPL(cse::ResourceCostInfo);
UTL_CLASS_IMPL(cse::ResourceWorkHoursInfo);
UTL_CLASS_IMPL(cse::LatenessCostInfo);
UTL_CLASS_IMPL(cse::JobOverheadCostInfo);
UTL_CLASS_IMPL(cse::ResourceCostReport);
UTL_CLASS_IMPL(cse::LatenessCostReport);
UTL_CLASS_IMPL(cse::JobOverheadCostReport);
UTL_CLASS_IMPL(cse::AuditReport);

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// InterestPeriodInfo /////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void
InterestPeriodInfo::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(InterestPeriodInfo));
    const InterestPeriodInfo& ipi = (const InterestPeriodInfo&)rhs;
    idx = ipi.idx;
    begin = ipi.begin;
    end = ipi.end;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
InterestPeriodInfo::serialize(Stream& stream, uint_t io, uint_t)
{
    utl::serialize(idx, stream, io);
    lut::serialize(begin, stream, io);
    lut::serialize(end, stream, io);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// ResourceInfo ///////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void
ResourceInfo::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(ResourceInfo));
    const ResourceInfo& ri = (const ResourceInfo&)rhs;
    id = ri.id;
    name = ri.name;
    rts = ri.rts;
    mits = ri.mits;
    mets = ri.mets;
    costPerUnitHired = ri.costPerUnitHired;
    costPerHour = ri.costPerHour;
    costPerDay = ri.costPerDay;
    costPerWeek = ri.costPerWeek;
    costPerMonth = ri.costPerMonth;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ResourceInfo::serialize(Stream& stream, uint_t io, uint_t)
{
    utl::serialize(id, stream, io);
    lut::serialize(name, stream, io);
    utl::serialize(rts, stream, io);
    utl::serialize(mits, stream, io);
    utl::serialize(mets, stream, io);
    utl::serialize(costPerUnitHired, stream, io);
    utl::serialize(costPerHour, stream, io);
    utl::serialize(costPerDay, stream, io);
    utl::serialize(costPerWeek, stream, io);
    utl::serialize(costPerMonth, stream, io);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// FixedCostInfo //////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void
FixedCostInfo::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(FixedCostInfo));
    const FixedCostInfo& fci = (const FixedCostInfo&)rhs;
    opId = fci.opId;
    interestPeriod = fci.interestPeriod;
    cost = fci.cost;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
FixedCostInfo::serialize(Stream& stream, uint_t io, uint_t)
{
    utl::serialize(opId, stream, io);
    utl::serialize(interestPeriod, stream, io);
    utl::serialize(cost, stream, io);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// OverheadCostInfo ///////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void
OverheadCostInfo::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(OverheadCostInfo));
    const OverheadCostInfo& oci = (const OverheadCostInfo&)rhs;
    interestPeriod = oci.interestPeriod;
    cost = oci.cost;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
OverheadCostInfo::serialize(Stream& stream, uint_t io, uint_t)
{
    utl::serialize(interestPeriod, stream, io);
    utl::serialize(cost, stream, io);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// InterestCostInfo ///////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void
InterestCostInfo::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(InterestCostInfo));
    const InterestCostInfo& ici = (const InterestCostInfo&)rhs;
    interestPeriod = ici.interestPeriod;
    cost = ici.cost;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
InterestCostInfo::serialize(Stream& stream, uint_t io, uint_t)
{
    utl::serialize(interestPeriod, stream, io);
    utl::serialize(cost, stream, io);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// ResourceUsageInfo //////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void
ResourceUsageInfo::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(ResourceUsageInfo));
    const ResourceUsageInfo& rui = (const ResourceUsageInfo&)rhs;
    id = rui.id;
    cap = rui.cap;
    begin = rui.begin;
    end = rui.end;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ResourceUsageInfo::serialize(Stream& stream, uint_t io, uint_t)
{
    utl::serialize(id, stream, io);
    utl::serialize(cap, stream, io);
    utl::serialize(begin, stream, io);
    utl::serialize(end, stream, io);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// ComponentScoreInfo /////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void
ComponentScoreInfo::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(ComponentScoreInfo));
    const ComponentScoreInfo& csi = (const ComponentScoreInfo&)rhs;
    name = csi.name;
    score = csi.score;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ComponentScoreInfo::serialize(Stream& stream, uint_t io, uint_t)
{
    lut::serialize(name, stream, io);
    utl::serialize(score, stream, io);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// ResourceSequenceCostInfo ///////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void
ResourceSequenceCostInfo::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(ResourceSequenceCostInfo));
    const ResourceSequenceCostInfo& rsci = (const ResourceSequenceCostInfo&)rhs;
    id = rsci.id;
    lhsId = rsci.lhsId;
    rhsId = rsci.rhsId;
    cost = rsci.cost;
    ip = rsci.ip;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ResourceSequenceCostInfo::serialize(Stream& stream, uint_t io, uint_t)
{
    utl::serialize(id, stream, io);
    utl::serialize(lhsId, stream, io);
    utl::serialize(rhsId, stream, io);
    utl::serialize(cost, stream, io);
    utl::serialize(ip, stream, io);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// OperationInfo //////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void
OperationInfo::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(OperationInfo));
    const OperationInfo& oi = (const OperationInfo&)rhs;
    id = oi.id;
    name = oi.name;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
OperationInfo::serialize(Stream& stream, uint_t io, uint_t)
{
    utl::serialize(id, stream, io);
    lut::serialize(name, stream, io);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// ResourceCostInfo ///////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void
ResourceCostInfo::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(ResourceCostInfo));
    const ResourceCostInfo& rci = (const ResourceCostInfo&)rhs;
    cap = rci.cap;
    date = rci.date;
    hiredCap = rci.hiredCap;
    hireCost = rci.hireCost;
    bestRate = rci.bestRate;
    workCost = rci.workCost;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ResourceCostInfo::serialize(Stream& stream, uint_t io, uint_t)
{
    utl::serialize(cap, stream, io);
    utl::serialize(date, stream, io);
    utl::serialize(hiredCap, stream, io);
    utl::serialize(hireCost, stream, io);
    utl::serialize((uint_t&)bestRate, stream, io);
    utl::serialize(workCost, stream, io);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// ResourceWorkHoursInfo //////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void
ResourceWorkHoursInfo::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(ResourceWorkHoursInfo));
    const ResourceWorkHoursInfo& rwi = (const ResourceWorkHoursInfo&)rhs;
    cap = rwi.cap;
    date = rwi.date;
    minutes = rwi.minutes;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ResourceWorkHoursInfo::serialize(Stream& stream, uint_t io, uint_t)
{
    utl::serialize(cap, stream, io);
    lut::serialize(date, stream, io);
    utl::serialize(minutes, stream, io);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// LatenessCostInfo ///////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void
LatenessCostInfo::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(LatenessCostInfo));
    const LatenessCostInfo& lci = (const LatenessCostInfo&)rhs;
    interestPeriod = lci.interestPeriod;
    opportunityCost = lci.opportunityCost;
    latenessCost = lci.latenessCost;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
LatenessCostInfo::serialize(Stream& stream, uint_t io, uint_t)
{
    utl::serialize(interestPeriod, stream, io);
    utl::serialize(opportunityCost, stream, io);
    utl::serialize(latenessCost, stream, io);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/* January 3, 2014 (Elisa) */
/* added for job overhead cost information */
/// JobOverheadCostInfo ////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
//
void
JobOverheadCostInfo::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(JobOverheadCostInfo));
    const JobOverheadCostInfo& joci = (const JobOverheadCostInfo&)rhs;
    interestPeriod = joci.interestPeriod;
    cost = joci.cost;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
JobOverheadCostInfo::serialize(Stream& stream, uint_t io, uint_t)
{
    utl::serialize(interestPeriod, stream, io);
    utl::serialize(cost, stream, io);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// ResourceCostReport /////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void
ResourceCostReport::copy(const utl::Object& rhs)
{
    ASSERTD(rhs.isA(ResourceCostReport));
    const ResourceCostReport& rcr = (const ResourceCostReport&)rhs;
    _id = rcr._id;
    copyVector(_costs, rcr._costs);
    copyVector(_workHours, rcr._workHours);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ResourceCostReport::serialize(Stream& stream, uint_t io, uint_t)
{
    if (io == io_rd)
    {
        deleteCont(_costs);
        deleteCont(_workHours);
    }
    utl::serialize(_id, stream, io);
    lut::serialize(_costs, stream, io);
    lut::serialize(_workHours, stream, io);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ResourceCostReport::setId(uint_t id)
{
    _id = id;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ResourceCostReport::init()
{
    _id = uint_t_max;
    deleteCont(_costs);
    deleteCont(_workHours);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ResourceCostReport::deInit()
{
    deleteCont(_costs);
    deleteCont(_workHours);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// LatenessCostReport /////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void
LatenessCostReport::copy(const utl::Object& rhs)
{
    ASSERTD(rhs.isA(LatenessCostReport));
    const LatenessCostReport& lcr = (const LatenessCostReport&)rhs;
    _id = lcr._id;
    _name = lcr._name;
    _costs = lcr._costs;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
LatenessCostReport::serialize(Stream& stream, uint_t io, uint_t)
{
    if (io == io_rd)
    {
        deleteCont(_costs);
    }
    utl::serialize(_id, stream, io);
    lut::serialize(_name, stream, io);
    lut::serialize(_costs, stream, io);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
LatenessCostReport::init()
{
    deleteCont(_costs);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
LatenessCostReport::deInit()
{
    deleteCont(_costs);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// JobOverheadCostReport /////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void
JobOverheadCostReport::copy(const utl::Object& rhs)
{
    ASSERTD(rhs.isA(JobOverheadCostReport));
    const JobOverheadCostReport& lcr = (const JobOverheadCostReport&)rhs;
    _id = lcr._id;
    _name = lcr._name;
    _costs = lcr._costs;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
JobOverheadCostReport::serialize(Stream& stream, uint_t io, uint_t)
{
    if (io == io_rd)
    {
        deleteCont(_costs);
    }
    utl::serialize(_id, stream, io);
    lut::serialize(_name, stream, io);
    lut::serialize(_costs, stream, io);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
JobOverheadCostReport::init()
{
    deleteCont(_costs);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
JobOverheadCostReport::deInit()
{
    deleteCont(_costs);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// AuditReport ////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void
AuditReport::copy(const utl::Object& rhs)
{
    ASSERTD(rhs.isA(AuditReport));
    const AuditReport& ar = (const AuditReport&)rhs;

    _name = ar._name;
    _originTime = ar._originTime;
    _horizonTime = ar._horizonTime;
    _timeStep = ar._timeStep;
    _score = ar._score;

    copyVector(_interestPeriods, ar._interestPeriods);
    copyMap(_resInfos, ar._resInfos);
    copyMap(_resourceCosts, ar._resourceCosts);
    copyMap(_latenessCosts, ar._latenessCosts);
    copyMap(_joboverheadCosts, ar._joboverheadCosts);
    copyMap(_fixedCosts, ar._fixedCosts);
    copyVector(_overheadCosts, ar._overheadCosts);
    copyVector(_interestCosts, ar._interestCosts);
    copyVector(_resourceUsages, ar._resourceUsages);
    copyVector(_componentInfos, ar._componentInfos);
    copyMap(_operationInfos, ar._operationInfos);
    copyVector(_resourceSequenceCosts, ar._resourceSequenceCosts);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
AuditReport::serialize(Stream& stream, uint_t io, uint_t)
{
    lut::serialize(_name, stream, io);
    lut::serialize(_originTime, stream, io);
    lut::serialize(_horizonTime, stream, io);
    utl::serialize(_timeStep, stream, io);
    utl::serialize(_score, stream, io);
    lut::serialize(_interestPeriods, stream, io);
    lut::serialize(_resInfos, stream, io);
    lut::serialize(_resourceCosts, stream, io);
    lut::serialize(_latenessCosts, stream, io);
    lut::serialize(_joboverheadCosts, stream, io);
    lut::serialize(_fixedCosts, stream, io);
    lut::serialize(_overheadCosts, stream, io);
    lut::serialize(_interestCosts, stream, io);
    lut::serialize(_resourceUsages, stream, io);
    lut::serialize(_componentInfos, stream, io);
    lut::serialize(_operationInfos, stream, io);
    lut::serialize(_resourceSequenceCosts, stream, io);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
AuditReport::setName(std::string name)
{
    _name = name;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
AuditReport::setScore(double score)
{
    _score = score;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
AuditReport::setOriginTime(time_t t)
{
    _originTime = t;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
AuditReport::setHorizonTime(time_t t)
{
    _horizonTime = t;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
AuditReport::setTimeStep(uint_t step)
{
    _timeStep = step;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

ResourceCostReport&
AuditReport::resourceCost(uint_t id)
{
    ResourceCostReport* report;
    resourceReports_t::iterator i = _resourceCosts.find(id);
    if (i == _resourceCosts.end())
    {
        report = new ResourceCostReport();
        report->setId(id);
        _resourceCosts.insert(resourceReports_t::value_type(id, report));
    }
    else
    {
        report = i->second;
    }
    return *report;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

LatenessCostReport&
AuditReport::latenessCost(uint_t id)
{
    LatenessCostReport* report;
    latenessReports_t::iterator i = _latenessCosts.find(id);
    if (i == _latenessCosts.end())
    {
        report = new LatenessCostReport();
        report->setId(id);
        _latenessCosts.insert(latenessReports_t::value_type(id, report));
    }
    else
    {
        report = i->second;
    }
    return *report;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// January 6, 2014 (Elisa)
// added job overhead cost

JobOverheadCostReport&
AuditReport::joboverheadCost(uint_t id)
{
    JobOverheadCostReport* report;
    joboverheadReports_t::iterator i = _joboverheadCosts.find(id);
    if (i == _joboverheadCosts.end())
    {
        report = new JobOverheadCostReport();
        report->setId(id);
        _joboverheadCosts.insert(joboverheadReports_t::value_type(id, report));
    }
    else
    {
        report = i->second;
    }
    return *report;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
AuditReport::init()
{
    _name = "";
    _originTime = -1;
    _horizonTime = -1;
    _timeStep = 0;
    _interestPeriods.clear();
    _resInfos.clear();
    _resourceCosts.clear();
    _latenessCosts.clear();
    _joboverheadCosts.clear();
    _fixedCosts.clear();
    _overheadCosts.clear();
    _interestCosts.clear();
    _resourceUsages.clear();
    _componentInfos.clear();
    _operationInfos.clear();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
AuditReport::deInit()
{
    deleteCont(_interestPeriods);
    deleteMapSecond(_resInfos);
    deleteMapSecond(_resourceCosts);
    deleteMapSecond(_latenessCosts);
    deleteMapSecond(_joboverheadCosts);
    deleteMapSecond(_fixedCosts);
    deleteCont(_overheadCosts);
    deleteCont(_interestCosts);
    deleteCont(_resourceUsages);
    deleteCont(_componentInfos);
    deleteMapSecond(_operationInfos);
    deleteCont(_resourceSequenceCosts);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;
