#include "libcse.h"
#include <libutl/Time.h>
#include <libutl/BufferedFDstream.h>
#include "DiscreteResource.h"

//////////////////////////////////////////////////////////////////////////////

#ifdef DEBUG
//#define DEBUG_UNIT
//static const utl::uint_t debugResId = 55;
#endif

//////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;
CLS_NS_USE;

//////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(cse::ResourceSequenceRuleApplication, utl::Object);
UTL_CLASS_IMPL(cse::DiscreteResource, cse::Resource);

//////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////

void
ResourceSequenceRuleApplication::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(ResourceSequenceRuleApplication));
    const ResourceSequenceRuleApplication& rsra
        = (const ResourceSequenceRuleApplication&)rhs;
    Object::copy(rsra);
    _lhsOp = rsra._lhsOp;
    _rhsOp = rsra._rhsOp;
    _rsr = rsra._rsr;
}

//////////////////////////////////////////////////////////////////////////////

void
DiscreteResource::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(DiscreteResource));
    const DiscreteResource& dr = (const DiscreteResource&)rhs;
    Resource::copy(dr);
    _sequenceId = dr._sequenceId;
    _rsl = nullptr;
    _minCap = dr._minCap;
    _existingCap = dr._existingCap;
    _maxCap = dr._maxCap;
    _stepCap = dr._stepCap;
    delete _cost;
    delete _defaultCalendar;
    delete _detailedCalendar;
    _cost = lut::clone(dr._cost);
    _defaultCalendar = lut::clone(dr._defaultCalendar);
    _detailedCalendar = lut::clone(dr._detailedCalendar);
}

//////////////////////////////////////////////////////////////////////////////

