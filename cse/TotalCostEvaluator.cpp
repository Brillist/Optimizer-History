#include "libcse.h"
#include <cls/DiscreteResource.h>
#include "DiscreteResource.h"
#include "ResourceCost.h"
#include "SchedulingContext.h"
#include "TotalCostEvaluator.h"
#include "TotalCostEvaluatorConfiguration.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

#undef new
#include <list>
#include <libutl/gblnew_macros.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef DEBUG
// #define DEBUG_UNIT
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;
CLP_NS_USE;
CLS_NS_USE;
GOP_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

static uint_t hourSec = 60 * 60;
static uint_t daySec = 24 * hourSec;
static uint_t weekSec = 7 * daySec;
static uint_t monthSec = 30 * daySec;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// CapCostRec /////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

class CapCostRec
{
public:
    CapCostRec(uint_t cap)
    {
        init(cap);
    }

    uint_t
    cap() const
    {
        return _cap;
    }

    uint_t
    capIdx() const
    {
        return _capIdx;
    }

    uint_t&
    capIdx()
    {
        return _capIdx;
    }

    int
    startTime() const
    {
        return _startTime;
    }

    int&
    startTime()
    {
        return _startTime;
    }

    int
    endTime() const
    {
        return _endTime;
    }

    int&
    endTime()
    {
        return _endTime;
    }

private:
    void
    init(uint_t cap)
    {
        _cap = cap;
        _capIdx = 0;
        _startTime = 0;
        _endTime = 0;
    }

private:
    uint_t _cap;
    uint_t _capIdx;
    int _startTime;
    int _endTime;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
/// CapCostRecOrdering /////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

struct CapCostRecOrdering : public std::binary_function<CapCostRec*, CapCostRec*, bool>
{
    bool
    operator()(const CapCostRec* lhs, const CapCostRec* rhs) const
    {
        if (lhs->startTime() != rhs->startTime())
        {
            return (lhs->startTime() < rhs->startTime());
        }
        return (lhs->cap() < rhs->cap());
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////
/// CapCostRecCapOrdering //////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

struct CapCostRecCapOrdering : public std::binary_function<CapCostRec*, CapCostRec*, bool>
{
    bool
    operator()(const CapCostRec* lhs, const CapCostRec* rhs) const
    {
        return (lhs->cap() < rhs->cap());
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////
/// CapSpan ////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

class CapSpan
{
public:
    CapSpan(uint_t cap, const Span<int>& span)
    {
        _cap = cap;
        _begin = span.begin();
        _end = span.end();
    }

    CapSpan(uint_t cap, int begin, int end)
    {
        _cap = cap;
        _begin = begin;
        _end = end;
    }

    uint_t
    size() const
    {
        return _end - _begin;
    }

    uint_t
    cap() const
    {
        return _cap;
    }

    uint_t&
    cap()
    {
        return _cap;
    }

    int
    begin() const
    {
        return _begin;
    }

    int&
    begin()
    {
        return _begin;
    }

    int
    end() const
    {
        return _end;
    }

    int&
    end()
    {
        return _end;
    }

private:
    uint_t _cap;
    int _begin;
    int _end;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

typedef std::unordered_map<uint_t, CapCostRec*> ccr_map_t;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// SpanInterestPeriod /////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

class SpanInterestPeriod : public utl::Span<int>
{
    UTL_CLASS_DECL(SpanInterestPeriod, utl::Span<int>);

public:
    SpanInterestPeriod(int begin, int end, uint_t period);

    virtual void copy(const utl::Object& rhs);

    uint_t
    period() const
    {
        return _period;
    }

private:
    void init();
    void
    deInit()
    {
    }

private:
    uint_t _period;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

SpanInterestPeriod::SpanInterestPeriod(int begin, int end, uint_t period)
    : Span<int>(begin, end)
{
    setRelaxed(true);
    _period = period;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
SpanInterestPeriod::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(SpanInterestPeriod));
    const SpanInterestPeriod& ip = (const SpanInterestPeriod&)rhs;
    Span<int>::copy(ip);
    _period = ip._period;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
SpanInterestPeriod::init()
{
    _period = lut::period_undefined;
    setRelaxed(true);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// TotalCostEvaluator /////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void
TotalCostEvaluator::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(TotalCostEvaluator));
    const TotalCostEvaluator& tce = (const TotalCostEvaluator&)rhs;
    ScheduleEvaluator::copy(tce);
    _originTS = tce._originTS;
    _horizonTS = tce._horizonTS;
    _timeStep = tce._timeStep;
    _forward = tce._forward;
    _overheadCostPerTS = tce._overheadCostPerTS;
    _interestRate = tce._interestRate;
    _numIPs = tce._numIPs;
    delete[] _ipCost;
    _ipCost = nullptr;
    if (_numIPs > 0)
    {
        _ipCost = new double[_numIPs];
    }

    _capsSize = 0;
    delete[] _caps;
    _caps = nullptr;
    _capDayTimesSize = 0;
    delete[] _capDayTimes;
    _capDayTimes = nullptr;
    _capDayHiresSize = 0;
    delete[] _capDayHires;
    _capDayHires = nullptr;
    _capDayFireBeforeHireSize = 0;
    delete[] _capDayFireBeforeHire;
    _capDayFireBeforeHire = nullptr;
    _dayCostsSize = 0;
    delete[] _dayCosts;
    _dayCosts = nullptr;
    _dayCostPeriods = 0;
    delete[] _dayCostPeriods;
    _dayCostPeriods = nullptr;

    _spanIPs = tce._spanIPs;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
TotalCostEvaluator::initialize(const IndEvaluatorConfiguration* p_cf)
{
    ASSERTD(dynamic_cast<const TotalCostEvaluatorConfiguration*>(p_cf) != nullptr);
    const TotalCostEvaluatorConfiguration& cf = (const TotalCostEvaluatorConfiguration&)*p_cf;
    ScheduleEvaluator::initialize(cf);

    // origin, horizon, due-time
    SchedulerConfiguration* schedulerConfig = cf.getSchedulerConfig();
    _originTS = schedulerConfig->originTimeSlot();
    _horizonTS = schedulerConfig->horizonTimeSlot();
    _timeStep = schedulerConfig->timeStep();
    _forward = schedulerConfig->forward();

    // overhead cost
    uint_t periodSeconds = periodToSeconds(cf.overheadCostPeriod());
    _overheadCostPerTS = cf.overheadCost() / ((double)periodSeconds / (double)_timeStep);

    // ipCost[]
    time_t originTime = schedulerConfig->originTime();
    time_t horizonTime = schedulerConfig->horizonTime();
    period_t period = cf.interestRatePeriod();
    periodSeconds = periodToSeconds(period);

    // calculate interest rate per period
    _interestRate = cf.interestRate() / 100.0;
    _interestRate /= (periodToSeconds(period_year) / periodSeconds);

    // time slot span => interest period
    int spanBegin = 0, spanEnd;
    uint_t spanIP = uint_t_max;
    uint_t ip = uint_t_max, lastIP = uint_t_max;
    uint_t ts = _originTS;
    for (time_t t = originTime; t < horizonTime; t += _timeStep, ++ts)
    {
        struct tm* tm = localtime(&t);

        // bump interest period (ip)
        switch (period)
        {
        case period_day:
            if (tm->tm_mday != (int)lastIP)
            {
                lastIP = tm->tm_mday;
                ++ip;
            }
            break;
        case period_week:
            if ((tm->tm_wday != (int)lastIP) && (lastIP == 6))
            {
                ++ip;
            }
            lastIP = tm->tm_wday;
            break;
        case period_month:
            if (tm->tm_mon != (int)lastIP)
            {
                lastIP = tm->tm_mon;
                ++ip;
            }
            break;
        default:
            ABORT();
        }

        if ((ip == spanIP) || (spanIP == uint_t_max))
        {
            spanEnd = ts;
        }
        else
        {
            _spanIPs.add(new SpanInterestPeriod(spanBegin, spanEnd + 1, spanIP));
            spanBegin = spanEnd = ts;
        }
        spanIP = ip;
    }

    // add the last span
    if (spanIP != uint_t_max)
    {
        _spanIPs.add(new SpanInterestPeriod(spanBegin, spanEnd + 1, spanIP));
    }
    _numIPs = _spanIPs.size();
    delete[] _ipCost;
    _ipCost = new double[_numIPs];
}

////////////////////////////////////////////////////////////////////////////////////////////////////

double
TotalCostEvaluator::calcScore(const IndBuilderContext* p_context) const
{
    ASSERTD(dynamic_cast<const SchedulingContext*>(p_context) != nullptr);
    const SchedulingContext& context = (const SchedulingContext&)*p_context;

    // zero the cost vector
    _totalCost = 0.0;
    for (uint_t i = 0; i < _numIPs; ++i)
    {
        _ipCost[i] = 0.0;
    }

    // create output stream for audit
    if (_audit)
    {
        _os = new std::ostringstream();
        *_os << heading("TotalCostEvaluator", '=', 80) << std::endl;

        time_t origTime = _schedulerConfig->originTime();
        time_t horiTime = _schedulerConfig->horizonTime();
        uint_t timeStep = _schedulerConfig->timeStep();

        *_os << heading("Configuration", '=', 80) << std::endl;
        std::string originTimeStr = time_str(origTime);
        std::string horizonTimeStr = time_str(horiTime);
        *_os << "origin: " << originTimeStr << std::endl;
        *_os << "horizon: " << horizonTimeStr << std::endl;
        *_os << "time-step: " << timeStep << " sec." << std::endl;
        delete _auditReport;
        _auditReport = new AuditReport();
        _auditReport->setName(this->name());
        _auditReport->setOriginTime(origTime);
        _auditReport->setHorizonTime(horiTime);
        _auditReport->setTimeStep(timeStep);

        *_os << heading("Interest Periods", '-', 75) << std::endl;
        spanip_col_t::iterator it;
        uint_t ipIdx = 0;
        for (it = _spanIPs.begin(); it != _spanIPs.end(); ++it, ++ipIdx)
        {
            SpanInterestPeriod* sip = (SpanInterestPeriod*)*it;
            time_t begin = origTime + (sip->begin() * timeStep);
            time_t end = origTime + (sip->end() * timeStep);
            *_os << "i.p. " << ipIdx << ": "
                 << "[" << time_str(begin) << "," << time_str(end) << ")" << std::endl;
            InterestPeriodInfo* info = new InterestPeriodInfo();
            info->idx = ipIdx;
            info->begin = begin;
            info->end = end;
            _auditReport->interestPeriods()->push_back(info);
        }
    }

    // resource cost
    calcResourceCost(context);

    // opportunity/lateness cost
    calcLatenessCost(context);

    /* January 3, 2014 (Elisa) */
    /* calculate the overhead for each job */
    calcJobOverheadCost(context);

    // operation cost
    calcFixedCost(context);

    // resource sequence cost
    calcResourceSequenceCost(context);

    // overhead cost
    calcOverheadCost(context);

    // interest cost
    calcInterestCost(context);

    if (_audit)
    {
        // Store the op info.
        const jobop_set_id_t& ops = context.clevorDataSet()->ops();
        jobop_set_id_t::const_iterator it;
        for (it = ops.begin(); it != ops.end(); ++it)
        {
            JobOp* op = *it;
            OperationInfo* info = new OperationInfo();
            info->id = op->id();
            info->name = op->name();
            _auditReport->operationInfos()->insert(opInfos_t::value_type(info->id, info));
        }
    }

    if (_audit)
    {
        _audit = false;
        *_os << '\0';
        _auditText = _os->str();
        delete _os;
        _auditReport->setScore(_totalCost);
    }

    return _totalCost;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
TotalCostEvaluator::init()
{
    _os = nullptr;
    _originTS = 0;
    _horizonTS = 0;
    _timeStep = 0;
    _forward = false;
    _overheadCostPerTS = 0.0;
    _interestRate = 0.0;
    _numIPs = 0;
    _totalCost = 0.0;
    _ipCost = nullptr;
    _dayIsBreak = nullptr;
    _dayIsBreakSize = 0;
    _caps = nullptr;
    _capsSize = 0;
    _capDayTimes = nullptr;
    _capDayTimesSize = 0;
    _capDayHires = nullptr;
    _capDayHiresSize = 0;
    _capDayFireBeforeHire = nullptr;
    _capDayFireBeforeHireSize = 0;
    _dayCosts = nullptr;
    _dayCostsSize = 0;
    _dayCostPeriods = nullptr;
    _dayCostPeriodsSize = 0;
    _auditReport = nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
TotalCostEvaluator::deInit()
{
    delete[] _ipCost;
    delete[] _dayIsBreak;
    delete[] _caps;
    delete[] _capDayTimes;
    delete[] _capDayHires;
    delete[] _capDayFireBeforeHire;
    delete[] _dayCosts;
    delete[] _dayCostPeriods;
    delete _auditReport;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
TotalCostEvaluator::calcResourceCost(const SchedulingContext& context) const
{
    double saveTotalCost = _totalCost;
    const cls::Schedule* clsSchedule = context.schedule();

    cls::Schedule::res_const_iterator rit;
    for (rit = clsSchedule->resourcesBegin(); rit != clsSchedule->resourcesEnd(); ++rit)
    {
        // only deal with discrete resources that have cost defined
        cls::Resource* res = *rit;
        if ((dynamic_cast<cls::DiscreteResource*>(res) == nullptr) || (res->object() == nullptr))
        {
            continue;
        }
        const cls::DiscreteResource& dres = (const cls::DiscreteResource&)*res;

        // calculate cost
        cslist_t cslist;
        cslistBuild(context, dres, cslist);
        cslistCost(context, dres, cslist);
        deleteCont(cslist);
    }

    double totalResourceCost = _totalCost - saveTotalCost;
    setComponentScore("ResourceCost", (int)totalResourceCost);
    if (_audit)
    {
        ComponentScoreInfo* info = new ComponentScoreInfo();
        info->name = "ResourceCost";
        info->score = totalResourceCost;
        _auditReport->componentInfos()->push_back(info);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

// void
// TotalCostEvaluator::cslistBuild(
//     const SchedulingContext& context,
//     const cls::DiscreteResource& res,
//     cslist_t& cslist) const
// {
// #ifdef DEBUG_UNIT
//     std::string myName("TotalCostEvaluator::cslistBuild(): ");
//     std::cout << myName << "res.id = " << res.id() << std::endl;
// #endif
//     if (res.id() == 9)
//     {
//         BREAKPOINT;
//     }
//     Span<int> scheduleSpan(_originTS, _horizonTS);
//     Span<int> makespan(_originTS, context.makespanTimeSlot());
//     scheduleSpan.clip(makespan);

//     ResourceCost* resCost = (ResourceCost*)res.object();
//     ASSERTD(resCost != nullptr);
//     uint_t rts = utl::max((uint_t)1, resCost->resolution() / _timeStep);
//     uint_t mets = resCost->minEmploymentTime() / _timeStep;
//     uint_t mits = resCost->maxIdleTime() / _timeStep;

//     if (_audit)
//     {
//         std::string title("calcResourceCost(id = ");
//         uint_t resId = res.id();
//         double hireCost = resCost->costPerUnitHired();
//         double hourCost = resCost->costPerHour();
//         double dayCost = resCost->costPerDay();
//         double weekCost = resCost->costPerWeek();
//         double monthCost = resCost->costPerMonth();

//         char resIdStr[128]; sprintf(resIdStr, "%u", resId);
//         title.append(resIdStr);
//         title.append(")");
//         *_os << heading(title, '-', 75) << std::endl;
//         *_os
//             << "Resolution: " << rts << " time-slots" << std::endl;
//         *_os
//             << "Max-Idle: " << mits << " time-slots" << std::endl;
//         *_os
//             << "Min-Employment: " << mets << " time-slots" << std::endl;
//         *_os
//             << "Cost-per-unit-hired: " << hireCost
//             << std::endl;
//         *_os
//             << "Cost-per-hour: $" << hourCost
//             << std::endl;
//         *_os
//             << "Cost-per-day: $" << dayCost
//             << std::endl;
//         *_os
//             << "Cost-per-week: $" << weekCost
//             << std::endl;
//         *_os
//             << "Cost-per-month: $" << monthCost
//             << std::endl;

//         resInfos_t::iterator it =
//             _auditReport->resourceInfos()->find(resId);
//         if (it == _auditReport->resourceInfos()->end())
//         {
//             // Not found, create it.
//             ResourceInfo* rcInfo = new ResourceInfo();
//             rcInfo->id = resId;
//             rcInfo->name = res.name();
//             rcInfo->rts = rts;
//             rcInfo->mits = mits;
//             rcInfo->mets = mets;
//             rcInfo->costPerUnitHired = hireCost;
//             rcInfo->costPerHour = hourCost;
//             rcInfo->costPerDay = dayCost;
//             rcInfo->costPerWeek = weekCost;
//             rcInfo->costPerMonth = monthCost;
//             _auditReport->resourceInfos()->insert(
//                 resInfos_t::value_type(resId,rcInfo));
//         }
//     }

//     cslist_t cslist_buf;
//     CapSpan* lastDipCS = nullptr;
//     uint_t reqCap, lastReqCap = 0;
//     int t = scheduleSpan.getBegin();

//     const ClevorDataSet* dataSet = context.clevorDataSet();
//     const DiscreteResource* dres =
//         dynamic_cast<DiscreteResource*>(dataSet->findResource(res.id()));
//     double minCap = (double)dres->minCap() / 100.0;
//     uint_t minEmployedCap = utl::max((uint_t)1, (uint_t)ceil(minCap));//Joe: what is minEmployedCap?
//     bool employed = false;
//     int employStart = 0;
//     CapSpan* lastCapSpan = nullptr;
//     cslist_t::iterator capSpanIt;
//     uint_t existingCap = dres->existingCap();
//     existingCap = (uint_t)ceil((double)existingCap / 100.0);

//     // 1. Build cslist from resource's timetable.
//     const DiscreteTimetable& tt = res.timetable();
//     const IntSpan* ts = tt.find(scheduleSpan.getBegin());
//     const IntSpan* tail = tt.tail();

//     while (ts != tail)
//     {
//         Span<int> tsSpan = ts->span();

//         // end of timetable ?
//         if (t >= scheduleSpan.getEnd())
//         {
//             break;
//         }

//         Span<int> overlap = tsSpan.overlap(scheduleSpan);

//         if (rts > 1)
//         {
//             // set overlap
//             overlap.setBegin(t);
//             int et = overlap.getEnd();
//             et = roundDown(et, (int)rts);
//             if (et == t) et += rts;
//             et = utl::min(et, scheduleSpan.getEnd());
//             overlap.setEnd(et);

//             // determine maximum required capacity during overlap
//             reqCap = ts->v0();
//             while (tsSpan.getEnd() < overlap.getEnd())
//             {
//                 ts = ts->next();
//                 if (ts == tail)
//                 {
//                     break;
//                 }
//                 tsSpan = ts->span();
//                 reqCap = utl::max(reqCap, ts->v0());
//             }

//             // note: ts.end >= overlap.end
//             if (tsSpan.getEnd() == overlap.getEnd())
//             {
//                 ts = ts->next();
//             }
//         }
//         else
//         {
//             reqCap = ts->v0();
//             ts = ts->next();
//         }

//         // convert reqCap
//         double tmp = (double)reqCap / 100.0;
//         reqCap = (uint_t)ceil(tmp);

//         // the enforcement of min-employment is straightforward even if
//         // the determination of employed is not
//         if (employed && (reqCap == 0))
//         {
//             int minEmployEnd = res.getEndTimeForStartTime(employStart, mets) + 1;
//             minEmployEnd = res.getNonBreakTimeNext(minEmployEnd);
//             if (minEmployEnd >= overlap.getEnd())
//             {
//                 // bridge the overlap
//                 reqCap = minEmployedCap;
// #ifdef DEBUG_UNIT
//                 std::cout << "+++ MIN EMPLOY bridging: ["
//                     << overlap.getBegin() << "," << overlap.getEnd()
//                     << ")" << std::endl;
// #endif
//             }
//             else if (minEmployEnd > overlap.getBegin())
//             {
//                 // minEmployEnd is in the midst of the overlap
//                 ASSERTD(minEmployEnd > lastCapSpan->end());
//                 // messier, have to extend the usage
//                 // and truncate the overlap.
//                 if (lastCapSpan->cap() == minEmployedCap)
//                 {
// #ifdef DEBUG_UNIT
//                     std::cout << "+++ MIN EMPLOY extend before:" << std::endl;
//                     std::cout << "[" << lastCapSpan->begin()
//                         << "," << lastCapSpan->end() << ")" << std::endl;
// #endif
//                     lastCapSpan->end() = minEmployEnd;
// #ifdef DEBUG_UNIT
//                     std::cout << "+++ after:" << std::endl;
//                     std::cout << "[" << lastCapSpan->begin()
//                         << "," << lastCapSpan->end() << ")" << std::endl;
// #endif
//                 }
//                 else
//                 {
//                     CapSpan* capSpan
//                         = new CapSpan(
//                             minEmployedCap,
//                             lastCapSpan->end(),
//                             minEmployEnd);
//                     lastCapSpan = capSpan;
//                     if (mits > 1)
//                     {
//                         cslist_buf.push_back(capSpan);
//                         if (cslist_buf.size() == 1)
//                         {
//                             capSpanIt = cslist_buf.begin();
//                         }
//                         else
//                         {
//                             ++capSpanIt;
//                         }
//                         lastReqCap = minEmployedCap;
//                     }
//                     else
//                     {
//                         cslist.push_back(capSpan); // problem?
//                     }
// #ifdef DEBUG_UNIT
//                     std::cout << "+++ MIN EMPLOY new span: ["
//                         << capSpan->begin() << ","
//                         << capSpan->end() << ")" << std::endl;
// #endif
//                 }
//                 overlap.setBegin(minEmployEnd);
//             }
//         } // end of dealing with minEmployment

//         // extend the last CapSpan?
//         if ((reqCap == lastReqCap) && !cslist_buf.empty())
//         {
//             CapSpan* capSpan = *capSpanIt;
//             ASSERTD(capSpan->end() == overlap.getBegin());
//             capSpan->end() = overlap.getEnd();
//             t = overlap.getEnd();
//             continue;
//         }

//         // add a new CapSpan
//         CapSpan* capSpan = new CapSpan(reqCap, overlap);
//         lastCapSpan = capSpan;
//         if (mits < 1)
//         {
//             // if there is no max idle, then the status of employed
//             // is straightforward.  Otherwise, we'll need to wait
//             // on the result of the max-idle calculations.
//             if (!employed && (reqCap > 0))
//             {
//                 employed = true;
//                 employStart = capSpan->begin();
//             }
//             else if (employed && (reqCap == 0))
//             {
//                 employed = false;
//             }
//             cslist.push_back(capSpan);
//             t = overlap.getEnd();
//             continue;
//         }

//         // if the resource already has capacity at the start of
//         // the schedule, make a new capSpan to represent the
//         // existing capacity for determining max idle spans.
//         if (cslist.empty() && cslist_buf.empty() && (existingCap > reqCap))
//         {
//             int nonBreakTime = res.getNonBreakTimeNext(0);
//             // Joe added the following two lines. Sept 18, 2007
//             nonBreakTime = utl::min(nonBreakTime, (int)mits);
//             if (nonBreakTime <= 0) break;
//             CapSpan* existingCapSpan
//                 = new CapSpan(
//                     existingCap,
//                     0,
//                     nonBreakTime);
//             cslist_buf.push_back(existingCapSpan);
//             ASSERTD(cslist_buf.size() == 1);
//             capSpanIt = cslist_buf.begin();
//             if (capSpan->begin() < existingCapSpan->end())
//             {
//                 capSpan->begin() = existingCapSpan->end();
//             }
//             lastReqCap = existingCap;
//         }
//         cslist_buf.push_back(capSpan);
//         if (cslist_buf.size() == 1) capSpanIt = cslist_buf.begin();
//         else ++capSpanIt;

// #ifdef DEBUG_UNIT
//         std::cout << "+++ MAX IDLE before:" << std::endl;
//         cslistDump(cslist_buf);
//         std::cout << "capSpan begin: " << (*capSpanIt)->begin() << std::endl;
// #endif

//         // reduced capacity?
//         if (reqCap < lastReqCap)
//         {
//             ASSERTD(capSpanIt != cslist_buf.begin());
//             --capSpanIt;
//             lastDipCS = *capSpanIt;
//             ++capSpanIt;
//         }
//         else if (reqCap > lastReqCap)
//         {
//             if (lastDipCS == nullptr)
//             {
//                 // build CapSpans until capSpan
//                 cslistUnbuffer(cslist, cslist_buf, capSpan);
//                 // set the employed flag
//                 if (lastReqCap == 0)
//                 {
//                     employed = true;
//                     employStart = capSpan->begin();
//                 }
//             }
//             else
//             {
//                 // walk backward
//                 cslist_t::iterator it = capSpanIt;
//                 --it;
//                 ASSERTD(it != cslist_buf.begin());
// #ifdef DEBUG
//                 --it;
//                 ASSERT(*it == lastDipCS);
//                 ++it;
// #endif

//                 // find extendCS
//                 CapSpan* extendCS = nullptr;
//                 cslist_t::iterator extendCSit;
//                 while (it != cslist_buf.begin())
//                 {
//                     --it;
//                     CapSpan* cs = *it;
//                     int et = res.getEndTimeForStartTime(cs->end(), mits) + 1;
//                     et = res.getNonBreakTimeNext(et);
//                     if (et >= capSpan->begin())
//                     {
//                         extendCS = cs;
//                         extendCSit = it;
//                         if (extendCS->cap() >= reqCap)
//                         {
//                             break;
//                         }
//                     }
//                     else
//                     {
//                         if (extendCS == nullptr)
//                         {
//                             cslistUnbuffer(cslist, cslist_buf, capSpan);
//                         }
//                         else
//                         {
//                             cslistUnbuffer(cslist, cslist_buf, extendCS);
//                         }
//                         break;
//                     }
//                 }

//                 // extend a CapSpan?
//                 if (extendCS == nullptr)
//                 {
//                     lastDipCS = nullptr;
//                     // there were no spans matching the max idle criteria
//                     if (lastReqCap == 0)
//                     {
//                         employed = true;
//                         employStart = capSpan->begin();
//                     }
//                 }
//                 else
//                 {
// #ifdef DEBUG_UNIT
//                     std::cout
//                         << "+++ extendCS: " << "[" << extendCS->begin()
//                         << "," << extendCS->end() << "): "
//                         << extendCS->cap() << std::endl;
// #endif
//                     ASSERTD(lastDipCS != nullptr);
//                     if (extendCSit == cslist_buf.begin())
//                     {
//                         lastDipCS = nullptr;
//                     }
//                     else
//                     {
//                         it = extendCSit;
//                         --it;
//                         lastDipCS = *it;
//                     }

//                     if (extendCS->cap() < reqCap)
//                     {
//                         extendCS->end() = capSpan->begin();
//                         it = extendCSit; ++it;
//                     }
//                     else if (extendCS->cap() > reqCap)
//                     {
//                         capSpan->begin() = extendCS->end();
//                         it = extendCSit; ++it;
//                         lastDipCS = extendCS;
//                     }
//                     else // equal
//                     {
//                         capSpan->begin() = extendCS->begin();
//                         it = extendCSit;
//                     }
//                     while (it != capSpanIt)
//                     {
//                         CapSpan* cs = *it;
//                         delete cs;
//                         it = cslist_buf.erase(it);
//                     }
//                 } // extendCS != nullptr
//                 if (lastDipCS == nullptr)
//                 {
//                     cslistUnbuffer(cslist, cslist_buf, capSpan);
//                 }
//             } // lastDipCS != nullptr
//         } // reqCap > lastReqCap

// #ifdef DEBUG_UNIT
//         std::cout << "+++ after:" << std::endl;
//         cslistDump(cslist_buf);
// #endif

//         lastReqCap = reqCap;
//         t = overlap.getEnd();
//     } // while (ts != tail)

//     // need to check if the employment of the resource
//     // has to be extended beyond the resource timetable
//     // due to min employment.
//     if (employed && (mets > 0))
//     {
//         int end = res.getEndTimeForStartTime(employStart, mets) + 1;
//         end = utl::min(_horizonTS, end);
//         if (lastCapSpan->end() < end)
//         {
//             // if the last CapSpan has capacity equal to
//             // the min employment, just extend it.
//             if (lastCapSpan->cap() == minEmployedCap)
//             {
//                 lastCapSpan->end() = end;
//             }
//             else
//             {
//                 // need to make a new CapSpan
//                 CapSpan* capSpan =
//                     new CapSpan(minEmployedCap, lastCapSpan->end(), end);
//                 lastCapSpan = capSpan;
//                 cslist_buf.push_back(capSpan);
//                 if (cslist_buf.size() == 1) capSpanIt = cslist_buf.begin();
//                 else ++capSpanIt;
//             }
// #ifdef DEBUG_UNIT
//             std::cout << "+++ MIN EMPLOY tail extension to " << end << std::endl;
// #endif
//         }
//         employed = false;
//     }

//     // clean up left-overs
//     cslistUnbuffer(cslist, cslist_buf, nullptr);

// #ifdef DEBUG_UNIT
//     std::cout << "----- finished: ----" << std::endl;
//     cslistDump(cslist);
// #endif
// }

////////////////////////////////////////////////////////////////////////////////////////////////////

void
TotalCostEvaluator::cslistDump(const cslist_t& cslist) const
{
    time_t origin = _schedulerConfig->originTime();
    uint_t timeStep = _schedulerConfig->timeStep();
    cslist_t::const_iterator it;
    for (it = cslist.begin(); it != cslist.end(); ++it)
    {
        CapSpan* cs = *it;
        time_t begin = cs->begin() * timeStep + origin;
        time_t end = cs->end() * timeStep + origin;
        std::cout << "[" << time_str(begin) << "<" << cs->begin() << ">"
                  << "," << time_str(end) << "<" << cs->end() << ">"
                  << "): " << cs->cap() << std::endl;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
TotalCostEvaluator::cslistUnbuffer(cslist_t& cslist,
                                   cslist_t& cslist_buf,
                                   const CapSpan* endCS) const
{
    while (!cslist_buf.empty())
    {
        CapSpan* cs = cslist_buf.front();
        if (cs == endCS)
        {
            break;
        }
        cslist_buf.pop_front();
        cslist.push_back(cs);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

// cslistCost calculate resource cost based on the CapSpan list generated
// by cslistBuild
void
TotalCostEvaluator::cslistCost(const SchedulingContext& context,
                               const cls::DiscreteResource& res,
                               const cslist_t& cslist) const
{
#ifdef DEBUG_UNIT
    std::string myName("TotalCostEvaluator::cslistCost(): ");
    std::cout << myName << "res.id = " << res.id() << std::endl;
#endif
    Span<int> scheduleSpan(_originTS, _horizonTS);
    Span<int> makespan(_originTS, context.makespanTimeSlot());
    scheduleSpan.clip(makespan);
    ResourceCost* resCost = (ResourceCost*)res.object();
    ASSERTD(resCost != nullptr);

    // cannot cost the resource without cost information
    if ((resCost->costPerHour() == double_t_max) && (resCost->costPerDay() == double_t_max) &&
        (resCost->costPerWeek() == double_t_max) && (resCost->costPerMonth() == double_t_max))
    {
        return;
    }

    // how many days in schedule?
    uint_t timeStep = _schedulerConfig->timeStep();
    int tsPerDay = daySec / timeStep;
    ASSERTD((tsPerDay * timeStep) == daySec);
    makespan.setEnd(roundUp(makespan.end(), tsPerDay));

    // might need to extend the makespan because of
    // min employment
    if (!cslist.empty())
    {
        CapSpan* lastSpan = cslist.back();
        if (lastSpan->end() > makespan.end())
        {
#ifdef DEBUG_UNIT
            std::cout << myName << "lastSpan->end() > makespan.end() | " << lastSpan->end() << " > "
                      << makespan.getEnd() << std::endl;
#endif
            makespan.setEnd(roundUp(lastSpan->end(), tsPerDay));
        }
    }

    uint_t numDays = makespan.size() / tsPerDay;

    // make a list of all required capacities
    uint_t numCaps = 0;
    ccr_map_t ccrs;
    // - make sure we always have capacity 0
    utl::arrayGrow(_caps, _capsSize, 16);
    _caps[numCaps++] = 0;
    CapCostRec* ccr = new CapCostRec(0);
    ccrs.insert(ccr_map_t::value_type(0, ccr));

    uint_t maxCap = 0;
    cslist_t::const_iterator cslistIt;
    for (cslistIt = cslist.begin(); cslistIt != cslist.end(); ++cslistIt)
    {
        CapSpan* capSpan = *cslistIt;
        uint_t cap = capSpan->cap();
        if (cap > maxCap)
            maxCap = 0;
        ccr_map_t::iterator it = ccrs.find(cap);
        if (it == ccrs.end())
        {
            utl::arrayGrow(_caps, _capsSize, numCaps + 1);
            _caps[numCaps++] = cap;
            ccr = new CapCostRec(cap);
            ccr->startTime() = roundDown(capSpan->begin(), tsPerDay);
            ccrs.insert(ccr_map_t::value_type(cap, ccr));
        }
        else
        {
            ccr = (*it).second;
        }
        ccr->endTime() = roundUp(capSpan->end(), tsPerDay);
        if (_audit)
        {
            uint_t timeStep = _schedulerConfig->timeStep();
            time_t origin = _schedulerConfig->originTime();
            ResourceUsageInfo* usage = new ResourceUsageInfo();
            usage->id = res.id();
            usage->cap = cap;
            usage->begin = capSpan->begin() * timeStep + origin;
            usage->end = capSpan->end() * timeStep + origin;
            _auditReport->resourceUsages()->push_back(usage);
        }
    }

    // we also need to make sure we always have a capacity = existingCap,
    // otherwise the following method of calculating hiring cost cannot
    // work properly.
    // Joe, June 15, 2009
    const ClevorDataSet* dataSet = context.clevorDataSet();
    const DiscreteResource* dres = dynamic_cast<DiscreteResource*>(dataSet->findResource(res.id()));
    uint_t existingCap = dres->existingCap();
    existingCap = (uint_t)ceil((double)existingCap / 100.0);
    if (ccrs.find(existingCap) == ccrs.end() && existingCap < maxCap)
    {
        utl::arrayGrow(_caps, _capsSize, numCaps + 1);
        _caps[numCaps++] = existingCap;
        ccr = new CapCostRec(existingCap);
        ccrs.insert(ccr_map_t::value_type(existingCap, ccr));
    }

    // index the required capacities
    std::sort(_caps, _caps + numCaps);
    uint_t capIdx = 0;
    ccr_map_t::iterator it;
    for (uint_t i = 0; i < numCaps; ++i)
    {
        uint_t cap = _caps[i];
        it = ccrs.find(cap);
        ASSERTD(it != ccrs.end());
        CapCostRec* ccr = (*it).second;
        ccr->capIdx() = capIdx++;
    }
    // capacity (n - 1) is always in use when capacity (n) is in use
    int minStart = int_t_max, maxEnd = int_t_min;
    for (uint_t i = numCaps - 1; i > 0; --i)
    {
        uint_t cap = _caps[i];
        it = ccrs.find(cap);
        ASSERTD(it != ccrs.end());
        CapCostRec* ccr = (*it).second;
        minStart = utl::min(minStart, ccr->startTime());
        maxEnd = utl::max(maxEnd, ccr->endTime());
        ccr->startTime() = minStart;
        ccr->endTime() = maxEnd;
    }

    // grow the dayIsBreak[], capDayTimes[], capDayHires[] arrays
    uint_t cap_x_days = numCaps * numDays;
    utl::arrayGrow(_dayIsBreak, _dayIsBreakSize, numDays);
    utl::arrayGrow(_capDayTimes, _capDayTimesSize, cap_x_days);
    utl::arrayGrow(_capDayHires, _capDayHiresSize, cap_x_days);
    utl::arrayGrow(_capDayFireBeforeHire, _capDayFireBeforeHireSize, cap_x_days);
    memset(_dayIsBreak, 1, numDays * sizeof(byte_t));
    memset(_capDayTimes, 0, cap_x_days * sizeof(uint_t));
    memset(_capDayHires, 0, cap_x_days * sizeof(uint_t));
    memset(_capDayFireBeforeHire, 0, cap_x_days * sizeof(byte_t));

    // the following change on lastCap and lastCapIdx is for fixing a hiringCost
    // calculation problem.
    // Joe, June 15, 2009
    uint_t lastCap = existingCap;
    uint_t capTmp = lastCap;
    while (ccrs.find(capTmp) == ccrs.end())
    {
        capTmp--;
    }
    uint_t lastCapIdx = ccrs.find(capTmp)->second->capIdx();
    ASSERTD(_caps[0] == 0);

    // count working time for each day & capacity
    for (cslistIt = cslist.begin(); cslistIt != cslist.end(); ++cslistIt)
    {
        CapSpan* capSpan = *cslistIt;
        uint_t cap = capSpan->cap();
        int begin = roundDown(capSpan->begin(), tsPerDay);
        int end = roundUp(capSpan->end(), tsPerDay);
        // ignore if the entire span is a break.
        uint_t nonBreak = res.getNonBreakTime(capSpan->begin(), capSpan->end() - 1);
        if (nonBreak == 0)
        {
            continue;
        }
        uint_t day = (begin / tsPerDay);

        it = ccrs.find(cap);
        ASSERTD(it != ccrs.end());
        CapCostRec* ccr = (*it).second;
        uint_t capIdx;
        capIdx = ccr->capIdx();

        // hiring?
        if (cap > lastCap)
        {
            for (uint_t ci = lastCapIdx + 1; ci <= capIdx; ++ci)
            {
                int offset = (ci * numDays) + day;
                ++_capDayHires[offset];
            }
        }
        else if (cap < lastCap)
        {
            for (uint_t ci = lastCapIdx; ci > capIdx; --ci)
            {
                int offset = (ci * numDays) + day;
                if (_capDayHires[offset] == 0)
                {
                    _capDayFireBeforeHire[offset] = 1;
                }
            }
        }
        lastCap = cap;
        lastCapIdx = capIdx;

        for (; begin < end; begin += tsPerDay, ++day)
        {
            int eod = begin + tsPerDay;
            int b = utl::max(capSpan->begin(), begin);
            int e = utl::min(capSpan->end(), eod);
            uint_t workingTime = res.getNonBreakTime(b, e - 1);

            if (workingTime == 0)
            {
                continue;
            }

            // day has non-break time...
            _dayIsBreak[day] = 0;

            for (uint_t ci = 1; ci <= capIdx; ++ci)
            {
                uint_t& wt = _capDayTimes[(ci * numDays) + day];
                wt += workingTime;
            }
        }
    }

    // reduce hires by one on each day
    //uint_t* ptr = _capDayHires;
    //uint_t* lim = _capDayHires + cap_x_days;
    //while (ptr < lim)
    //{
    //   if (*ptr > 0) --(*ptr);
    //   ++ptr;
    //}

    // grow dayCosts[], dayCostPeriods[] arrays
    utl::arrayGrow(_dayCosts, _dayCostsSize, numDays);
    utl::arrayGrow(_dayCostPeriods, _dayCostPeriodsSize, numDays);

    // cost each unit of capacity
    double saveTotalCost = _totalCost;
    lastCap = 0;
    for (uint_t capIdx = 1; capIdx < numCaps; ++capIdx)
    {
        // find cap, capDiff
        uint_t cap = _caps[capIdx];
        uint_t capDiff = (cap - lastCap);
        lastCap = cap;

        if (_audit)
        {
            char buf[128];
            sprintf(buf, "%u", cap);
            std::string capStr(buf);
            std::string title = "Capacity " + capStr;
            *_os << heading(title, '.', 65) << std::endl;
        }

        // prepare for costing loop
        it = ccrs.find(cap);
        ASSERTD(it != ccrs.end());
        CapCostRec* ccr = (*it).second;
        uint_t ccrCapIdx = ccr->capIdx();
        uint_t capDayBegin = (ccr->startTime() / tsPerDay);
        uint_t numCapDays = (ccr->endTime() - ccr->startTime()) / tsPerDay;
        ASSERTD(numCapDays > 0);
        uint_t capDayEnd = capDayBegin + numCapDays;
        uint_t* ht = _capDayHires + (ccrCapIdx * numDays);
        uint_t* wt = _capDayTimes + (ccrCapIdx * numDays);
        byte_t* fbht = _capDayFireBeforeHire + (ccrCapIdx * numDays);

        // show hours worked on each day
        if (_audit)
        {
            ResourceCostReport* report = _auditReport->resourceCost(res.id());
            uint_t curTs = capDayBegin * tsPerDay;
            *_os << "Hours worked on each day: ";
            for (uint_t day = capDayBegin; day < capDayEnd; ++day)
            {
                double hoursWorked = wt[day] * timeStep / 3600.0;
                *_os << day << ":" << hoursWorked << " ";
                time_t originTime = _schedulerConfig->originTime();
                time_t curTime = originTime + (curTs * timeStep);
                ResourceWorkHoursInfo* newInfo = new ResourceWorkHoursInfo();
                newInfo->cap = cap;
                newInfo->date = curTime;
                newInfo->minutes = wt[day] * capDiff * timeStep / 60;
                report->workHours()->push_back(newInfo);
                curTs += tsPerDay;
            }
            *_os << std::endl;
        }

        // note costs
        double hiringCost = resCost->costPerUnitHired();
        double hourCost = resCost->costPerHour();
        double dayCost = resCost->costPerDay();
        double weekCost = resCost->costPerWeek();
        double monthCost = resCost->costPerMonth();
        if (hiringCost != double_t_max)
            hiringCost *= (double)capDiff;
        if (hourCost != double_t_max)
            hourCost *= (double)capDiff;
        if (dayCost != double_t_max)
            dayCost *= (double)capDiff;
        if (weekCost != double_t_max)
            weekCost *= (double)capDiff;
        if (monthCost != double_t_max)
            monthCost *= (double)capDiff;

        // consider hourly and daily rate for each day
        uint_t day;
        for (day = capDayBegin; day < capDayEnd; ++day)
        {
            _dayCosts[day] = 0.0;
            _dayCostPeriods[day] = period_undefined;

            uint_t workingTime = wt[day];

            // ignore day that is a break or has no working time
            if (_dayIsBreak[day] || (workingTime == 0))
            {
                continue;
            }

            // hourly cost?
            if (hourCost != double_t_max)
            {
                // HOWARD'S CHANGE
                // Round workingHours to the hour.
                uint_t workingHours = roundUp((workingTime * timeStep), (uint_t)3600) / 3600;
                double cost = workingHours * hourCost;
                // For hourly costs, all hiring has to be
                // accounted for.
                cost += ht[day] * hiringCost;
                _dayCosts[day] = cost;
                _dayCostPeriods[day] = period_hour;
            }

            // daily cost?
            if (dayCost != double_t_max)
            {
                double cost = dayCost;
                // Only account for hiring cost if it is
                // done before any firing had occured.
                if ((!fbht[day]) && (ht[day] > 0))
                {
                    cost += hiringCost;
                }
                if ((cost < _dayCosts[day]) || (_dayCostPeriods[day] == period_undefined))
                {
                    _dayCosts[day] = cost;
                    _dayCostPeriods[day] = period_day;
                }
            }
        }

        // HOWARD'S CHANGE
        // copy _dayCosts, we're going to need it later on.
        double* dayCosts = new double[numDays];
        for (uint_t i = 0; i < numDays; i++)
        {
            dayCosts[i] = _dayCosts[i];
        }

        // HOWARD'S CHANGES
        // consider use of weekly rate
        if (weekCost != double_t_max)
        {
            // We want to build all the possible weekly rates day by day.
            uint_t beginDay = capDayBegin;
            uint_t beginPlus7 = beginDay + 7;
            uint_t endDay = beginDay;

            while (beginDay < capDayEnd)
            {
                if ((!_dayIsBreak[beginDay]) && (wt[beginDay] != 0))
                {
                    // Determine the cost of the next 7 or less days.
                    double next7DaysCost = 0.0;
                    uint_t lim = utl::min(beginPlus7, capDayEnd);
                    for (endDay = beginDay; endDay < lim; ++endDay)
                    {
                        next7DaysCost += _dayCosts[endDay];
                    }
                    // Determine the cost for using weekly cost.
                    double cost = weekCost;
                    if ((!fbht[beginDay]) && (ht[beginDay] > 0))
                    {
                        cost += hiringCost;
                    }
                    // Compare the costs.
                    if (((cost < next7DaysCost) || (_dayCostPeriods[beginDay] == period_undefined)))
                    {
                        _dayCosts[beginDay] = cost;
                        _dayCostPeriods[beginDay] = period_week;
                    }
                }
                // Tomorrow is another day.
                beginDay++;
                beginPlus7 = beginDay + 7;
            }
        }

        // consider use of monthly rate
        // HOWARD'S CHANGE
        // No need to consider it for every day until there
        // is a yearly rate.
        if (monthCost != double_t_max)
        {
            uint_t beginDay = capDayBegin;
            // Find next beginDay.
            time_t originTime = _schedulerConfig->originTime();
            time_t t;
            t = originTime + (beginDay * tsPerDay * timeStep);
            struct tm* tm = localtime(&t);
            ++tm->tm_mon;
            t = mktime(tm);
            uint_t beginPlus30 = (t - originTime) / (tsPerDay * timeStep);
            uint_t endDay;

            while (beginDay < capDayEnd)
            {
                if ((_dayIsBreak[beginDay]) || (wt[beginDay] == 0))
                {
                    ++beginDay;
                    t = originTime + (beginDay * tsPerDay * timeStep);
                    tm = localtime(&t);
                    ++tm->tm_mon;
                    t = mktime(tm);
                    beginPlus30 = (t - originTime) / (tsPerDay * timeStep);
                    continue;
                }
                // Determine the cost of the next 30 or so days.
                double next30DaysCost = 0.0;
                double totalMonthCost = monthCost;
                uint_t lim = utl::min(beginPlus30, capDayEnd);
                endDay = beginDay;
                while (endDay < lim)
                {
                    next30DaysCost += _dayCosts[endDay];
                    if (_dayCostPeriods[endDay] == period_week)
                    {
                        // handle the case where the end point for
                        // a weekly rate exceeds the month boundary.
                        uint_t dayLim = endDay + 7;
                        for (uint_t end = beginPlus30; end < dayLim; ++end)
                        {
                            totalMonthCost += dayCosts[end];
                        }
                        endDay = dayLim;
                    }
                    else
                    {
                        ++endDay;
                    }
                }
                double cost = monthCost;
                // Account for any hiring costs.
                if ((!fbht[beginDay]) && (ht[beginDay] > 0))
                {
                    totalMonthCost += hiringCost;
                    cost += hiringCost;
                }
                // Compare the costs.
                if ((totalMonthCost < next30DaysCost) ||
                    (_dayCostPeriods[beginDay] == period_undefined))
                {
                    _dayCosts[beginDay] = cost;
                    _dayCostPeriods[beginDay] = period_month;
                    beginDay = beginPlus30;
                }
                else
                {
                    ++beginDay;
                }
                t = originTime + (beginDay * tsPerDay * timeStep);
                tm = localtime(&t);
                ++tm->tm_mon;
                t = mktime(tm);
                beginPlus30 = (t - originTime) / (tsPerDay * timeStep);
            }
        }

        delete[] dayCosts;

        // execute the plan for costing the resource
        uint_t t = capDayBegin * tsPerDay;
        day = capDayBegin;
        while (day < capDayEnd)
        {
            // did not cost the day?
            // (no working time or the day is a break)
            if (_dayCostPeriods[day] == period_undefined)
            {
                ++day;
                t += tsPerDay;
                continue;
            }

            // what interest period (ip) does day belong to?
            spanip_col_t::iterator it = _spanIPs.findFirstIt(Span<int>(t, t + 1));
            ASSERTD(it != _spanIPs.end());
            SpanInterestPeriod* sip = (SpanInterestPeriod*)*it;
            uint_t ip = sip->period();

            if (_audit)
            {
                time_t originTime = _schedulerConfig->originTime();
                time_t curTime = originTime + (t * timeStep);
                *_os << "Day: " << day << ": " << time_str(curTime) << std::endl;

                ResourceCostReport* report = _auditReport->resourceCost(res.id());
                ResourceCostInfo* newCost = new ResourceCostInfo();
                newCost->cap = cap;
                newCost->date = curTime;
                newCost->hiredCap = 0;
                newCost->hireCost = 0;
                newCost->bestRate = period_t(period_undefined);
                report->costs()->push_back(newCost);
            }

            // get cost & period
            double cost = _dayCosts[day];
            period_t period = (period_t)_dayCostPeriods[day];
            // Determine the cost components.
            double rateCost = 0;
            uint_t capHired = 0;
            double hireCost = 0;

            switch (period)
            {
            case period_hour:
            {
                if (ht[day] > 0)
                {
                    capHired = ht[day] * capDiff;
                    hireCost = ht[day] * hiringCost;
                }
                rateCost = cost - hireCost;
            }
            break;
            case period_day:
            case period_week:
            case period_month:
            {
                if ((!fbht[day]) && (ht[day] > 0))
                {
                    capHired = capDiff;
                    hireCost = hiringCost;
                }
                rateCost = cost - hireCost;
            }
            break;
            default:
                ABORT();
            }

            // need to (re-)hire?
            if (capHired > 0)
            {
                if (_audit)
                {
                    *_os << "Hiring " << capHired << " unit(s): "
                         << "$" << hireCost << std::endl;
                    *_os << "    i.p. " << ip << ": $" << _ipCost[ip];

                    ResourceCostInfo* costInfo =
                        _auditReport->resourceCost(res.id()).costs()->back();
                    costInfo->hiredCap = capHired;
                    costInfo->hireCost = hireCost;
                }
                _ipCost[ip] += hireCost;
                _totalCost += hireCost;
                if (_audit)
                {
                    *_os << " + $" << hireCost << " = $" << _ipCost[ip] << std::endl;
                }
            }

            // print the chosen rate
            if (_audit)
            {
                *_os << "bestRate: " << periodToString((period_t)_dayCostPeriods[day])
                     << ", cost: $" << rateCost;
                if ((period == period_hour) || (period == period_day))
                {
                    double hoursWorked = wt[day] * timeStep / 3600.0;
                    *_os << ", hoursWorked: " << hoursWorked;
                }
                *_os << std::endl;
                ResourceCostInfo* costInfo = _auditReport->resourceCost(res.id()).costs()->back();
                costInfo->bestRate = (period_t)_dayCostPeriods[day];
                costInfo->workCost = rateCost;
            }

            // how many days to advance?
            uint_t periodDays;
            uint_t nextT;
            switch (period)
            {
            case period_hour:
            case period_day:
                periodDays = 1;
                break;
            case period_week:
                periodDays = 7;
                break;
            case period_month:
            {
                // HOWARD'S CHANGE.
                // Slightly more complicated.
                time_t tTime_T = _schedulerConfig->originTime() + (t * timeStep);
                struct tm* tm = localtime(&tTime_T);
                ++tm->tm_mon;
                time_t nextMonth = mktime(tm);
                // Shouldn't have to do any rounding up since
                // everything is on day boundary atm.
                periodDays = (uint_t)difftime(nextMonth, tTime_T) / daySec;
            }
            break;
            default:
                ABORT();
            }
            nextT = t + (periodDays * tsPerDay);

            if (periodDays == 1)
            {
                if (_audit)
                {
                    *_os << "    i.p. " << ip << ": $" << _ipCost[ip];
                }
                _ipCost[ip] += rateCost;
                _totalCost += rateCost;
                if (_audit)
                {
                    *_os << " + $" << rateCost << " = $" << _ipCost[ip] << std::endl;
                }
            }
            else
            {
                double costPerTS = (double)rateCost / (double)(nextT - t);
                calcPeriodCost(Span<int>(t, nextT), costPerTS);
            }

            day += periodDays;
            t = nextT;
        }
    }
    double totalResCost = _totalCost - saveTotalCost;
    resCost->cost() = (uint_t)totalResCost;
    /*std::cout
        << "resourceCost(" << res.id() << ") = "
        << totalResCost
        << std::endl;*/

    deleteMapSecond(ccrs);
#ifdef DEBUG_UNIT
    std::cout << myName << "----- finished -----" << std::endl;
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
TotalCostEvaluator::calcLatenessCost(const SchedulingContext& context) const
{
    if (_audit)
    {
        *_os << heading("calcOpportunity/Inventory/LatenessCost()", '-', 75) << std::endl;
    }

    double oppCost = 0.0;
    double invCost = 0.0;
    double latCost = 0.0;
    double saveTotalCost = _totalCost;

    const job_set_id_t& jobs = context.clevorDataSet()->jobs();
    job_set_id_t::const_iterator it;
    for (it = jobs.begin(); it != jobs.end(); ++it)
    {
        const Job* job = *it;
        if (job->dueTime() == -1 || !job->active())
        {
            continue;
        }
        int dueTime = context.timeToTimeSlot(job->dueTime());
        int makespan = job->makespan();

#ifdef DEBUG
        // check the makespan
        int maxEndTime = -1;
        for (Job::const_iterator it = job->begin(); it != job->end(); ++it)
        {
            JobOp* op = *it;
            if (op->type() == op_summary)
                continue;
            if (op->ignorable())
                continue;
            Activity* act = op->activity();
            if (act == nullptr)
                continue;

            int endTime = _schedulerConfig->forward() ? act->ef() + 1 : act->lf() + 1;
            if (endTime > maxEndTime)
            {
                maxEndTime = endTime;
            }
        }
        // the maxEndTime != -1 check is for summaries with no
        // schedulables (most likely containing only milestones).
        if ((maxEndTime != -1) && (makespan != maxEndTime))
        {
            std::cout << "end times differ for job " << job->id() << " makespan " << makespan
                      << " jobOp end time " << maxEndTime << std::endl;
        }
#endif
        if (dueTime == makespan)
        {
            continue;
        }
        // opportunity cost
        double opportunityCostPerTS = 0.0;
        if (job->opportunityCostPeriod() != period_undefined)
        {
            uint_t periodSeconds;
            periodSeconds = periodToSeconds(job->opportunityCostPeriod());
            opportunityCostPerTS =
                job->opportunityCost() / ((double)periodSeconds / (double)_timeStep);
        }

        double latenessCostPerTS = 0.0;
        double latenessIncrement = 0.0;
        double pSeconds = 0.0;
        if (job->latenessCostPeriod() != period_undefined)
        {
            uint_t periodSeconds = periodToSeconds(job->latenessCostPeriod());
            pSeconds = (double)periodSeconds;
            latenessCostPerTS = job->latenessCost() / ((double)periodSeconds / (double)_timeStep);
            latenessIncrement = job->latenessIncrement() / 100;
            latenessIncrement = latenessIncrement + (double)1.0;
        }
        double inventoryCostPerTS = 0.0;
        if (job->inventoryCostPeriod() != period_undefined)
        {
            uint_t periodSeconds = periodToSeconds(job->inventoryCostPeriod());
            inventoryCostPerTS = job->inventoryCost() / ((double)periodSeconds / (double)_timeStep);
        }

        if (_audit)
        {
            *_os << "WorkOrder id = " << job->id() << " name = " << job->name() << ": ";
        }
        if (makespan < dueTime)
        {
            // opportunity cost
            if (_audit)
            {
                *_os << "Opportunity Cost" << std::endl;
            }
            calcPeriodCost(Span<int>(makespan, dueTime), -1.0 * opportunityCostPerTS);
            if (_audit)
            {
                LatenessCostReport* report = _auditReport->latenessCost(job->id());
                if (report->getName().empty())
                {
                    report->setName(job->name());
                }
                std::map<uint_t, double>::const_iterator i;
                for (i = _auditIpCosts.begin(); i != _auditIpCosts.end(); i++)
                {
                    LatenessCostInfo* info = new LatenessCostInfo();
                    info->interestPeriod = i->first;
                    info->opportunityCost = i->second;
                    info->latenessCost = 0;
                    info->inventoryCost = 0;
                    report->costs()->push_back(info);
                }
            }
            oppCost += _totalCost - saveTotalCost;
            saveTotalCost = _totalCost;
            // inventory cost
            if (_audit)
            {
                *_os << "Inventory Cost" << std::endl;
            }
            calcPeriodCost(Span<int>(makespan, dueTime), inventoryCostPerTS);
            if (_audit)
            {
                LatenessCostReport* report = _auditReport->latenessCost(job->id());
                if (report->getName().empty())
                {
                    report->setName(job->name());
                }
                std::map<uint_t, double>::const_iterator i;
                for (i = _auditIpCosts.begin(); i != _auditIpCosts.end(); i++)
                {
                    LatenessCostInfo* info = new LatenessCostInfo();
                    info->interestPeriod = i->first;
                    info->opportunityCost = 0;
                    info->latenessCost = 0;
                    info->inventoryCost = i->second;
                    report->costs()->push_back(info);
                }
            }
            invCost += _totalCost - saveTotalCost;
            saveTotalCost = _totalCost;
        }
        /* November 21, 2013 (Elisa) */
        /* code will be added in this section to use the */
        /* lateness cost increment in calculations */
        else if (makespan > dueTime) // lateness cost
        {
            if (_audit)
            {
                *_os << "Lateness Cost" << std::endl;
            }
            calcPeriodCost(Span<int>(dueTime, makespan), latenessCostPerTS, latenessIncrement,
                           pSeconds, (double)_timeStep);
            if (_audit)
            {
                LatenessCostReport* report = _auditReport->latenessCost(job->id());
                if (report->getName().empty())
                {
                    report->setName(job->name());
                }
                std::map<uint_t, double>::const_iterator i;
                for (i = _auditIpCosts.begin(); i != _auditIpCosts.end(); i++)
                {
                    LatenessCostInfo* info = new LatenessCostInfo();
                    info->interestPeriod = i->first;
                    info->opportunityCost = 0;
                    info->latenessCost = i->second;
                    info->inventoryCost = 0;
                    report->costs()->push_back(info);
                }
            }
            latCost += _totalCost - saveTotalCost;
            saveTotalCost = _totalCost;
        }
    }

    //     double totalLatenessCost = _totalCost - saveTotalCost;
    //     setComponentScore("LatenessCost", (int)totalLatenessCost);
    setComponentScore("OpportunityCost", (int)oppCost);
    setComponentScore("InventoryCost", (int)invCost);
    setComponentScore("LatenessCost", (int)latCost);
    if (_audit)
    {
        ComponentScoreInfo* info1 = new ComponentScoreInfo();
        info1->name = "OpportunityCost";
        info1->score = oppCost; //totalLatenessCost;
        _auditReport->componentInfos()->push_back(info1);
        ComponentScoreInfo* info2 = new ComponentScoreInfo();
        info2->name = "InventoryCost";
        info2->score = invCost; //totalLatenessCost;
        _auditReport->componentInfos()->push_back(info2);
        ComponentScoreInfo* info3 = new ComponentScoreInfo();
        info3->name = "LatenessCost";
        info3->score = latCost; //totalLatenessCost;
        _auditReport->componentInfos()->push_back(info3);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
TotalCostEvaluator::calcFixedCost(const SchedulingContext& context) const
{
    if (_audit)
    {
        *_os << heading("calcFixedCost()", '-', 75) << std::endl;
    }

    double saveTotalCost = _totalCost;
    const jobop_set_id_t& ops = context.clevorDataSet()->ops();
    jobop_set_id_t::const_iterator it;
    for (it = ops.begin(); it != ops.end(); ++it)
    {
        JobOp* op = *it;

        // op must be active and non-summary and unstarted
        if (!op->job()->active() || (op->type() == op_summary) ||
            (op->status() != opstatus_unstarted))
        {
            continue;
        }

        // op must have a cost
        double opCost = op->cost();
        if (opCost == 0.0)
        {
            continue;
        }

        // op must have an activity
        if (op->activity() == nullptr)
            continue;
        Activity* act = op->activity();

        // find SpanInterestPeriod where activity starts to execute
        Span<int> searchSpan;
        int es = act->es();
        searchSpan.set(es, es + 1);
        spanip_col_t::iterator it = _spanIPs.findFirstIt(searchSpan);
        ASSERTD(it != _spanIPs.end());
        SpanInterestPeriod* sip = (SpanInterestPeriod*)*it;

        uint_t ip = sip->period();
        if (_audit)
        {
            *_os << "Fixed Cost (op id = " << op->id() << "): "
                 << "$" << opCost << std::endl;
            *_os << "    i.p. " << ip << ": $" << _ipCost[ip];
            FixedCostInfo*& info = (*_auditReport->fixedCosts())[op->id()];
            info = new FixedCostInfo();
            info->opId = op->id();
            info->interestPeriod = ip;
            info->cost = opCost;
        }
        _ipCost[ip] += opCost;
        _totalCost += opCost;
        if (_audit)
        {
            *_os << " + $" << opCost << " = $" << _ipCost[ip] << std::endl;
        }
    }
    double totalFixedCost = _totalCost - saveTotalCost;
    setComponentScore("FixedCost", (int)totalFixedCost);
    if (_audit)
    {
        ComponentScoreInfo* info = new ComponentScoreInfo();
        info->name = "FixedCost";
        info->score = totalFixedCost;
        _auditReport->componentInfos()->push_back(info);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
TotalCostEvaluator::calcResourceSequenceCost(const SchedulingContext& context) const
{
    if (_audit)
    {
        *_os << heading("calcResourceSequenceCost()", '-', 75) << std::endl;
    }

    double saveTotalCost = _totalCost;
    const ClevorDataSet* dataSet = context.clevorDataSet();
    const res_set_id_t& resources = dataSet->resources();
    res_set_id_t::const_iterator it;
    for (it = resources.begin(); it != resources.end(); ++it)
    {
        DiscreteResource* res = dynamic_cast<DiscreteResource*>(*it);
        if (res == nullptr)
            continue;
        const rsra_vector_t& rsras = res->sequenceRuleApplications();
        rsra_vector_t::const_iterator raIt;
        for (raIt = rsras.begin(); raIt != rsras.end(); ++raIt)
        {
            const ResourceSequenceRuleApplication& app = *raIt;
            const ResourceSequenceRule* rule = app.rule();
            double cost = rule->cost();
            if (cost == 0.0)
                continue;
            cls::Activity* lhsAct = app.lhsOp()->activity();
            cls::Activity* rhsAct = app.rhsOp()->activity();
            if ((lhsAct == nullptr) || (rhsAct == nullptr))
                continue;

            // impose the cost
            int costTS = lhsAct->ef();
            Span<int> searchSpan(costTS, costTS + 1);
            spanip_col_t::iterator it = _spanIPs.findFirstIt(searchSpan);
            ASSERTD(it != _spanIPs.end());
            SpanInterestPeriod* sip = (SpanInterestPeriod*)*it;
            uint_t ip = sip->period();
            if (_audit)
            {
                *_os << "Resource Sequence Cost "
                     << "(res-id = " << res->id() << ", lhs-op-id = " << lhsAct->id()
                     << ", rhs-op-id = " << rhsAct->id() << "): "
                     << "$" << cost << std::endl;
                *_os << "    i.p. " << ip << ": $" << _ipCost[ip];
                ResourceSequenceCostInfo* info = new ResourceSequenceCostInfo();
                info->id = res->id();
                info->lhsId = lhsAct->id();
                info->rhsId = rhsAct->id();
                info->cost = cost;
                info->ip = ip;
                _auditReport->resourceSequenceCosts()->push_back(info);
            }
            _ipCost[ip] += cost;
            _totalCost += cost;
            if (_audit)
            {
                *_os << " + $" << cost << " = $" << _ipCost[ip] << std::endl;
            }
        }
    }
    double totalResourceSequenceCost = _totalCost - saveTotalCost;
    setComponentScore("ResourceSequenceCost", (int)totalResourceSequenceCost);
    if (_audit)
    {
        ComponentScoreInfo* info = new ComponentScoreInfo();
        info->name = "ResourceSequenceCost";
        info->score = totalResourceSequenceCost;
        _auditReport->componentInfos()->push_back(info);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
TotalCostEvaluator::calcOverheadCost(const SchedulingContext& context) const
{
    if (_audit)
    {
        *_os << heading("calcOverheadCost()", '-', 75) << std::endl;
    }
    double saveTotalCost = _totalCost;
    // Note: Overhead cost is always calculated from the original time.
    // Joe, Nov. 11, 2007
    Span<int> makespan(_originTS, context.makespanTimeSlot());

    // We need to roundUp makespan by a day..
    // For example, if a makespan starts from 10am Nov. 11 and finishes
    // at 1pm Nov. 12, we calculates overhead cost of 2 days.
    // But if it finishes at 9am, Nov. 12, we will calculate it as 1 day.
    // Joe, Nov 11, 2007.
    int tsPerDay = daySec / (uint_t)_schedulerConfig->timeStep();
    makespan.setEnd(roundUp(makespan.end(), tsPerDay));

    if (_audit)
    {
        *_os << "Overhead Cost" << std::endl;
    }
    calcPeriodCost(makespan, _overheadCostPerTS);
    double totalOverheadCost = _totalCost - saveTotalCost;
    setComponentScore("OverheadCost", (int)totalOverheadCost);
    if (_audit)
    {
        std::map<uint_t, double>::const_iterator i;
        for (i = _auditIpCosts.begin(); i != _auditIpCosts.end(); i++)
        {
            OverheadCostInfo* info = new OverheadCostInfo();
            info->interestPeriod = i->first;
            info->cost = i->second;
            _auditReport->overheadCosts()->push_back(info);
        }
        ComponentScoreInfo* info = new ComponentScoreInfo();
        info->name = "OverheadCost";
        info->score = totalOverheadCost;
        _auditReport->componentInfos()->push_back(info);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
TotalCostEvaluator::calcInterestCost(const SchedulingContext& context) const
{
    if (_audit)
    {
        *_os << heading("calcInterestCost()", '-', 75) << std::endl;
    }
    int makeSpanEnd = context.makespanTimeSlot();
    double saveTotalCost = _totalCost;
    double totalCost = 0.0;
    uint_t ip;
    double interestCost;
    for (spanip_col_t::iterator it = _spanIPs.begin(); it != _spanIPs.end(); ++it)
    {
        SpanInterestPeriod* sip = (SpanInterestPeriod*)*it;
        ip = sip->period();
        interestCost = 0.0;
        if (sip->begin() <= makeSpanEnd)
        {
            interestCost = totalCost * _interestRate;
        }
        if (_audit)
        {
            *_os << "    i.p. " << ip << ": $" << _ipCost[ip];
        }
        _ipCost[ip] += interestCost;
        _totalCost += interestCost;
        if (_audit)
        {
            *_os << " + $" << interestCost << " = $" << _ipCost[ip] << std::endl;
            InterestCostInfo* info = new InterestCostInfo();
            info->interestPeriod = ip;
            info->cost = interestCost;
            _auditReport->interestCosts()->push_back(info);
        }
        totalCost += _ipCost[ip];
    }
    double totalInterestCost = _totalCost - saveTotalCost;
    setComponentScore("InterestCost", (int)totalInterestCost);
    if (_audit)
    {
        ComponentScoreInfo* info = new ComponentScoreInfo();
        info->name = "InterestCost";
        info->score = totalInterestCost;
        _auditReport->componentInfos()->push_back(info);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
TotalCostEvaluator::calcPeriodCost(const utl::Span<int>& p_span, double costPerTS) const
{
    if (_audit)
    {
        _auditIpCosts.clear();
    }
    if (costPerTS == 0.0)
    {
        return;
    }

    Span<int> span = p_span;

    double saveTotalCost = _totalCost;
    if (_audit)
    {
        uint_t timeStep = _schedulerConfig->timeStep();
        time_t originTime = _schedulerConfig->originTime();
        time_t beginTime = originTime + (span.begin() * timeStep);
        time_t endTime = originTime + (span.end() * timeStep);
        *_os << " => calcPeriodCost(" << time_str(beginTime) << ", " << time_str(endTime) << ", "
             << "$" << costPerTS << ")" << std::endl;
    }

    for (spanip_col_t::iterator it = _spanIPs.findFirstIt(span);
         (it != _spanIPs.end()) && !span.isNil(); ++it)
    {
        const SpanInterestPeriod& sip = (const SpanInterestPeriod&)**it;
        Span<int> overlap = sip.overlap(span);
        span.setBegin(overlap.end());
        uint_t ip = sip.period();
        double cost = (costPerTS * (double)overlap.size());
        if (_audit)
        {
            *_os << "    i.p. " << ip << ": $" << _ipCost[ip];
        }
        _ipCost[ip] += cost;
        _totalCost += cost;
        if (_audit)
        {
            *_os << " + $" << cost << " = $" << _ipCost[ip] << std::endl;
            _auditIpCosts[ip] = cost;
        }
    }

    if (_audit)
    {
        *_os << "    total: $" << (_totalCost - saveTotalCost) << std::endl;
    }
}

/* November 21, 2013 (Elisa) */
/* this verion on the fuction includes the lateness cost increment */
void
TotalCostEvaluator::calcPeriodCost(const utl::Span<int>& p_span,
                                   double costPerTS,
                                   double incrCost,
                                   double periodS,
                                   double tStep) const
{
    if (_audit)
    {
        _auditIpCosts.clear();
    }
    if (costPerTS == 0.0)
    {
        return;
    }

    Span<int> span = p_span;

    double saveTotalCost = _totalCost;
    if (_audit)
    {
        uint_t timeStep = _schedulerConfig->timeStep();
        time_t originTime = _schedulerConfig->originTime();
        time_t beginTime = originTime + (span.begin() * timeStep);
        time_t endTime = originTime + (span.end() * timeStep);
        *_os << " => calcPeriodCost(" << time_str(beginTime) << ", " << time_str(endTime) << ", "
             << "$" << costPerTS << ", "
             << "%" << incrCost << ", " << periodS << ", " << timeStep << ")" << std::endl;
    }

    for (spanip_col_t::iterator it = _spanIPs.findFirstIt(span);
         (it != _spanIPs.end()) && !span.isNil(); ++it)
    {
        const SpanInterestPeriod& sip = (const SpanInterestPeriod&)**it;
        Span<int> overlap = sip.overlap(span);
        span.setBegin(overlap.end());
        uint_t ip = sip.period();
        double cost = (costPerTS * (double)overlap.size());
        double firstcost = (costPerTS * (double)overlap.size());
        double realtStep = (double)(periodS / tStep);
        if (_audit)
        {
            //*_os << "    i.p. " << ip << ": " << (double)overlap.size()
            //     << " " << periodS << " " << tStep
            //     << " " << realtStep << " $" << _ipCost[ip] << std::endl;
            *_os << "    i.p. " << ip << ": $" << _ipCost[ip];
        }

        if (realtStep <= (double)overlap.size() && incrCost > (double)1.0)
        {
            cost = 0.0;
            double realcostPerTS = costPerTS;
            firstcost = (realcostPerTS * realtStep);

            for (double i = realtStep; i <= (double)overlap.size(); i += realtStep)
            {
                cost = cost + (realcostPerTS * realtStep);
                if (_audit)
                {
                    //*_os << "             + $" << (realcostPerTS * realtStep)
                    //     << " " << realcostPerTS << " realtStep:" << realtStep << std::endl;
                }
                realcostPerTS = realcostPerTS * incrCost;
                if ((i + realtStep) > (double)overlap.size())
                {
                    double remaining = (double)overlap.size() - i;
                    cost = cost + (realcostPerTS * remaining);
                    if (_audit)
                    {
                        //*_os << "             + $" << (realcostPerTS * remaining)
                        //      << " " << realcostPerTS << " remaining:" << remaining << std::endl;
                    }
                }
            }
        }
        else
        {
            if (_audit)
            {
                //*_os << "             + $" << firstcost << std::endl;
                //*_os << "             + $" << (cost-firstcost) << std::endl;
            }
        }
        //if (_audit)
        //{
        //  *_os << "    i.p. " << ip << ": " << (double)overlap.size()
        //       << " " << periodS << " " << tStep
        //       << " " << (periodS/tStep) << " $" << _ipCost[ip];
        //}
        _ipCost[ip] += cost;
        _totalCost += cost;
        if (_audit)
        {
            //*_os << "               = $" << _ipCost[ip] << std::endl;
            *_os << " + " << cost << " = $" << _ipCost[ip] << std::endl;
            _auditIpCosts[ip] = cost;
        }
    }

    if (_audit)
    {
        *_os << "    total: $" << (_totalCost - saveTotalCost) << std::endl;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Joe rewrote cslistBuild on August 21, 2008
// Note: resCost->resolution() is not considered, because cslistCost will roundup
//       a day's working time to hours anyway when hourly rate is given.
//       we need to do further modifications to this file in the future when
//       res->resolution() is bigger than a hour.
////////////////////////////////////////////////////////////////////////////////////////////////////

void
TotalCostEvaluator::cslistBuild(const SchedulingContext& context,
                                const cls::DiscreteResource& res,
                                cslist_t& cslist) const
{
#ifdef DEBUG_UNIT
    std::string myName("TotalCostEvaluator::cslistBuild(): ");
    std::cout << myName << "res.id = " << res.id() << std::endl;
#endif

    Span<int> scheduleSpan(_originTS, _horizonTS);
    Span<int> makespan(_originTS, context.makespanTimeSlot());
    scheduleSpan.clip(makespan);

    ResourceCost* resCost = (ResourceCost*)res.object();
    ASSERTD(resCost != nullptr);
    uint_t rts = utl::max((uint_t)1, resCost->resolution() / _timeStep);
    uint_t mets = resCost->minEmploymentTime() / _timeStep;
    uint_t mits = resCost->maxIdleTime() / _timeStep;

    //extend scheduleSpan if mets > 0
    if (mets > 0)
    {
        int et = res.getEndTimeForStartTime(scheduleSpan.end(), mets) + 1;
        scheduleSpan.setEnd(et);
    }
    if (_audit)
    {
        std::string title("calcResourceCost(id = ");
        uint_t resId = res.id();
        double hireCost = resCost->costPerUnitHired();
        double hourCost = resCost->costPerHour();
        double dayCost = resCost->costPerDay();
        double weekCost = resCost->costPerWeek();
        double monthCost = resCost->costPerMonth();

        char resIdStr[128];
        sprintf(resIdStr, "%u", resId);
        title.append(resIdStr);
        title.append(")");
        *_os << heading(title, '-', 75) << std::endl;
        *_os << "Resolution: " << rts << " time-slots" << std::endl;
        *_os << "Max-Idle: " << mits << " time-slots" << std::endl;
        *_os << "Min-Employment: " << mets << " time-slots" << std::endl;
        *_os << "Cost-per-unit-hired: " << hireCost << std::endl;
        *_os << "Cost-per-hour: $" << hourCost << std::endl;
        *_os << "Cost-per-day: $" << dayCost << std::endl;
        *_os << "Cost-per-week: $" << weekCost << std::endl;
        *_os << "Cost-per-month: $" << monthCost << std::endl;

        resInfos_t::iterator it = _auditReport->resourceInfos()->find(resId);
        if (it == _auditReport->resourceInfos()->end())
        {
            // Not found, create it.
            ResourceInfo* rcInfo = new ResourceInfo();
            rcInfo->id = resId;
            rcInfo->name = res.name();
            rcInfo->rts = rts;
            rcInfo->mits = mits;
            rcInfo->mets = mets;
            rcInfo->costPerUnitHired = hireCost;
            rcInfo->costPerHour = hourCost;
            rcInfo->costPerDay = dayCost;
            rcInfo->costPerWeek = weekCost;
            rcInfo->costPerMonth = monthCost;
            _auditReport->resourceInfos()->insert(resInfos_t::value_type(resId, rcInfo));
        }
    }

    // t: time-point for checking
    int t = scheduleSpan.begin();
    const ClevorDataSet* dataSet = context.clevorDataSet();
    const DiscreteResource* dres = dynamic_cast<DiscreteResource*>(dataSet->findResource(res.id()));
    uint_t maxCap = roundUp(dres->clsResource()->maxReqCap(), (uint_t)100);
    maxCap = maxCap / 100;
    //     bool employed[maxCap];
    std::vector<bool> canFired(maxCap, false), alreadyIdle(maxCap, false);
    std::vector<int> employStarts(maxCap, 0), minEmployEnds(maxCap, 0);
    std::vector<int> idleStarts(maxCap, 0), maxIdleEnds(maxCap, 0);
    CapSpan* lastCapSpan = nullptr;
    int reqCap, lastReqCap = 0;
    uint_t existingCap = dres->existingCap();
    existingCap = (uint_t)ceil((double)existingCap / 100.0);

    // Build cslist from resource's timetable.
    const DiscreteTimetable& tt = res.timetable();
    const IntSpan* tail = tt.tail();
    CapSpan* capSpan;
    const IntSpan* ts;
    t = scheduleSpan.begin();

    while (t < scheduleSpan.end())
    {
        ts = tt.find(t);

        Span<int> tsSpan = ts->span();
        Span<int> overlap = tsSpan.overlap(scheduleSpan);

        // reset overlap's st
        ASSERTD(t < overlap.end());
        overlap.setBegin(t);

        // convert reqCap
        reqCap = ts->v0();
        double tmp = (double)reqCap / 100.0;
        reqCap = (int)ceil(tmp);

        // step 1: dealing with existing-cap
        if (cslist.empty() && existingCap > 0 && (mets > 0 || mits > 0))
        {
            if (mets > 0)
            {
                for (int i = 0; i < (int)existingCap; i++)
                {
                    //                     employed[i] = true;
                    canFired[i] = false;
                    employStarts[i] = t;
                    minEmployEnds[i] = res.getEndTimeForStartTime(employStarts[i], mets);
                    minEmployEnds[i] = res.getNonBreakTimeNext(minEmployEnds[i]);
                }
            }
            //             if (mits > 0 && reqCap < existingCap)
            //             {
            //                 for (int i = reqCap; i < existingCap; i++)
            //                 {
            //                     idleStarts[i] = t;
            //                     int tmp = res.getEndTimeForStartTime(idleStarts[i], mits) + 1;
            //                     maxIdleEnds[i] = res.getNonBreakTimeNext(tmp);
            //                 }
            //             }
            lastReqCap = existingCap;
        }

        // step 2: dealing with min-employment and max-idle-time
        int payCap = reqCap;
        if (reqCap > lastReqCap)
        {
            for (int i = 0; i < lastReqCap; i++)
            {
                alreadyIdle[i] = false;
            }
            for (int i = lastReqCap; i < reqCap; i++)
            {
                //                 employed[i] = true;
                canFired[i] = false;
                employStarts[i] = overlap.begin();
                minEmployEnds[i] = res.getEndTimeForStartTime(employStarts[i], mets);
                minEmployEnds[i] = res.getNonBreakTimeNext(minEmployEnds[i]);
                alreadyIdle[i] = false;
            }
            t = overlap.end(); //t updated
        }
        else if (reqCap <= lastReqCap)
        {
            int spanEnd = overlap.end();
            for (int i = reqCap; i < lastReqCap; i++)
            {
                int newPayCap = payCap;
                if (mets > 0)
                {
                    if (!canFired[i])
                    {
                        if (minEmployEnds[i] > overlap.begin())
                        {
                            newPayCap++;
                            spanEnd = min(spanEnd, minEmployEnds[i]);
                        }
                        else
                        {
                            canFired[i] = true;
                        }
                    }
                }
                if (mits > 0)
                {
                    if (alreadyIdle[i] == false)
                    {
                        idleStarts[i] = tsSpan.begin();
                        maxIdleEnds[i] = res.getEndTimeForStartTime(idleStarts[i], mits);
                        maxIdleEnds[i] = res.getNonBreakTimeNext(maxIdleEnds[i]);
                        alreadyIdle[i] = true;
                    }
                    if (newPayCap == payCap)
                    {
                        const IntSpan* nextTs = ts->next();
                        while (nextTs != tail && maxIdleEnds[i] >= nextTs->span().begin())
                        {
                            int nextTsCap = (int)ceil((double)nextTs->v0() / 100.0);
                            if (nextTsCap > payCap)
                            {
                                newPayCap++;
                                break;
                            }
                            nextTs = nextTs->next();
                        }
                    }
                    //                     if (maxIdleEnds[i] >= nextTs->span().getBegin() &&
                    //                         nextTsCap >= payCap)
                    //                     {
                    //                         if (newPayCap == payCap) newPayCap++;
                    //                         spanEnd = min(spanEnd, maxIdleEnds[i]);
                    //                     }
                }
                payCap = newPayCap;
            }
            t = spanEnd; // t updated
                         //             t = min(spanEnd, overlap.getEnd());
        }

        // merge with previous capSpan if
        // 1) has the same cap
        // 2) this capSpan's total nonbreaktime = 0 AND
        //    its cap = 0
        // note: it's necessary to remove 2) case, because the next span will re-hire this cap and apply mets to it.
        if (lastCapSpan != nullptr &&
            (payCap == lastReqCap ||
             (res.getNonBreakTime((int)overlap.begin(), (int)t - 1) == 0 && payCap == 0)))
        {
            capSpan = lastCapSpan;
            capSpan->end() = t;
            cslist.pop_back();
        }
        else
        {
            capSpan = new CapSpan(payCap, overlap.begin(), t);
        }
        cslist.push_back(capSpan);
        lastCapSpan = capSpan;       // lastCapSpan updated
        lastReqCap = capSpan->cap(); // lastReqCap updated
    }
#ifdef DEBUG_UNIT
    std::cout << "----- cslistBuild finished: ----" << std::endl;
    cslistDump(cslist);
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/* January 3, 2014 (Elisa) */
/* code to calculate the overhead for each job */
void
TotalCostEvaluator::calcJobOverheadCost(const SchedulingContext& context) const
{
    if (_audit)
    {
        *_os << heading("calcJobOverheadCost()", '-', 75) << std::endl;
        _auditIpCosts.clear();
    }
    double saveTotalCost = _totalCost;
    double jobovrdCost = 0.0;

    const job_set_id_t& jobs = context.clevorDataSet()->jobs();
    job_set_id_t::const_iterator it;
    for (it = jobs.begin(); it != jobs.end(); ++it)
    {
        const Job* job = *it;
        if (job->dueTime() == -1 || !job->active())
            continue;
        int makespan = job->makespan();
        int minStartTime = 99999;
        for (Job::const_iterator it = job->begin(); it != job->end(); ++it)
        {
            JobOp* op = *it;
            if (op->type() == op_summary)
                continue;
            if (op->ignorable())
                continue;
            Activity* act = op->activity();
            if (act == nullptr)
                continue;

            int startTime = act->es() + 1;
            if (startTime < minStartTime)
                minStartTime = startTime;
        }

        // overhead cost
        double overheadCostPerTS = 0.0;
        if (job->overheadCostPeriod() != period_undefined)
        {
            uint_t periodSeconds;
            periodSeconds = periodToSeconds(job->overheadCostPeriod());
            overheadCostPerTS = job->overheadCost() / ((double)periodSeconds / (double)_timeStep);
        }
        if (_audit)
        {
            *_os << "WorkOrder id = " << job->id() << " name = " << job->name() << ": "
                 << "Job Overhead Cost" << std::endl;
        }
        calcJobPeriodCost(Span<int>(minStartTime, makespan), overheadCostPerTS);
        if (_audit)
        {
            JobOverheadCostReport* report = _auditReport->joboverheadCost(job->id());
            if (report->getName().empty())
            {
                report->setName(job->name());
            }
            std::map<uint_t, double>::const_iterator i;
            for (i = _auditIpCosts.begin(); i != _auditIpCosts.end(); i++)
            {
                JobOverheadCostInfo* info = new JobOverheadCostInfo();
                info->interestPeriod = i->first;
                info->cost = i->second;
                report->costs()->push_back(info);
            }
            jobovrdCost += _totalCost - saveTotalCost;
            saveTotalCost = _totalCost;
        }
    }
    setComponentScore("JobOverheadCost", (int)jobovrdCost);
    if (_audit)
    {
        *_os << "Total Job Overhead Cost: $" << jobovrdCost << std::endl;
        ComponentScoreInfo* info = new ComponentScoreInfo();
        info->name = "JobOverheadCost";
        info->score = jobovrdCost;
        _auditReport->componentInfos()->push_back(info);
    }
}

void
TotalCostEvaluator::calcJobPeriodCost(const utl::Span<int>& p_span, double costPerTS) const
{
    if (_audit)
    {
        _auditIpCosts.clear();
    }
    if (costPerTS == 0.0)
        return;

    Span<int> span = p_span;

    double saveTotalCost = _totalCost;
    if (_audit)
    {
        uint_t timeStep = _schedulerConfig->timeStep();
        time_t originTime = _schedulerConfig->originTime();
        time_t beginTime = originTime + (span.begin() * timeStep);
        time_t endTime = originTime + (span.end() * timeStep);
        *_os << " => calcJobPeriodCost(" << time_str(beginTime) << ", " << time_str(endTime) << ", "
             << "$" << costPerTS << ")" << std::endl;
    }
    for (spanip_col_t::iterator it = _spanIPs.findFirstIt(span);
         (it != _spanIPs.end()) && !span.isNil(); ++it)
    {
        const SpanInterestPeriod& sip = (const SpanInterestPeriod&)**it;
        Span<int> overlap = sip.overlap(span);
        span.setBegin(overlap.end());
        uint_t ip = sip.period();
        double cost = 0.0;
        if (costPerTS != 0.0)
            cost = (costPerTS * (double)overlap.size());
        //double cost = (costPerTS * (double)overlap.size());
        if (_audit)
        {
            *_os << "    i.p. " << ip << ": $" << _ipCost[ip];
        }
        _ipCost[ip] += cost;
        _totalCost += cost;
        if (_audit)
        {
            *_os << " + $" << cost << " = $" << _ipCost[ip] << std::endl;
            // _auditIpCosts[ip] += cost;
            _auditIpCosts[ip] = cost;
        }
    }
    if (_audit)
    {
        *_os << "    total: $" << (_totalCost - saveTotalCost) << std::endl;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
CSE_NS_END;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(cse::SpanInterestPeriod);
UTL_CLASS_IMPL(cse::TotalCostEvaluator);
