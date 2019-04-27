#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <libutl/SpanCol.h>
#include <cls/DiscreteResource.h>
#include <cse/ResourceCost.h>
#include <cse/ScheduleEvaluator.h>
#include <cse/SchedulingContext.h>
#include <cse/AuditReport.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

class CapSpan;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Evaluate a schedule by calculating its total cost.

   The total cost of a fully constructed schedule has the following components:

   ##cls::DiscreteResource Utilization Cost

   The first step in costing cls::DiscreteResource utilization is to take **min-employment-time**
   (ResourceCost::minEmploymentTime) and **max-idle-time** (ResourceCost::maxIdleTime) into account,
   translating actually used resource capacity into effectively used capacity for costing.
   In effect, these parameters cause some resource capacity that isn't used to still be costed.
   **Min-employment-time** (if specified) means that when resource capacity is employed, it must
   continue to be costed for the specified length of time (regardless of whether it's actually used).
   **Max-idle-time** (if specified) requires a period of idleness not exceeding the specified
   length to be costed.

   After converting actual utilization into costed utilization as described above, the
   cost of employing each cls::DiscreteResource is calculated using the cheapest of the available
   rates:

   - hourly (ResourceCost::costPerHour)
   - daily (ResourceCost::costPerDay)
   - weekly (ResourceCost::costPerWeek)
   - monthly (ResourceCost::costPerMonth)

   Also, if *cost-per-unit-hired* (ResourceCost::costPerUnitHired) is specified, that cost
   is applied to each instance where the costed capacity increases.

   ##Job Opportunity, Inventory, Lateness Costs

   Opportunity cost (if specified) is a negative cost that's assessed when a Job is completed
   before its due time.  Opportunity cost is subtracted from the schedule's total cost at a
   constant rate (Job::opportunityCost) per time period (Job::opportunityCostPeriod) from the
   Job's completion time (Job::makespan) to its due time (Job::dueTime).

   Inventory cost (if specified) is a cost that's assessed when a Job is completed before its
   due time.  Inventory cost is added to the schedule's total cost at a constant rate
   (Job::inventoryCost) per time period (Job::inventoryCostPeriod) from the Job's completion
   time (Job::makespan) to its due time (Job::dueTime).
   
   Lateness cost (if specified) is a cost that's assessed when a Job is completed after its
   due time.  Lateness cost is added to the schedule's total cost at an initial rate of
   Job::latenessCost per time period (Job::latenessCostPeriod) compounding by a factor of
   of Job::latenessIncrement in each successive period.  This compounding cost reflects the
   fact that lateness has a non-linear cost, so for example it's better for two Job%s to each
   be one day late than for one Job to be two days late.

   ##Job Overhead Cost

   Job overhead cost is assessed at a constant rate (Job::overheadCost) per time period
   (Job::overheadCostPeriod) from the time work begins on the Job until its completion.

   ##Fixed Cost

   Each JobOp may have a fixed cost (JobOp::cost) that is applied when it begins to execute.
   Fixed cost is only applied to JobOp%s that are active (JobOp::active), non-summary
   (JobOp::type), and unstarted (JobOp::status).

   ##Resource Sequence Cost

   Resource sequence costs are assessed when a unary cse::DiscreteResource transitions between
   specifically identified cse::JobOp%s (see cse::DiscreteResource::sequenceList,
   cse::ResourceSequenceList).

   ##Overhead Cost

   Overhead cost is assessed at a constant rate (TotalCostEvaluatorConfiguration::overheadCost)
   per time period (TotalCostEvaluatorConfiguration::overheadCostPerPeriod) until the schedule's
   makespan (SchedulingContext::makespan).

   ##Interest Cost

   Interest cost is calculated as the final step, when the total expenditure within each
   interest compounding period is known.  Interest cost is assessed at the configured rate
   (TotalCostEvaluatorConfiguration::interestRate), compounding in each period
   (TotalCostEvaluatorconfiguration::interestRatePeriod).

   \see TotalCostEvaluatorConfiguration
   \see cse::DiscreteResource
   \see cls::DiscreteResource
   \see ResourceCost
   \ingroup cse
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class TotalCostEvaluator : public ScheduleEvaluator
{
    UTL_CLASS_DECL(TotalCostEvaluator, ScheduleEvaluator);

public:
    virtual void copy(const utl::Object& rhs);

    virtual std::string name() const;

    virtual void initialize(const gop::IndEvaluatorConfiguration* cf);

    virtual double calcScore(const gop::IndBuilderContext* context) const;

    AuditReport*
    auditReport() const
    {
        return _auditReport;
    }

private:
    using ispancol_t = utl::SpanCol<int>;
    using cslist_t = std::deque<CapSpan*>;
    using spanip_col_t = utl::TRBtree<utl::Span<int>>;

private:
    void init();
    void deInit();

    void calcResourceCost(const SchedulingContext& context) const;
    void cslistBuild(const SchedulingContext& context,
                     const cls::DiscreteResource& res,
                     cslist_t& cslist) const;
    void cslistDump(const cslist_t& cslist) const;
    void cslistCost(const SchedulingContext& context,
                    const cls::DiscreteResource& res,
                    const cslist_t& cslist) const;

    void calcLatenessCost(const SchedulingContext& context) const;
    void calcJobOverheadCost(const SchedulingContext& context) const;
    void calcFixedCost(const SchedulingContext& context) const;
    void calcResourceSequenceCost(const SchedulingContext& context) const;
    void calcOverheadCost(const SchedulingContext& context) const;
    void calcInterestCost(const SchedulingContext& context) const;

    void calcPeriodCost(const utl::Span<int>& span, double costPerTS) const;
    void calcPeriodCost(const utl::Span<int>& span,
                        double costPerTS,
                        double incrCost,
                        double periodSeconds,
                        double timeStep) const;

private:
    mutable std::ostringstream* _os;
    int _originTS;
    int _horizonTS;
    uint_t _timeStep;
    bool _forward;
    double _overheadCostPerTS;
    double _interestRate;
    uint_t _numIPs;
    spanip_col_t _ipSpans;
    mutable double* _ipCosts;
    mutable double _totalCost;
    mutable byte_t* _dayIsBreak;
    mutable size_t _dayIsBreakSize;
    mutable uint_t* _caps;
    mutable size_t _capsSize;
    mutable uint_t* _capDayTimes;
    mutable size_t _capDayTimesSize;
    mutable uint_t* _capDayHires;
    mutable size_t _capDayHiresSize;
    mutable byte_t* _capDayFireBeforeHire;
    mutable size_t _capDayFireBeforeHireSize;
    mutable double* _dayCosts;
    mutable size_t _dayCostsSize;
    mutable uint_t* _dayCostPeriods;
    mutable size_t _dayCostPeriodsSize;
    mutable AuditReport* _auditReport;
    mutable std::map<uint_t, double> _auditIpCosts;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;
