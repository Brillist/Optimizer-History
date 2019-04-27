#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Scheduler configuration.

   \ingroup cse
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class SchedulerConfiguration : public utl::Object
{
    UTL_CLASS_DECL(SchedulerConfiguration, utl::Object);

public:
    virtual void copy(const utl::Object& rhs);

    virtual void serialize(utl::Stream& stream, uint_t io, uint_t mode = utl::ser_default);

    /// \name Accessors (const)
    //@{
    /** Get the originTime. */
    time_t
    originTime() const
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

    /** Get schedulingOriginTime. */
    time_t
    schedulingOriginTime() const
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

    /** Get the initial seed flag */
    bool
    useInitialAsSeed() const
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
    //@}

    /// \name Accessors (non-const)
    //@{
    /** Set the originTime. */
    void
    setOriginTime(time_t originTime)
    {
        _originTime = originTime;
    }

    /** Set the horizonTime. */
    void
    setHorizonTime(time_t horizonTime)
    {
        _horizonTime = horizonTime;
    }

    /** Set the time-step. */
    void
    setTimeStep(uint_t timeStep)
    {
        _timeStep = timeStep;
    }

    /** Set scheduling-origin-time. */
    void
    setSchedulingOriginTime(time_t schedulingOriginTime)
    {
        _schedulingOriginTime = schedulingOriginTime;
    }

    /** Set the auto-freeze duration. */
    void
    setAutoFreezeDuration(uint_t autoFreezeDuration)
    {
        _autoFreezeDuration = autoFreezeDuration;
    }

    /** Set the use-initial-as-seed flag */
    void
    setUseInitialAsSeed(bool useInitialAsSeed)
    {
        _useInitialAsSeed = useInitialAsSeed;
    }

    /** Get backward-scheduling flag. */
    void
    setBackward(bool backward)
    {
        _backward = backward;
    }
    //@}

    /// \name Convert between time_t (or seconds) and time-slots
    //@{
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
    //@}

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
