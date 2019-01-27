#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Scheduler configuration.

   SchedulerConfiguration stores global scheduling parameters.

   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class SchedulerConfiguration : public utl::Object
{
    UTL_CLASS_DECL(SchedulerConfiguration, utl::Object);

public:
    virtual void copy(const utl::Object& rhs);

    virtual void serialize(utl::Stream& stream, uint_t io, uint_t mode = utl::ser_default);

    /** Get the originTime. */
    time_t
    originTime() const
    {
        return _originTime;
    }

    /** Get the originTime. */
    time_t&
    originTime()
    {
        return _originTime;
    }

    /** Get the origin time-slot. */
    int
    originTimeSlot() const
    {
        return 0;
    }

    /** Get the horizonTime. */
    time_t
    horizonTime() const
    {
        return _horizonTime;
    }

    /** Get the horizonTime. */
    time_t&
    horizonTime()
    {
        return _horizonTime;
    }

    /** Get the horizon time-slot. */
    int
    horizonTimeSlot() const
    {
        return timeToTimeSlot(_horizonTime);
    }

    /** Get the time-step. */
    uint_t
    timeStep() const
    {
        return _timeStep;
    }

    /** Get the time-step. */
    uint_t&
    timeStep()
    {
        return _timeStep;
    }

    /** Get schedulingOriginTime. */
    time_t
    schedulingOriginTime() const
    {
        return _schedulingOriginTime;
    }

    /** Get scheduling-origin-time. */
    time_t&
    schedulingOriginTime()
    {
        return _schedulingOriginTime;
    }

    /** Get scheduling-origin time-slot. */
    int
    schedulingOriginTimeSlot() const
    {
        return timeToTimeSlot(_schedulingOriginTime);
    }

    /** Get the auto-freeze duration. */
    uint_t
    autoFreezeDuration() const
    {
        return _autoFreezeDuration;
    }

    /** Get the auto-freeze duration. */
    uint_t&
    autoFreezeDuration()
    {
        return _autoFreezeDuration;
    }

    /** Get the initial seed flag */
    bool
    useInitialAsSeed() const
    {
        return _useInitialAsSeed;
    }

    /** Get the initial seed flag */
    bool&
    useInitialAsSeed()
    {
        return _useInitialAsSeed;
    }

    /** Get forward-scheduling flag. */
    bool
    forward() const
    {
        return !_backward;
    }

    /** Get backward-scheduling flag. */
    bool
    backward() const
    {
        return _backward;
    }

    /** Get backward-scheduling flag. */
    bool&
    backward()
    {
        return _backward;
    }

    /** Convert a time-slot to a time. */
    time_t timeSlotToTime(int ts) const;

    /** Convert a time to a time-slot. */
    int timeToTimeSlot(time_t t) const;

    /** Convert a time-slot to a duration. */
    int timeSlotToDuration(int ts) const;

    /** Convert a time-slot to a duration. */
    uint_t timeSlotToDuration(uint_t ts) const;

    /** Convert a duration to a time-slot. */
    int durationToTimeSlot(int t) const;

    /** Convert a duration to a time-slot. */
    uint_t durationToTimeSlot(uint_t) const;

private:
    void init();
    void
    deInit()
    {
    }

private:
    time_t _originTime;
    time_t _horizonTime;
    uint_t _timeStep;
    time_t _schedulingOriginTime;
    uint_t _autoFreezeDuration;
    bool _useInitialAsSeed;
    bool _backward;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;
