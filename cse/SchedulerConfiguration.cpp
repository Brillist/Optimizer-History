#include "libcse.h"
#include <libutl/Time.h>
#include <libutl/BufferedFDstream.h>
#include "SchedulerConfiguration.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(cse::SchedulerConfiguration);

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

void
SchedulerConfiguration::copy(const Object& rhs)
{
    ASSERTD(dynamic_cast<const SchedulerConfiguration*>(&rhs) != nullptr);
    const SchedulerConfiguration& cf = (const SchedulerConfiguration&)rhs;
    _originTime = cf._originTime;
    _horizonTime = cf._horizonTime;
    _timeStep = cf._timeStep;
    _schedulingOriginTime = cf._schedulingOriginTime;
    _autoFreezeDuration = cf._autoFreezeDuration;
    _useInitialAsSeed = cf._useInitialAsSeed;
    _backward = cf._backward;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
SchedulerConfiguration::serialize(Stream& stream, uint_t io, uint_t)
{
    lut::serialize(_originTime, stream, io);
    lut::serialize(_horizonTime, stream, io);
    utl::serialize(_timeStep, stream, io);
    lut::serialize(_schedulingOriginTime, stream, io);
    utl::serialize(_autoFreezeDuration, stream, io);
    utl::serialize(_useInitialAsSeed, stream, io);
    utl::serialize(_backward, stream, io);
    if (io == io_rd)
    {
        int remainder = (_horizonTime - _originTime) % _timeStep;
        if (remainder != 0)
        {
            int ts = 1 + (_horizonTime - _originTime) / _timeStep;
#ifdef DEBUG
            utl::cout << "WARNING: The horizonTime has been rounded up ("
                      << Time(_horizonTime).toString() << "->"
                      << Time(timeSlotToTime(ts)).toString()
                      << "). [origin:" << Time(_originTime).toString() << ", step:" << _timeStep
                      << "s]" << utl::endlf;
#endif
            _horizonTime = timeSlotToTime(ts);
        }
        //         _originTime = Time(_originTime).roundDown(tm_day);
        //         _horizonTime = Time(_horizonTime).roundUp(tm_day);
        if (_schedulingOriginTime == -1)
        {
            if (_backward)
            {
                _schedulingOriginTime = _horizonTime;
            }
            else
            {
                _schedulingOriginTime = _originTime;
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

time_t
SchedulerConfiguration::timeSlotToTime(int ts) const
{
    return _originTime + (ts * _timeStep);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int
SchedulerConfiguration::timeToTimeSlot(time_t t) const
{
    // at or before origin => origin
    if (t <= _originTime)
        return 0;

    // at or after horizon => horizon
    t = utl::min(t, _horizonTime);
    //     t = ult::min(t, _horizonTime - 1);

    // note: _originTime < t < _horizonTime
    int ts = (t - _originTime) / _timeStep;

#ifdef DEBUG
    int remainder = (t - _originTime) % _timeStep;
    if (remainder != 0)
    {
        utl::cout << "WARNING: A time has been rounded down (" << Time(t).toString() << "->"
                  << Time(timeSlotToTime(ts)).toString()
                  << "). [origin:" << Time(_originTime).toString()
                  << ", horizon:" << Time(_horizonTime).toString() << ", step:" << _timeStep << "s]"
                  << utl::endlf;
    }
#endif
    return ts;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int
SchedulerConfiguration::timeSlotToDuration(int ts) const
{
    return (ts * _timeStep);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int
SchedulerConfiguration::durationToTimeSlot(int d) const
{
    if (abs(d) > (_horizonTime - _originTime))
    {
        if (d < 0)
        {
            d = _originTime - _horizonTime;
        }
        else
        {
            d = _horizonTime - _originTime;
        }
    }

    // note: _originTime < t < _horizonTime
    int ts = d / _timeStep;

#ifdef DEBUG
    int remainder = d % _timeStep;
    if (remainder != 0)
    {
        utl::cout << "WARNING: A duration"
                  << " has been rounded down (" << d << "->" << timeSlotToDuration(ts)
                  << "). [maxDur:" << (_horizonTime - _originTime) << ", step:" << _timeStep << "s]"
                  << utl::endlf;
    }
#endif
    return ts;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

uint_t
SchedulerConfiguration::timeSlotToDuration(uint_t ts) const
{
    return (ts * (uint_t)_timeStep);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

uint_t
SchedulerConfiguration::durationToTimeSlot(uint_t d) const
{
    d = utl::min(d, (uint_t)(_horizonTime - _originTime));

    // note: _originTime < t < _horizonTime
    uint_t ts = d / (uint_t)_timeStep;

#ifdef DEBUG
    int remainder = d % (uint_t)_timeStep;
    if (remainder != 0)
    {
        utl::cout << "WARNING: A duration"
                  << " has been rounded down (" << d << "->" << timeSlotToDuration(ts)
                  << "). [maxDur:" << (_horizonTime - _originTime) << ", step:" << _timeStep << "s]"
                  << utl::endlf;
    }
#endif
    return ts;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
SchedulerConfiguration::init()
{
    _originTime = -1;
    _horizonTime = -1;
    _timeStep = 0;
    _schedulingOriginTime = -1;
    _autoFreezeDuration = 0;
    _useInitialAsSeed = false;
    _backward = false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;
