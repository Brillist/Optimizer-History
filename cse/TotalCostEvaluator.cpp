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

struct CapCostRec
{
    CapCostRec(uint_t cap)
        : cap(cap)
        , capIdx(0)
        , startTime(0)
        , endTime(0)
    {
    }
public:
    uint_t cap;
    uint_t capIdx;
    int startTime;
    int endTime;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
/// CapCostRecOrdering /////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

struct CapCostRecOrdering
{
    bool
    operator()(const CapCostRec* lhs, const CapCostRec* rhs) const
    {
        if (lhs->startTime != rhs->startTime)
        {
            return (lhs->startTime < rhs->startTime);
        }
        return (lhs->cap < rhs->cap);
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////
/// CapCostRecCapOrdering //////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

struct CapCostRecCapOrdering
{
    bool
    operator()(const CapCostRec* lhs, const CapCostRec* rhs) const
    {
        return (lhs->cap < rhs->cap);
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////
/// CapSpan ////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

struct CapSpan
{
    CapSpan(uint_t cap, const Span<int>& span)
        : cap(cap)
        , begin(span.begin())
        , end(span.end())
    {
    }

    CapSpan(uint_t cap, int begin, int end)
        : cap(cap)
        , begin(begin)
        , end(end)
    {
    }

    uint_t
    size() const
    {
        return end - begin;
    }
public:
    uint_t cap;
    int begin;
    int end;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

using ccr_map_t = std::unordered_map<uint_t, CapCostRec*>;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// SpanInterestPeriod /////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

class SpanInterestPeriod : public utl::Span<int>
{
    UTL_CLASS_DECL(SpanInterestPeriod, utl::Span<int>);

public:
    SpanInterestPeriod(int begin, int end, uint_t period)
        : Span<int>(begin, end)
        , period(period)
    {
        setRelaxed(true);
    }

    virtual void copy(const utl::Object& rhs);

public:
    uint_t period;

private:
    void init()
    {
        period = lut::period_undefined;
        setRelaxed(true);
    }
    void
    deInit()
    {
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////

void
SpanInterestPeriod::copy(const Object& rhs)
{
    auto& ip = utl::cast<SpanInterestPeriod>(rhs);
    super::copy(ip);
    period = ip.period;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// TotalCostEvaluator /////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void
TotalCostEvaluator::copy(const Object& rhs)
{
    auto& tce = utl::cast<TotalCostEvaluator>(rhs);
    super::copy(tce);

    // copy basic config that was set by initialize()
    _originTS = tce._originTS;
    _horizonTS = tce._horizonTS;
    _timeStep = tce._timeStep;
    _forward = tce._forward;
    _overheadCostPerTS = tce._overheadCostPerTS;
    _interestRate = tce._interestRate;
    _numIPs = tce._numIPs;

    // create _ipCosts[]
    delete[] _ipCosts;
    _ipCosts = nullptr;
    if (_numIPs > 0)
    {
        _ipCosts = new double[_numIPs];
    }

    // copy _ipSpans
    _ipSpans = tce._ipSpans;

    // everything else should be null or zero...

    // _dayIsBreak
    ASSERTD(_dayIsBreak == tce._dayIsBreak);
    ASSERTD(_dayIsBreakSize == tce._dayIsBreakSize);
    // _caps
    ASSERTD(_caps == tce._caps);
    ASSERTD(_capsSize == tce._capsSize);
    // _capDayTimes
    ASSERTD(_capDayTimes == tce._capDayTimes);
    ASSERTD(_capDayTimesSize == tce._capDayTimesSize);
    // _capDayHires
    ASSERTD(_capDayHires == tce._capDayHires);
    ASSERTD(_capDayHiresSize == tce._capDayHiresSize);
    // _capDayFireBeforeHire
    ASSERTD(_capDayFireBeforeHire == tce._capDayFireBeforeHire);
    ASSERTD(_capDayFireBeforeHireSize == tce._capDayFireBeforeHireSize);
    // _dayCosts
    ASSERTD(_dayCosts == tce._dayCosts);
    ASSERTD(_dayCostsSize == tce._dayCostsSize);
    // _dayCostPeriods
    ASSERTD(_dayCostPeriods == tce._dayCostPeriods);
    ASSERTD(_dayCostPeriodsSize == tce._dayCostPeriodsSize);
    // _auditReport
    ASSERTD(_auditReport == tce._auditReport);
    ASSERTD(_auditIpCosts.empty() && tce._auditIpCosts.empty());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::string
TotalCostEvaluator::name() const
{
    return "TotalCost";
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
TotalCostEvaluator::initialize(const IndEvaluatorConfiguration* p_cf)
{
    auto& cf = utl::cast<TotalCostEvaluatorConfiguration>(*p_cf);
    super::initialize(cf);

    // origin, horizon, due-time
    auto schedulerConfig = cf.getSchedulerConfig();
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

    // populate _ipSpans[]
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

        // same IP as last iteration?
        if ((ip == spanIP) || (spanIP == uint_t_max))
        {
            // yes -> extend span
            spanEnd = ts;
        }
        else
        {
            // no -> complete existing IP, start new IP
            _ipSpans.add(new SpanInterestPeriod(spanBegin, spanEnd + 1, spanIP));
            spanBegin = spanEnd = ts;
        }
        spanIP = ip;
    }
    // loop is done -> complete the last IP
    if (spanIP != uint_t_max)
    {
        _ipSpans.add(new SpanInterestPeriod(spanBegin, spanEnd + 1, spanIP));
    }

    // create _ipCosts[] (same size as _ipSpans)
    _numIPs = _ipSpans.size();
    delete[] _ipCosts;
    _ipCosts = new double[_numIPs];
}

////////////////////////////////////////////////////////////////////////////////////////////////////

double
TotalCostEvaluator::calcScore(const IndBuilderContext* p_context) const
{
    auto& context = utl::cast<SchedulingContext>(*p_context);

    // zero the cost vector
    _totalCost = 0.0;
    for (uint_t i = 0; i != _numIPs; ++i)
    {
        _ipCosts[i] = 0.0;
    }

    // auditing -> create output stream & print general information to it
    //             create _auditReport
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
        for (auto sip_ : _ipSpans)
        {
            auto sip = utl::cast<SpanInterestPeriod>(sip_);
            time_t begin = origTime + (sip->begin() * timeStep);
            time_t end = origTime + (sip->end() * timeStep);
            *_os << "i.p. " << ipIdx << ": "
                 << "[" << time_str(begin) << "," << time_str(end) << ")" << std::endl;
            auto info = new InterestPeriodInfo();
            info->idx = ipIdx;
            info->begin = begin;
            info->end = end;
            _auditReport->interestPeriods()->push_back(info);
            ++ipIdx;
        }
    }

    // resource cost
    calcResourceCost(context);

    // opportunity/lateness cost
    calcLatenessCost(context);

    // overhead for each job
    calcJobOverheadCost(context);

    // operation cost
    calcFixedCost(context);

    // resource sequence cost
    calcResourceSequenceCost(context);

    // overhead cost
    calcOverheadCost(context);

    // interest cost
    calcInterestCost(context);

    // auditing?
    if (_audit)
    {
        // audit must be explicitly enabled
        _audit = false;

        // _auditReport has an OperationInfo for each op
        auto& ops = context.clevorDataSet()->ops();
        for (auto op : ops)
        {
            auto info = new OperationInfo();
            info->id = op->id();
            info->name = op->name();
            _auditReport->operationInfos()->insert(opInfos_t::value_type(info->id, info));
        }

        // record audit text in _auditText, delete audit output stream
        *_os << '\0';
        _auditText = _os->str();
        delete _os;

        // record the total cost in _auditReport
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
    _ipCosts = nullptr;
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
    delete[] _ipCosts;
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
    auto clsSchedule = context.schedule();

    // for each cls::Resource
    auto resourcesEnd = clsSchedule->resourcesEnd();
    for (auto rit = clsSchedule->resourcesBegin(); rit != resourcesEnd; ++rit)
    {
        // only calculate cost for discrete resources that have cost defined
        auto res = *rit;
        if (!res->isA(cls::DiscreteResource) || (res->object() == nullptr))
        {
            continue;
        }

        // calculate cost
        const auto& dres = utl::cast<cls::DiscreteResource>(*res);
        cslist_t cslist;
        cslistBuild(context, dres, cslist);
        cslistCost(context, dres, cslist);
        deleteCont(cslist);
    }

    // increase in _totalCost is recorded as ResourceCost
    double totalResourceCost = _totalCost - saveTotalCost;
    setComponentScore("ResourceCost", (int)totalResourceCost);

    // auditing -> record resource cost in _auditReport
    if (_audit)
    {
        auto info = new ComponentScoreInfo();
        info->name = "ResourceCost";
        info->score = totalResourceCost;
        _auditReport->componentInfos()->push_back(info);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

// Note: resCost->resolution() is not considered, because cslistCost will round up
//       .. a day's working time to hours anyway when hourly rate is given.
//       We will need to change this code to accommodate resCost->resolution > 1 hr.
void
TotalCostEvaluator::cslistBuild(const SchedulingContext& context,
    const cls::DiscreteResource& res,
    cslist_t& cslist) const
{
#ifdef DEBUG_UNIT
    std::string myName("TotalCostEvaluator::cslistBuild(): ");
    std::cout << myName << "res.id = " << res.id() << std::endl;
#endif

    // scheduleSpan: [originTS, makespanTS)
    Span<int> scheduleSpan(_originTS, min(_horizonTS, context.makespanTimeSlot()));

    auto resCost = utl::cast<ResourceCost>(res.object());
    ASSERTD(resCost != nullptr);
    uint_t resolutionSlots = utl::max((uint_t)1, resCost->resolution() / _timeStep);
    uint_t minEmploySlots = resCost->minEmploymentTime() / _timeStep;
    uint_t maxIdleSlots = resCost->maxIdleTime() / _timeStep;

    // extend scheduleSpan if minEmploySlots > 0
    if (minEmploySlots > 0)
    {
        int et = res.getEndTimeForStartTime(scheduleSpan.end(), minEmploySlots) + 1;
        scheduleSpan.setEnd(et);
    }

    // auditing -> print this resource's ResourceCost info, record it in _auditReport
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
        *_os << "Resolution: " << resolutionSlots << " time-slots" << std::endl;
        *_os << "Max-Idle: " << maxIdleSlots << " time-slots" << std::endl;
        *_os << "Min-Employment: " << maxIdleSlots << " time-slots" << std::endl;
        *_os << "Cost-per-unit-hired: " << hireCost << std::endl;
        *_os << "Cost-per-hour: $" << hourCost << std::endl;
        *_os << "Cost-per-day: $" << dayCost << std::endl;
        *_os << "Cost-per-week: $" << weekCost << std::endl;
        *_os << "Cost-per-month: $" << monthCost << std::endl;

        auto it = _auditReport->resourceInfos()->find(resId);
        if (it == _auditReport->resourceInfos()->end())
        {
            // not found -> create it.
            auto rcInfo = new ResourceInfo();
            rcInfo->id = resId;
            rcInfo->name = res.name();
            rcInfo->resolutionSlots = resolutionSlots;
            rcInfo->maxIdleSlots = maxIdleSlots;
            rcInfo->minEmploySlots = minEmploySlots;
            rcInfo->costPerUnitHired = hireCost;
            rcInfo->costPerHour = hourCost;
            rcInfo->costPerDay = dayCost;
            rcInfo->costPerWeek = weekCost;
            rcInfo->costPerMonth = monthCost;
            _auditReport->resourceInfos()->insert(resInfos_t::value_type(resId, rcInfo));
        }
    }

    int t = scheduleSpan.begin(); // current time slot
    auto dataSet = context.clevorDataSet();
    auto dres = utl::cast<DiscreteResource>(dataSet->findResource(res.id()));
    uint_t maxCap = roundUp(dres->clsResource()->maxReqCap(), (uint_t)100);
    maxCap = maxCap / 100;
    std::vector<bool> canFired(maxCap + 1, false), alreadyIdle(maxCap + 1, false);
    int_vector_t employStarts(maxCap + 1, 0), minEmployEnds(maxCap + 1, 0);
    int_vector_t idleStarts(maxCap + 1, 0), maxIdleEnds(maxCap + 1, 0);
    CapSpan* lastCapSpan = nullptr;
    int reqCap, lastReqCap = 0;
    uint_t existingCap = dres->existingCap();
    existingCap = (uint_t)ceil((double)existingCap / 100.0);

    // build cslist from resource's timetable
    auto& tt = res.timetable();
    auto tail = tt.tail();
    CapSpan* capSpan;
    const IntSpan* ts;
    t = scheduleSpan.begin();

    // while t is within the schedule's makespan
    while (t < scheduleSpan.end())
    {
        // ts = timetable's IntSpan that contains t
        // tsSpan = ts converted to Span<int>
        ts = tt.find(t);
        Span<int> tsSpan = ts->span();

        // overlap is normally [t, tsSpan.end)
        Span<int> overlap(t, min(tsSpan.end(), scheduleSpan.end()));
        ASSERTD(overlap.size() != 0);

        // convert reqCap
        reqCap = ts->v0();
        double tmp = (double)reqCap / 100.0;
        reqCap = (int)ceil(tmp);

        // step 1: take existing-cap into account
        if (cslist.empty() && (existingCap > 0) && ((minEmploySlots > 0) || (maxIdleSlots > 0)))
        {
            if (minEmploySlots > 0)
            {
                for (int cap = 0; cap != (int)existingCap; ++cap)
                {
                    canFired[cap] = false;
                    employStarts[cap] = t;
                    minEmployEnds[cap] = res.getEndTimeForStartTime(t, minEmploySlots);
                    minEmployEnds[cap] = res.getNonBreakTimeNext(minEmployEnds[cap]);
                }
            }
            lastReqCap = existingCap;
        }

        // step 2: dealing with min-employment and max-idle-time
        // required capacity increased?
        int payCap = reqCap;
        if (reqCap > lastReqCap)
        {
            // for capacities that were already utilized:
            // .. alreadyIdle = false
            int capIdx;
            for (capIdx = 0; capIdx != lastReqCap; ++capIdx)
            {
                alreadyIdle[capIdx] = false;
            }
            // for newly utilized capacities:
            // .. canFired = false
            // .. alreadyIdle = false
            // .. employStarts = t
            // .. minEmployEnds = earliest time when employment can end (based on minEmploySlots)
            ASSERTD(capIdx == lastReqCap);
            ASSERTD(t == overlap.begin());
            for (; capIdx != reqCap; ++capIdx)
            {
                canFired[capIdx] = false;
                alreadyIdle[capIdx] = false;
                employStarts[capIdx] = t;
                minEmployEnds[capIdx] = res.getEndTimeForStartTime(t, minEmploySlots);
                minEmployEnds[capIdx] = res.getNonBreakTimeNext(minEmployEnds[capIdx]);
            }

            // advance beyond overlap
            t = overlap.end();
        }
        else // required capacity decreased (or unchanged)
        {
            ASSERTD(reqCap <= lastReqCap);
            ASSERTD(t == overlap.begin());
            int spanEnd = overlap.end();
            // for newly disutilized capacities (lowest -> highest)
            for (int capIdx = reqCap; capIdx != lastReqCap; ++capIdx)
            {
                // newPayCap = payCap initially
                //             possibly adjusted to (payCap + 1) below
                int newPayCap = payCap;

                // min-employment: possibly pay for this unused capacity
                if (minEmploySlots > 0)
                {
                    if (!canFired[capIdx])
                    {
                        if (minEmployEnds[capIdx] > t)
                        {
                            newPayCap++;
                            spanEnd = min(spanEnd, minEmployEnds[capIdx]);
                        }
                        else
                        {
                            canFired[capIdx] = true;
                        }
                    }
                }

                // max-idle: pay for up to maxIdleSlots of disused capacity
                if (maxIdleSlots > 0)
                {
                    // not already idle at this capacity
                    // .. -> note beginning of idleness
                    // .. -> note end of max-idle period
                    if (!alreadyIdle[capIdx])
                    {
                        idleStarts[capIdx] = tsSpan.begin();
                        maxIdleEnds[capIdx] = res.getEndTimeForStartTime(idleStarts[capIdx],
                                                                         maxIdleSlots);
                        maxIdleEnds[capIdx] = res.getNonBreakTimeNext(maxIdleEnds[capIdx]);
                        alreadyIdle[capIdx] = true;
                    }

                    // min-employment doesn't already cause us to pay for this unused capacity?
                    if (newPayCap == payCap)
                    {
                        // while Timetable's next IntSpan begins at/after max-idle-end
                        const IntSpan* nextTs = ts->next();
                        while ((nextTs != tail) && (nextTs->span().begin() <= maxIdleEnds[capIdx]))
                        {
                            // nextTs's capacity > current paid capacity?
                            int nextTsCap = (int)ceil((double)nextTs->v0() / 100.0);
                            if (nextTsCap > payCap)
                            {
                                newPayCap++;
                                break;
                            }
                            nextTs = nextTs->next();
                        }
                    }
                }

                // adjust payCap
                payCap = newPayCap;
            }

            // advance to spanEnd
            t = spanEnd;
        }

        // merge with previous capSpan?
        if ((lastCapSpan != nullptr) &&
            ((payCap == lastReqCap) ||
             ((res.getNonBreakTime((int)overlap.begin(), (int)t - 1) == 0) && (payCap == 0))))
        {
            capSpan = lastCapSpan;
            capSpan->end = t;
        }
        else // not merging -> create new CapSpan
        {
            capSpan = new CapSpan(payCap, overlap.begin(), t);
            cslist.push_back(capSpan);
        }
        lastCapSpan = capSpan;     // update lastCapSpan
        lastReqCap = capSpan->cap; // update lastReqCap
    }
#ifdef DEBUG_UNIT
    std::cout << "----- cslistBuild finished: ----" << std::endl;
    cslistDump(cslist);
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
TotalCostEvaluator::cslistDump(const cslist_t& cslist) const
{
    // print CapSpans to stdout
    time_t origin = _schedulerConfig->originTime();
    uint_t timeStep = _schedulerConfig->timeStep();
    for (auto cs : cslist)
    {
        time_t begin = cs->begin * timeStep + origin;
        time_t end = cs->end * timeStep + origin;
        std::cout << "[" << time_str(begin) << "<" << cs->begin << ">"
                  << "," << time_str(end) << "<" << cs->end << ">"
                  << "): " << cs->cap << std::endl;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
TotalCostEvaluator::cslistCost(const SchedulingContext& context,
                               const cls::DiscreteResource& res,
                               const cslist_t& cslist) const
{
#ifdef DEBUG_UNIT
    std::string myName("TotalCostEvaluator::cslistCost(): ");
    std::cout << myName << "res.id = " << res.id() << std::endl;
#endif

    // we can't cost the resource without cost information
    auto resCost = utl::cast<ResourceCost>(res.object());
    if ((resCost->costPerHour() == 0.0) && (resCost->costPerDay() == 0.0) &&
        (resCost->costPerWeek() == 0.0) && (resCost->costPerMonth() == 0.0))
    {
        return;
    }

    // timeStep = seconds per time-slot
    // tsPerDay = time-slots per day
    uint_t timeStep = _schedulerConfig->timeStep();
    int tsPerDay = daySec / timeStep;
    ASSERTD((tsPerDay * timeStep) == daySec);

    // makespan: [originTS, makespanTS)
    Span<int> makespan(_originTS, min(_horizonTS, context.makespanTimeSlot()));

    // possibly extend makespan for min-employment
    if (!cslist.empty())
    {
        auto lastSpan = cslist.back();
        if (lastSpan->end > makespan.end())
        {
#ifdef DEBUG_UNIT
            std::cout << myName << "lastSpan->end() > makespan.end() | " << lastSpan->end() << " > "
                << makespan.getEnd() << std::endl;
#endif
            makespan.setEnd(lastSpan->end);
        }
    }

    // numDays = number of days in the schedule
    makespan.setEnd(roundUp(makespan.end(), tsPerDay));
    uint_t numDays = makespan.size() / tsPerDay;

    // ccrs = per-capacity CapCostRecs
    // numCaps = number of unique required capacities
    //         = ccrs.size()
    //         = _caps[] size
    //         >= 1 (we always have capacity 0)
    ccr_map_t ccrs;
    uint_t numCaps = 0;
    utl::arrayGrow(_caps, _capsSize, 16);
    _caps[numCaps++] = 0;
    auto ccr = new CapCostRec(0);
    ccrs.insert(ccr_map_t::value_type(0, ccr));

    // iterate over CapSpans
    uint_t maxCap = 0;
    for (auto capSpan : cslist)
    {
        // cap = this capSpan's capacity
        uint_t cap = capSpan->cap;

        // update maxCap
        maxCap = max(maxCap, cap);

        // find the CapCostRec for cap
        auto it = ccrs.find(cap);

        // no CapCostRec found?
        if (it == ccrs.end())
        {
            // ccr = newly added CapCostRec for cap
            utl::arrayGrow(_caps, _capsSize, numCaps + 1);
            _caps[numCaps++] = cap;
            ccr = new CapCostRec(cap);
            ccr->startTime = roundDown(capSpan->begin, tsPerDay);
            ccrs.insert(ccr_map_t::value_type(cap, ccr));
        }
        else
        {
            // ccr = pre-existing CapCostRec for cap
            ccr = (*it).second;
        }

        // CapCostRec's end time = capSpan->end rounded up to day's end
        ccr->endTime = roundUp(capSpan->end, tsPerDay);

        // auditing -> add ResourceUsageInfo for this CapSpan to _auditReport
        if (_audit)
        {
            uint_t timeStep = _schedulerConfig->timeStep();
            time_t origin = _schedulerConfig->originTime();
            auto usage = new ResourceUsageInfo();
            usage->id = res.id();
            usage->cap = cap;
            usage->begin = origin + capSpan->begin * timeStep;
            usage->end = origin + capSpan->end * timeStep;
            _auditReport->resourceUsages()->push_back(usage);
        }
    }

    // no CapCostRec for existingCap -> create one
    auto dataSet = context.clevorDataSet();
    auto dres = utl::cast<DiscreteResource>(dataSet->findResource(res.id()));
    uint_t existingCap = (uint_t)ceil((double)dres->existingCap() / 100.0);
    if ((ccrs.find(existingCap) == ccrs.end()) && (existingCap < maxCap))
    {
        utl::arrayGrow(_caps, _capsSize, numCaps + 1);
        _caps[numCaps++] = existingCap;
        ccr = new CapCostRec(existingCap);
        ccrs.insert(ccr_map_t::value_type(existingCap, ccr));
    }

    // index the required capacities
    // .. sort _caps[]
    // .. set capIdx for each CapCostRec to its location in _caps[]
    std::sort(_caps, _caps + numCaps);
    for (uint_t capIdx = 0; capIdx != numCaps; ++capIdx)
    {
        uint_t cap = _caps[capIdx];
        auto it = ccrs.find(cap);
        ASSERTD(it != ccrs.end());
        auto ccr = (*it).second;
        ccr->capIdx = capIdx;
    }
    ASSERTD(_caps[0] == 0);

    // CapCostRecs must eclipse higher capacity CapCostRecs
    int minStart = int_t_max, maxEnd = int_t_min;
    for (uint_t capIdx = numCaps - 1; capIdx != 0; --capIdx)
    {
        uint_t cap = _caps[capIdx];
        auto it = ccrs.find(cap);
        ASSERTD(it != ccrs.end());
        auto ccr = (*it).second;
        minStart = utl::min(minStart, ccr->startTime);
        maxEnd = utl::max(maxEnd, ccr->endTime);
        ccr->startTime = minStart;
        ccr->endTime = maxEnd;
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

    // lastCap = existingCap
    // lastCapIdx = index of existingCap (or next highest recorded capacity) within _caps[]
    uint_t lastCap = existingCap;
    uint_t lastCapIdx;
    {
        uint_t capTmp = existingCap;
        while (ccrs.find(capTmp) == ccrs.end())
        {
            capTmp--;
        }
        lastCapIdx = ccrs.find(capTmp)->second->capIdx;
    }

    // count working time for each day & capacity
    for (auto capSpan : cslist)
    {
        // [begin,end) = capSpan rounded to start/end of day
        uint_t cap = capSpan->cap;
        int begin = roundDown(capSpan->begin, tsPerDay);
        int end = roundUp(capSpan->end, tsPerDay);

        // ignore if the entire span is a break
        uint_t nonBreak = res.getNonBreakTime(capSpan->begin, capSpan->end - 1);
        if (nonBreak == 0)
        {
            continue;
        }
        uint_t day = (begin / tsPerDay);

        auto it = ccrs.find(cap);
        ASSERTD(it != ccrs.end());
        auto ccr = (*it).second;
        uint_t capIdx = ccr->capIdx;

        // hiring?
        if (cap > lastCap)
        {
            for (uint_t ci = lastCapIdx + 1; ci <= capIdx; ++ci)
            {
                int offset = (ci * numDays) + day;
                ++_capDayHires[offset];
            }
        }
        else if (cap < lastCap) // firing
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

        // for each day capSpan overlaps
        for (; begin < end; begin += tsPerDay, ++day)
        {
            // workingTime = non-break time within day
            int eod = begin + tsPerDay;
            int b = utl::max(capSpan->begin, begin);
            int e = utl::min(capSpan->end, eod);
            uint_t workingTime = res.getNonBreakTime(b, e - 1);

            // no working time during day?
            if (workingTime == 0)
            {
                continue;
            }

            // day has non-break time...
            _dayIsBreak[day] = 0;

            // add workingTime to _capDayTimes for all capacities up to cap
            for (uint_t ci = 1; ci <= capIdx; ++ci)
            {
                uint_t& wt = _capDayTimes[(ci * numDays) + day];
                wt += workingTime;
            }
        }
    }

    // grow dayCosts[], dayCostPeriods[] arrays
    utl::arrayGrow(_dayCosts, _dayCostsSize, numDays);
    utl::arrayGrow(_dayCostPeriods, _dayCostPeriodsSize, numDays);

    // cost each capacity
    double saveTotalCost = _totalCost;
    lastCap = 0;
    for (uint_t capIdx = 1; capIdx != numCaps; ++capIdx)
    {
        // find cap, capDiff
        uint_t cap = _caps[capIdx];
        uint_t capDiff = (cap - lastCap);
        lastCap = cap;

        // auditing -> print heading
        if (_audit)
        {
            char buf[128];
            sprintf(buf, "%u", cap);
            std::string capStr(buf);
            std::string title = "Capacity " + capStr;
            *_os << heading(title, '.', 65) << std::endl;
        }

        // prepare for costing loop
        auto it = ccrs.find(cap);
        ASSERTD(it != ccrs.end());
        auto ccr = (*it).second;
        uint_t ccrCapIdx = ccr->capIdx;
        uint_t capDayBegin = (ccr->startTime / tsPerDay);
        uint_t numCapDays = (ccr->endTime - ccr->startTime) / tsPerDay;
        ASSERTD(numCapDays != 0);
        uint_t capDayEnd = capDayBegin + numCapDays;
        uint_t* ht = _capDayHires + (ccrCapIdx * numDays);
        uint_t* wt = _capDayTimes + (ccrCapIdx * numDays);
        byte_t* fbht = _capDayFireBeforeHire + (ccrCapIdx * numDays);

        // auditing -> print hours worked on each day
        //          -> add ResourceWorkHoursInfo to _auditReport
        if (_audit)
        {
            auto& report = _auditReport->resourceCost(res.id());
            uint_t curTs = capDayBegin * tsPerDay;
            *_os << "Hours worked on each day: ";
            for (uint_t day = capDayBegin; day < capDayEnd; ++day)
            {
                double hoursWorked = wt[day] * timeStep / 3600.0;
                *_os << day << ":" << hoursWorked << " ";
                time_t originTime = _schedulerConfig->originTime();
                time_t curTime = originTime + (curTs * timeStep);
                auto newInfo = new ResourceWorkHoursInfo();
                newInfo->cap = cap;
                newInfo->date = curTime;
                newInfo->minutes = wt[day] * capDiff * timeStep / 60;
                report.workHours()->push_back(newInfo);
                curTs += tsPerDay;
            }
            *_os << std::endl;
        }

        // note costs (each is amplified by capDiff)
        double hiringCost = resCost->costPerUnitHired();
        double hourCost = resCost->costPerHour();
        double dayCost = resCost->costPerDay();
        double weekCost = resCost->costPerWeek();
        double monthCost = resCost->costPerMonth();
        if (hiringCost != 0.0)
            hiringCost *= (double)capDiff;
        if (hourCost != 0.0)
            hourCost *= (double)capDiff;
        if (dayCost != 0.0)
            dayCost *= (double)capDiff;
        if (weekCost != 0.0)
            weekCost *= (double)capDiff;
        if (monthCost != 0.0)
            monthCost *= (double)capDiff;

        // consider hourly and daily rate for each day
        uint_t day;
        for (day = capDayBegin; day != capDayEnd; ++day)
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
            if (hourCost != 0.0)
            {
                // round workingHours to the hour.
                uint_t workingHours = roundUp((workingTime * timeStep), (uint_t)3600) / 3600;
                double cost = workingHours * hourCost;
                // for hourly costs, all hiring has to be accounted for.
                cost += ht[day] * hiringCost;
                _dayCosts[day] = cost;
                _dayCostPeriods[day] = period_hour;
            }

            // daily cost?
            if (dayCost != 0.0)
            {
                double cost = dayCost;
                // only account for hiring cost if it is done before any firing had occured
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

        // dayCosts = copy of _dayCosts
        double* dayCosts = new double[numDays];
        for (uint_t i = 0; i < numDays; i++)
        {
            dayCosts[i] = _dayCosts[i];
        }

        // consider use of weekly rate
        if (weekCost != 0.0)
        {
            // we want to build all the possible weekly rates day by day
            uint_t beginDay = capDayBegin;
            uint_t beginDayPlus7 = beginDay + 7;
            uint_t endDay = beginDay;

            while (beginDay < capDayEnd)
            {
                if ((!_dayIsBreak[beginDay]) && (wt[beginDay] != 0))
                {
                    // determine the cost of the next 7 or less days
                    double next7DaysCost = 0.0;
                    uint_t lim = utl::min(beginDayPlus7, capDayEnd);
                    for (endDay = beginDay; endDay < lim; ++endDay)
                    {
                        next7DaysCost += _dayCosts[endDay];
                    }
                    // determine the cost for using weekly cost
                    double cost = weekCost;
                    if ((!fbht[beginDay]) && (ht[beginDay] > 0))
                    {
                        cost += hiringCost;
                    }
                    // compare the costs
                    if (((cost < next7DaysCost) || (_dayCostPeriods[beginDay] == period_undefined)))
                    {
                        _dayCosts[beginDay] = cost;
                        _dayCostPeriods[beginDay] = period_week;
                    }
                }
                // tomorrow is another day
                beginDay++;
                beginDayPlus7 = beginDay + 7;
            }
        }

        // consider use of monthly rate
        if (monthCost != 0.0)
        {
            uint_t beginDay = capDayBegin;
            // find next beginDay
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

                // account for any hiring costs
                if ((!fbht[beginDay]) && (ht[beginDay] > 0))
                {
                    totalMonthCost += hiringCost;
                    cost += hiringCost;
                }

                // compare the costs
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
            auto it = _ipSpans.findFirstIt(Span<int>(t, t + 1));
            ASSERTD(it != _ipSpans.end());
            auto sip = utl::cast<SpanInterestPeriod>(*it);
            uint_t ip = sip->period;

            if (_audit)
            {
                time_t originTime = _schedulerConfig->originTime();
                time_t curTime = originTime + (t * timeStep);
                *_os << "Day: " << day << ": " << time_str(curTime) << std::endl;

                auto& report = _auditReport->resourceCost(res.id());
                auto newCost = new ResourceCostInfo();
                newCost->cap = cap;
                newCost->date = curTime;
                newCost->hiredCap = 0;
                newCost->hireCost = 0;
                newCost->bestRate = period_t(period_undefined);
                report.costs()->push_back(newCost);
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
                    *_os << "    i.p. " << ip << ": $" << _ipCosts[ip];

                    auto costInfo = _auditReport->resourceCost(res.id()).costs()->back();
                    costInfo->hiredCap = capHired;
                    costInfo->hireCost = hireCost;
                }
                _ipCosts[ip] += hireCost;
                _totalCost += hireCost;
                if (_audit)
                {
                    *_os << " + $" << hireCost << " = $" << _ipCosts[ip] << std::endl;
                }
            }

            // auditing -> print the chosen rate
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
                auto costInfo = _auditReport->resourceCost(res.id()).costs()->back();
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
                // no need for rounding since everything is on day boundary
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
                    *_os << "    i.p. " << ip << ": $" << _ipCosts[ip];
                }
                _ipCosts[ip] += rateCost;
                _totalCost += rateCost;
                if (_audit)
                {
                    *_os << " + $" << rateCost << " = $" << _ipCosts[ip] << std::endl;
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

    // assess resource cost as the addition to _totalCost that happened here
    double totalResCost = _totalCost - saveTotalCost;
    resCost->cost() = (uint_t)totalResCost;

    // clean up CapCostRecs
    deleteMapSecond(ccrs);

#ifdef DEBUG_UNIT
    std::cout << myName << "----- finished -----" << std::endl;
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
TotalCostEvaluator::calcLatenessCost(const SchedulingContext& context) const
{
    // auditing -> print header
    if (_audit)
    {
        *_os << heading("calcOpportunity/Inventory/LatenessCost()", '-', 75) << std::endl;
    }

    double oppCost = 0.0;
    double invCost = 0.0;
    double latCost = 0.0;
    double saveTotalCost = _totalCost;

    // for each Job
    auto& jobs = context.clevorDataSet()->jobs();
    for (auto job : jobs)
    {
        // job has no due time or is inactive -> skip it
        if (job->dueTime() == -1 || !job->active())
        {
            continue;
        }

        // note job's due time and completion time
        int dueTime = context.timeToTimeSlot(job->dueTime());
        int makespan = job->makespan();

        // DEBUG build: warn about discrepancy between job's makespan and latest op end time
#ifdef DEBUG
        // check the makespan
        int maxEndTime = -1;
        for (auto op : *job)
        {
            auto act = op->activity();
            if ((op->type() == op_summary) || op->ignorable() || (act == nullptr))
            {
                continue;
            }

            int endTime = _schedulerConfig->forward() ? act->ef() + 1 : act->lf() + 1;
            if (endTime > maxEndTime)
            {
                maxEndTime = endTime;
            }
        }

        // job has schedulable ops and makespan doesn't match latest op end time?
        if ((maxEndTime != -1) && (makespan != maxEndTime))
        {
            // print warning
            std::cout << "end times differ for job " << job->id() << " makespan " << makespan
                      << " jobOp end time " << maxEndTime << std::endl;
        }
#endif

        // job ends exactly at due time -> no cost adjustment
        if (dueTime == makespan)
        {
            continue;
        }

        // opportunityCostPerTS = opportunity cost per time slot
        double opportunityCostPerTS = 0.0;
        if (job->opportunityCostPeriod() != period_undefined)
        {
            uint_t periodSeconds;
            periodSeconds = periodToSeconds(job->opportunityCostPeriod());
            opportunityCostPerTS =
                job->opportunityCost() / ((double)periodSeconds / (double)_timeStep);
        }

        // latenessCostPerTS = lateness cost per time slot
        double latenessCostPerTS = 0.0;
        double latenessIncrement = 0.0;
        double latenessPeriodSeconds = 0.0;
        if (job->latenessCostPeriod() != period_undefined)
        {
            latenessPeriodSeconds = (double)periodToSeconds(job->latenessCostPeriod());
            latenessCostPerTS = job->latenessCost() / (latenessPeriodSeconds / (double)_timeStep);
            latenessIncrement = job->latenessIncrement() / 100;
            latenessIncrement += 1.0;
        }

        // inventoryCostPerTS
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
                auto& report = _auditReport->latenessCost(job->id());
                if (report.getName().empty())
                {
                    report.setName(job->name());
                }
                for (auto& ipCost : _auditIpCosts)
                {
                    auto info = new LatenessCostInfo();
                    info->interestPeriod = ipCost.first;
                    info->opportunityCost = ipCost.second;
                    info->latenessCost = 0;
                    info->inventoryCost = 0;
                    report.costs()->push_back(info);
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
                auto& report = _auditReport->latenessCost(job->id());
                if (report.getName().empty())
                {
                    report.setName(job->name());
                }
                for (auto& ipCost : _auditIpCosts)
                {
                    auto info = new LatenessCostInfo();
                    info->interestPeriod = ipCost.first;
                    info->opportunityCost = 0;
                    info->latenessCost = 0;
                    info->inventoryCost = ipCost.second;
                    report.costs()->push_back(info);
                }
            }
            invCost += _totalCost - saveTotalCost;
            saveTotalCost = _totalCost;
        }
        else if (makespan > dueTime) // lateness cost
        {
            if (_audit)
            {
                *_os << "Lateness Cost" << std::endl;
            }
            calcPeriodCost(Span<int>(dueTime, makespan), latenessCostPerTS, latenessIncrement,
                           latenessPeriodSeconds, (double)_timeStep);
            if (_audit)
            {
                auto& report = _auditReport->latenessCost(job->id());
                if (report.getName().empty())
                {
                    report.setName(job->name());
                }
                for (auto& ipCost : _auditIpCosts)
                {
                    auto info = new LatenessCostInfo();
                    info->interestPeriod = ipCost.first;
                    info->opportunityCost = 0;
                    info->latenessCost = ipCost.second;
                    info->inventoryCost = 0;
                    report.costs()->push_back(info);
                }
            }
            latCost += _totalCost - saveTotalCost;
            saveTotalCost = _totalCost;
        }
    }

    setComponentScore("OpportunityCost", (int)oppCost);
    setComponentScore("InventoryCost", (int)invCost);
    setComponentScore("LatenessCost", (int)latCost);
    if (_audit)
    {
        auto info1 = new ComponentScoreInfo();
        info1->name = "OpportunityCost";
        info1->score = oppCost;
        _auditReport->componentInfos()->push_back(info1);
        auto info2 = new ComponentScoreInfo();
        info2->name = "InventoryCost";
        info2->score = invCost;
        _auditReport->componentInfos()->push_back(info2);
        auto info3 = new ComponentScoreInfo();
        info3->name = "LatenessCost";
        info3->score = latCost;
        _auditReport->componentInfos()->push_back(info3);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
TotalCostEvaluator::calcJobOverheadCost(const SchedulingContext& context) const
{
    // auditing -> print header
    if (_audit)
    {
        *_os << heading("calcJobOverheadCost()", '-', 75) << std::endl;
        _auditIpCosts.clear();
    }

    double saveTotalCost = _totalCost;
    double jobOverheadCost = 0.0;

    // iterate over jobs
    for (auto job : context.clevorDataSet()->jobs())
    {
        // skip a job that has no due time or is not active
        if ((job->dueTime() == -1) || !job->active())
        {
            continue;
        }

        // makespan = job's makespan
        // minStartTime = earliest start time of job's ops
        int makespan = job->makespan();
        int minStartTime = int_t_max;
        for (auto op : *job)
        {
            // skip summary, ignorable, and activity-less op
            auto act = op->activity();
            if ((op->type() == op_summary) || op->ignorable() || (act == nullptr))
            {
                continue;
            }

            // update minStartTime
            minStartTime = min(minStartTime, act->es() + 1);
        }

        // overheadCostPerTS = overhead cost per time slot
        double overheadCostPerTS = 0.0;
        if (job->overheadCostPeriod() != period_undefined)
        {
            auto periodSeconds = periodToSeconds(job->overheadCostPeriod());
            overheadCostPerTS = job->overheadCost() / ((double)periodSeconds / (double)_timeStep);
        }

        // auditing -> print subheading for this Job
        if (_audit)
        {
            *_os << "WorkOrder id = " << job->id() << " name = " << job->name() << ": "
                << "Job Overhead Cost" << std::endl;
        }

        // calculate interest period costs for this job
        calcPeriodCost(Span<int>(minStartTime, makespan), overheadCostPerTS);

        // auditing -> record job's overhead cost in its JobOverheadCostReport
        //             (with a JobOverheadCostInfo for each interest period)
        if (_audit)
        {
            auto& report = _auditReport->joboverheadCost(job->id());
            if (report.getName().empty())
            {
                report.setName(job->name());
            }
            for (auto& ipCost : _auditIpCosts)
            {
                auto info = new JobOverheadCostInfo();
                info->interestPeriod = ipCost.first;
                info->cost = ipCost.second;
                report.costs()->push_back(info);
            }
        }

        // add this job's overhead cost to the total
        jobOverheadCost += _totalCost - saveTotalCost;

        // so we can calculate the next job's overhead cost
        saveTotalCost = _totalCost;
    }
    setComponentScore("JobOverheadCost", (int)jobOverheadCost);
    if (_audit)
    {
        *_os << "Total Job Overhead Cost: $" << jobOverheadCost << std::endl;
        auto info = new ComponentScoreInfo();
        info->name = "JobOverheadCost";
        info->score = jobOverheadCost;
        _auditReport->componentInfos()->push_back(info);
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
    auto& ops = context.clevorDataSet()->ops();
    for (auto op : ops)
    {
        // op must be active, non-summary, unstarted
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
        auto act = op->activity();
        if (act == nullptr)
        {
            continue;
        }

        // sip = SpanInterestPeriod that contains op's start time
        //  ip = sip's index with _ipCosts[]
        Span<int> searchSpan(act->es(), act->es() + 1);
        auto it = _ipSpans.findFirstIt(searchSpan);
        ASSERTD(it != _ipSpans.end());
        auto sip = utl::cast<SpanInterestPeriod>(*it);
        uint_t ip = sip->period;

        // auditing -> print subheading
        if (_audit)
        {
            *_os << "Fixed Cost (op id = " << op->id() << "): "
                 << "$" << opCost << std::endl;
            *_os << "    i.p. " << ip << ": $" << _ipCosts[ip];
            auto& info = (*_auditReport->fixedCosts())[op->id()];
            info = new FixedCostInfo();
            info->opId = op->id();
            info->interestPeriod = ip;
            info->cost = opCost;
        }

        // add the fixed cost
        _ipCosts[ip] += opCost;
        _totalCost += opCost;

        // auditing -> print added and total cost in this interest period
        if (_audit)
        {
            *_os << " + $" << opCost << " = $" << _ipCosts[ip] << std::endl;
        }
    }

    // assess total fixed cost as the addition to _totalCost that happened here
    double totalFixedCost = _totalCost - saveTotalCost;
    setComponentScore("FixedCost", (int)totalFixedCost);

    // auditing -> add ComponentScoreInfo to _auditReport for fixed cost
    if (_audit)
    {
        auto info = new ComponentScoreInfo();
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
    auto dataSet = context.clevorDataSet();
    auto& resources = dataSet->resources();
    for (auto res : resources)
    {
        // skip non-DiscreteResource
        if (!res->isA(DiscreteResource))
        {
            continue;
        }

        auto dres = utl::cast<DiscreteResource>(res);

        // for each of dres's ResourceSequenceRuleApplications
        auto& rsras = dres->sequenceRuleApplications();
        for (auto& app : rsras)
        {
            auto rule = app.rule();

            // note the cost (it can't be 0)
            double cost = rule->cost();
            if (cost == 0.0)
            {
                continue;
            }

            // note the lhs and rhs activities (these must both exist)
            auto lhsAct = app.lhsOp()->activity();
            auto rhsAct = app.rhsOp()->activity();
            if ((lhsAct == nullptr) || (rhsAct == nullptr))
            {
                continue;
            }

            // sip = SpanInterestPeriod that contains lhs activity's finish time
            //  ip = sip's index with _ipCosts[]
            Span<int> searchSpan(lhsAct->ef(), lhsAct->ef() + 1);
            auto it = _ipSpans.findFirstIt(searchSpan);
            ASSERTD(it != _ipSpans.end());
            auto sip = utl::cast<SpanInterestPeriod>(*it);
            uint_t ip = sip->period;

            // auditing -> print subheading
            if (_audit)
            {
                *_os << "Resource Sequence Cost "
                     << "(res-id = " << dres->id() << ", lhs-op-id = " << lhsAct->id()
                     << ", rhs-op-id = " << rhsAct->id() << "): "
                     << "$" << cost << std::endl;
                *_os << "    i.p. " << ip << ": $" << _ipCosts[ip];
                auto info = new ResourceSequenceCostInfo();
                info->id = dres->id();
                info->lhsId = lhsAct->id();
                info->rhsId = rhsAct->id();
                info->cost = cost;
                info->ip = ip;
                _auditReport->resourceSequenceCosts()->push_back(info);
            }

            // add the cost
            _ipCosts[ip] += cost;
            _totalCost += cost;

            // auditing -> print added and total cost for this interest period
            if (_audit)
            {
                *_os << " + $" << cost << " = $" << _ipCosts[ip] << std::endl;
            }
        }
    }

    // assess resource sequence cost as the addition to _totalCost that happened here
    double totalResourceSequenceCost = _totalCost - saveTotalCost;
    setComponentScore("ResourceSequenceCost", (int)totalResourceSequenceCost);

    // auditing -> add ComponentScoreInfo to _auditReport for resource sequence cost
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
    // auditing -> print header
    if (_audit)
    {
        *_os << heading("calcOverheadCost()", '-', 75) << std::endl;
    }

    // makespan = schedule's makespan with end time rounded up to day's end
    double saveTotalCost = _totalCost;
    Span<int> makespan(_originTS, context.makespanTimeSlot());
    int tsPerDay = daySec / (uint_t)_schedulerConfig->timeStep();
    makespan.setEnd(roundUp(makespan.end(), tsPerDay));

    // auditing -> print subheading
    if (_audit)
    {
        *_os << "Overhead Cost" << std::endl;
    }

    // assess _overheadCostPerTS during makespan
    calcPeriodCost(makespan, _overheadCostPerTS);

    // assess overhead cost as the addition to _totalCost that happened here
    double totalOverheadCost = _totalCost - saveTotalCost;
    setComponentScore("OverheadCost", (int)totalOverheadCost);

    // auditing -> add OverHeadCostInfo to _auditReport
    //          -> add ComponentScoreInfo for overhead cost
    if (_audit)
    {
        std::map<uint_t, double>::const_iterator i;
        for (i = _auditIpCosts.begin(); i != _auditIpCosts.end(); i++)
        {
            auto info = new OverheadCostInfo();
            info->interestPeriod = i->first;
            info->cost = i->second;
            _auditReport->overheadCosts()->push_back(info);
        }
        auto info = new ComponentScoreInfo();
        info->name = "OverheadCost";
        info->score = totalOverheadCost;
        _auditReport->componentInfos()->push_back(info);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
TotalCostEvaluator::calcInterestCost(const SchedulingContext& context) const
{
    // auditing -> print header
    if (_audit)
    {
        *_os << heading("calcInterestCost()", '-', 75) << std::endl;
    }

    int makeSpanEnd = context.makespanTimeSlot();
    double saveTotalCost = _totalCost;
    double totalCost = 0.0;

    // for each SpanInterestPeriod
    for (auto sip_ : _ipSpans)
    {
        auto sip = utl::cast<SpanInterestPeriod>(sip_);
        uint_t ip = sip->period;
        double interestCost = 0.0;
        if (sip->begin() <= makeSpanEnd)
        {
            interestCost = totalCost * _interestRate;
        }

        // auditing -> print current interest period cost
        if (_audit)
        {
            *_os << "    i.p. " << ip << ": $" << _ipCosts[ip];
        }

        _ipCosts[ip] += interestCost;
        _totalCost += interestCost;
        totalCost += _ipCosts[ip];

        // auditing -> print added and total cost for this interest period
        if (_audit)
        {
            *_os << " + $" << interestCost << " = $" << _ipCosts[ip] << std::endl;
            auto info = new InterestCostInfo();
            info->interestPeriod = ip;
            info->cost = interestCost;
            _auditReport->interestCosts()->push_back(info);
        }
    }

    // assess interest cost as the addition to _totalCost that happened here
    double totalInterestCost = _totalCost - saveTotalCost;
    setComponentScore("InterestCost", (int)totalInterestCost);

    // auditing -> add ComponentScoreInfo for interest cost
    if (_audit)
    {
        auto info = new ComponentScoreInfo();
        info->name = "InterestCost";
        info->score = totalInterestCost;
        _auditReport->componentInfos()->push_back(info);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
TotalCostEvaluator::calcPeriodCost(const utl::Span<int>& p_span, double costPerTS) const
{
    // auditing -> clear interest period costs in audit
    if (_audit)
    {
        _auditIpCosts.clear();
    }

    // no cost to be assessed -> done
    if (costPerTS == 0.0)
    {
        return;
    }

    auto span = p_span;
    double saveTotalCost = _totalCost;

    // auditing -> print subheading
    if (_audit)
    {
        uint_t timeStep = _schedulerConfig->timeStep();
        time_t originTime = _schedulerConfig->originTime();
        time_t beginTime = originTime + (span.begin() * timeStep);
        time_t endTime = originTime + (span.end() * timeStep);
        *_os << " => calcPeriodCost(" << time_str(beginTime) << ", " << time_str(endTime) << ", "
            << "$" << costPerTS << ")" << std::endl;
    }

    // iterate through SpanInterestPeriods that overlap with span
    auto spanIPsEnd = _ipSpans.end();
    for (auto it = _ipSpans.findFirstIt(span); (it != spanIPsEnd) && !span.isNil(); ++it)
    {
        auto& sip = utl::cast<SpanInterestPeriod>(**it);
        auto overlap = sip.overlap(span);
        span.setBegin(overlap.end());
        uint_t ip = sip.period;
        double cost = (costPerTS * (double)overlap.size());

        // auditing -> print current interest period cost
        if (_audit)
        {
            *_os << "    i.p. " << ip << ": $" << _ipCosts[ip];
        }

        // add the cost for this interest period
        _ipCosts[ip] += cost;
        _totalCost += cost;

        // auditing -> print added and total cost for this interest period
        if (_audit)
        {
            *_os << " + $" << cost << " = $" << _ipCosts[ip] << std::endl;
            _auditIpCosts[ip] = cost;
        }
    }

    // auditing -> print total
    if (_audit)
    {
        *_os << "    total: $" << (_totalCost - saveTotalCost) << std::endl;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
TotalCostEvaluator::calcPeriodCost(const utl::Span<int>& p_span,
    double costPerTS,
    double incrCost,
    double periodSeconds,
    double timeStep) const
{
    // auditing -> clear interest period costs in audit
    if (_audit)
    {
        _auditIpCosts.clear();
    }

    // no cost to be assessed -> do nothing
    if (costPerTS == 0.0)
    {
        return;
    }

    auto span = p_span;
    double saveTotalCost = _totalCost;

    // auditing -> print subheading
    if (_audit)
    {
        uint_t timeStep = _schedulerConfig->timeStep();
        time_t originTime = _schedulerConfig->originTime();
        time_t beginTime = originTime + (span.begin() * timeStep);
        time_t endTime = originTime + (span.end() * timeStep);
        *_os << " => calcPeriodCost(" << time_str(beginTime) << ", " << time_str(endTime) << ", "
            << "$" << costPerTS << ", "
            << "%" << incrCost << ", " << periodSeconds << ", " << timeStep << ")" << std::endl;
    }

    // iterate through SpanInterestPeriods that overlap with span
    double periodSteps = (double)(periodSeconds / timeStep);
    for (auto it = _ipSpans.findFirstIt(span); (it != _ipSpans.end()) && !span.isNil(); ++it)
    {
        // overlap = this SpanInterestPeriod's overlap with span
        auto& sip = utl::cast<SpanInterestPeriod>(**it);
        auto overlap = sip.overlap(span);
        span.setBegin(overlap.end());
        uint_t ip = sip.period;
        double cost = (costPerTS * (double)overlap.size());

        // auditing -> print current interest period cost
        if (_audit)
        {
            *_os << "    i.p. " << ip << ": $" << _ipCosts[ip];
        }

        // overlap is at least periodSteps in size AND incrCost > 1.0?
        if ((periodSteps <= (double)overlap.size()) && (incrCost > (double)1.0))
        {
            cost = 0.0;
            double realCostPerTS = costPerTS;

            // for each period of length periodSteps in overlap
            for (double i = periodSteps; i <= (double)overlap.size(); i += periodSteps)
            {
                cost += (realCostPerTS * periodSteps);
                realCostPerTS *= incrCost;

                // handle the final part of overlap which is less than periodSteps in size
                if ((i + periodSteps) > (double)overlap.size())
                {
                    double remaining = (double)overlap.size() - i;
                    cost += (realCostPerTS * remaining);
                }
            }
        }

        // add the cost for this interest period
        _ipCosts[ip] += cost;
        _totalCost += cost;

        // auditing -> print the added and total cost for this interest period
        if (_audit)
        {
            *_os << " + " << cost << " = $" << _ipCosts[ip] << std::endl;
            _auditIpCosts[ip] = cost;
        }
    }

    // auditing -> print total
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
