#ifndef CSE_TOTALCOSTEVALUATOR_H
#define CSE_TOTALCOSTEVALUATOR_H

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
   Evaluate the total cost of the schedule.

   The total cost of the schedule has the following components:

   <b>Interest Rate</b>

   As expenditures are made during the course of the project, interest is
   calculated on that money.  The "cost of money" is specified by the
   following parameters:

   - <b>interestRate</b> : interest rate
   - <b>interestRatePeriod</b> : interest compounding period

   <b>Opportunity Cost</b>

   Generally, a project will begin to earn income or provide a benefit
   of some kind when it is completed.  If the project is completed ahead
   of its due time, income from the project is assessed from the time
   of project completion to the due time as a constant rate per time period,
   as specified by the following parameters:

   - <b>opportunityCost</b> : opportunity cost
   - <b>opportunityCostPeriod</b> : opportunity cost period

   <b>Lateness Cost</b>

   Generally, there is a cost associated with project lateness.  If
   the project is completed later than its due time, lateness cost is
   calculated as a constant rate per time period, as specified by the
   following parameters:

   - <b>latenessCost</b> : lateness cost
   - <b>latenessCostPeriod</b> : lateness cost period
   // November 21, 2013 (Elisa)
   - <b>latenessIncrement</b> : lateness cost increment

   <b>Operation Cost</b>

   For some operations, there is a cost associated with beginning work.

   <b>Resource Utilization</b>

   The cost of resource utilization is calculated using each resource's
   associated cost information (see ResourceCost).

   <b>Overhead</b>

   The overhead cost is a constant multiple of the schedule's makespan.

   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class TotalCostEvaluator : public ScheduleEvaluator
{
    UTL_CLASS_DECL(TotalCostEvaluator);

public:
    virtual void copy(const utl::Object& rhs);

    virtual std::string
    name() const
    {
        return "TotalCost";
    }

    virtual void initialize(const gop::IndEvaluatorConfiguration* cf);

    virtual double calcScore(const gop::IndBuilderContext* context) const;

    AuditReport*
    auditReport() const
    {
        return _auditReport;
    }

private:
    typedef utl::SpanCol<int> ispancol_t;
    typedef std::list<CapSpan*> cslist_t;

private:
    void init();
    void deInit();

    void calcResourceCost(const SchedulingContext& context) const;
    void cslistBuild(const SchedulingContext& context,
                     const cls::DiscreteResource& res,
                     cslist_t& cslist) const;
    void cslistDump(const cslist_t& cslist) const;
    void cslistUnbuffer(cslist_t& cslist, cslist_t& cslist_buf, const CapSpan* endCS) const;
    void cslistCost(const SchedulingContext& context,
                    const cls::DiscreteResource& res,
                    const cslist_t& cslist) const;

    void calcLatenessCost(const SchedulingContext& context) const;
    void calcFixedCost(const SchedulingContext& context) const;
    void calcResourceSequenceCost(const SchedulingContext& context) const;
    void calcOverheadCost(const SchedulingContext& context) const;
    void calcInterestCost(const SchedulingContext& context) const;

    void calcPeriodCost(const utl::Span<int>& span, double costPerTS) const;
    /* November 21, 2013 (Elisa) */
    /* added the lateness cost increment to this function */
    void calcPeriodCost(const utl::Span<int>& span,
                        double costPerTS,
                        double incr,
                        double pStep,
                        double tStep) const;

    /* January 3, 2014 (Elisa) */
    /* calculate the overhead for each job */
    void calcJobOverheadCost(const SchedulingContext& context) const;
    void calcJobPeriodCost(const utl::Span<int>& span, double constPerTS) const;

private:
    typedef utl::TRBtree<utl::Span<int>> spanip_col_t;

private:
    mutable std::ostringstream* _os;
    int _originTS;
    int _horizonTS;
    utl::uint_t _timeStep;
    bool _forward;
    double _overheadCostPerTS;
    double _interestRate;
    utl::uint_t _numIPs;
    mutable double _totalCost;
    mutable double* _ipCost;
    mutable utl::byte_t* _dayIsBreak;
    mutable size_t _dayIsBreakSize;
    mutable utl::uint_t* _caps;
    mutable size_t _capsSize;
    mutable utl::uint_t* _capDayTimes;
    mutable size_t _capDayTimesSize;
    mutable utl::uint_t* _capDayHires;
    mutable size_t _capDayHiresSize;
    mutable utl::byte_t* _capDayFireBeforeHire;
    mutable size_t _capDayFireBeforeHireSize;
    mutable double* _dayCosts;
    mutable size_t _dayCostsSize;
    mutable utl::uint_t* _dayCostPeriods;
    mutable size_t _dayCostPeriodsSize;
    spanip_col_t _spanIPs;
    mutable AuditReport* _auditReport;
    mutable std::map<utl::uint_t, double> _auditIpCosts;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