void
DiscreteResource::serialize(Stream& stream, uint_t io, uint_t)
{
    Resource::serialize(stream, io);
    utl::serialize(_sequenceId, stream, io);
    utl::serialize(_minCap, stream, io);
    utl::serialize(_existingCap, stream, io);
    utl::serialize(_maxCap, stream, io);
    utl::serialize(_stepCap, stream, io);
    utl::serializeNullable(_cost, stream, io);
    utl::serializeNullable(_defaultCalendar, stream, io);
    utl::serializeNullable(_detailedCalendar, stream, io);
    if (io == io_rd)
    {
        if (_minCap == uint_t_max)
        {
            _minCap = utl::min(100U, _maxCap);
        }
        if (_existingCap == uint_t_max)
        {
            _existingCap = 0;
        }
        if (_stepCap == uint_t_max)
        {
            _stepCap = 100;
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

ResourceCalendar*
DiscreteResource::makeCurrentCalendar(
    const SchedulerConfiguration* config) const
{
#ifdef DEBUG_UNIT
    if (_id == debugResId)
    {
        cout << "res id: " << _id << endlf;
    }
#endif

    ResourceCalendar* currentCal = new ResourceCalendar();
    if (_defaultCalendar != nullptr)
    {
        applyDefaultCalendar(config, currentCal, _defaultCalendar);
        overrideCalendar(config, currentCal, _defaultCalendar, rcs_exception);
    }
    if (_detailedCalendar != nullptr)
    {
        applyDefaultCalendar(config, currentCal, _detailedCalendar);
        overrideCalendar(config, currentCal, _detailedCalendar, rcs_exception);
    }
    return currentCal;
}

//////////////////////////////////////////////////////////////////////////////

/*ResourceCalendar*
DiscreteResource::makeCurrentCalendar(
    const SchedulerConfiguration* config) const
{
    ResourceCalendar* defaultCal = new ResourceCalendar();

#ifdef DEBUG_UNIT
    if (_id == debugResId)
    {
        cout << "res id: " << _id << endlf;
    }
#endif

    // apply dayOfWeek spans from default calendar
    if (_defaultCalendar != nullptr)
    {
        overrideCalendar(
            nullptr,
            defaultCal,
            _defaultCalendar,
            rcs_dayOfWeek);
    }

#ifdef DEBUG_UNIT
    if (_id == debugResId)
    {
        cout << "dayOfWeek spans (default):" << endlf;
        defaultCal->dump(cout, config->originTime(), config->timeStep());
        cout.flush();
    }
#endif

    // apply dayOfWeek spans from detailed calendar
    if (_detailedCalendar != nullptr)
    {
        overrideCalendar(
            nullptr,
            defaultCal,
            _detailedCalendar,
            rcs_dayOfWeek);
    }

#ifdef DEBUG_UNIT
    if (_id == debugResId)
    {
        cout << "dayOfWeek spans (default+detailed):" << endlf;
        defaultCal->dump(cout, config->originTime(), config->timeStep());
        cout.flush();
    }
#endif

    // apply default
    ResourceCalendar* currentCal = new ResourceCalendar();
    applyDefaultCalendar(config, currentCal, defaultCal);

#ifdef DEBUG_UNIT
    if (_id == debugResId)
    {
        cout << "currentCal with dayOfWeek spans applied" << endl;
        currentCal->dump(cout, config->originTime(), config->timeStep());
        cout.flush();
    }
#endif

    // apply exceptions from default calendar
    if (_defaultCalendar != nullptr)
    {
        overrideCalendar(
            config,
            currentCal,
            _defaultCalendar,
            rcs_exception);
    }

    // apply exceptions from detailed calendar
    if (_detailedCalendar != nullptr)
    {
        overrideCalendar(
            config,
            currentCal,
            _detailedCalendar,
            rcs_exception);
    }

#ifdef DEBUG_UNIT
    if (_id == debugResId)
    {
        cout << "currentCal with exception spans applied" << endl;
        currentCal->dump(cout, config->originTime(), config->timeStep());
        cout.flush();
    }
#endif

    delete defaultCal;
    return currentCal;
}*/

//////////////////////////////////////////////////////////////////////////////

void
DiscreteResource::init()
{
    _sequenceId = uint_t_max;
    _rsl = nullptr;
    _minCap = _existingCap = _maxCap = _stepCap = _selectedCap
        = utl::uint_t_max;
    _cost = nullptr;
    _defaultCalendar = _detailedCalendar = nullptr;
}

//////////////////////////////////////////////////////////////////////////////

void
DiscreteResource::deInit()
{
    delete _cost;
    delete _defaultCalendar;
    delete _detailedCalendar;
}

//////////////////////////////////////////////////////////////////////////////

void
DiscreteResource::applyDefaultCalendar(
    const SchedulerConfiguration* config,
    cls::ResourceCalendar* outSched,
    cls::ResourceCalendar* inSched) const
{
    time_t originTime = config->originTime();
    time_t roundedOriginTime = Time(originTime).roundDown(tm_day);
    time_t horizonTime = config->horizonTime();

    ResourceCalendar::iterator it;
    for (it = inSched->begin(); it != inSched->end(); ++it)
    {
        cls::ResourceCalendarSpan* rss =
            static_cast<cls::ResourceCalendarSpan*>(*it);

        // only apply dayOfWeek spans
        if (rss->type() != rcs_dayOfWeek)
        {
            continue;
        }

        time_t rssBegin = rss->getBegin();
        time_t rssEnd = rss->getEnd();
        uint_t rssCap = rss->capacity();
        if (rssCap == uint_t_max) rssCap = _maxCap;
        ASSERTD(rssCap != uint_t_max);
        for (; rssBegin < rssEnd; rssBegin = time_date(rssBegin) + 86400)
        {
            /*char rssBeginStr[128];
              char rssEndStr[128];
              strcpy(rssBeginStr, ctime(&rssBegin));
              strcpy(rssEndStr, ctime(&rssEnd));*/

            // determine begin, end times of new span
            time_t begin = roundedOriginTime + time_timeOfDay(rssBegin);
            time_t end = begin + (rssEnd - rssBegin);

            uint_t beginDOW = time_dayOfWeek(begin);
            uint_t endDOW = time_dayOfWeek(end);

            // fix end time to (begin + 1d) if is not on same weekday
            if (beginDOW != endDOW)
            {
                end = time_date(begin) + 86400;
            }

            // shift begin,end to the same day-of-week as rssBegin
            while (time_dayOfWeek(begin) != time_dayOfWeek(rssBegin))
            {
                begin += 86400;
                end += 86400;
            }

            /*char beginStr[128];
              char endStr[128];
              strcpy(beginStr, ctime(&begin));
              strcpy(endStr, ctime(&end));*/

            // apply the span to same weekday until horizon
            while (begin < horizonTime)
            {
                cls::ResourceCalendarSpan* newRSS = new
                    cls::ResourceCalendarSpan(
                        config->timeToTimeSlot(max(begin, originTime)),
                        config->timeToTimeSlot(min(end, horizonTime)),
                        rcs_exception,
                        rss->status(),
                        rssCap);
                outSched->add(newRSS);
                begin += 7 * 86400;
                end += 7 * 86400;
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

void
DiscreteResource::overrideCalendar(
    const SchedulerConfiguration* config,
    cls::ResourceCalendar* outSched,
    cls::ResourceCalendar* inSched,
    rcs_t type) const
{
    time_t originTime = config->originTime();
    time_t horizonTime = config->horizonTime();
    ResourceCalendar::iterator it;
    for (it = inSched->begin(); it != inSched->end(); ++it)
    {
        cls::ResourceCalendarSpan* rss =
            static_cast<cls::ResourceCalendarSpan*>(*it);
        uint_t rssCap = rss->capacity();
        if (rssCap == uint_t_max) rssCap = _maxCap;
        ASSERTD(rssCap != uint_t_max);

        if ((type != rcs_undefined) && (rss->type() != type))
        {
            continue;
        }

        if (config == nullptr)
        {
            rss = rss->clone();
            outSched->add(rss);
        }
        else
        {
            time_t begin = rss->getBegin();
            time_t end = rss->getEnd();
            ASSERTD(begin <= end);
            if (end <= originTime || begin >= horizonTime)
                continue;
            begin = max(begin, originTime);
            end = min(end, horizonTime);
            ASSERTD(begin <= end);
            int beginTS = config->timeToTimeSlot(begin);
            int endTS = config->timeToTimeSlot(end);
            if (beginTS < endTS)
            {
                rss = new
                    ResourceCalendarSpan(
                        beginTS,
                        endTS,
                        rss->type(),
                        rss->status(),
                        rssCap);
                outSched->add(rss);
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

CSE_NS_END;
