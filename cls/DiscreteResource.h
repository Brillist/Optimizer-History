#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <libutl/Array.h>
#include <cls/Activity.h>
#include <cls/Resource.h>
#include <cls/DiscreteTimetable.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

class PtActivity;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Discrete resource.

   A DiscreteResource has:

   - a ResourceCalendar that specifies whether it's working or on break in each time slot
   - a DiscreteTimetable that tracks the required and provided capacity in each time slot

   \see BrkActivity
   \see DiscreteTimetable
   \see ResourceCalendar
   \ingroup cls
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class DiscreteResource : public Resource
{
    UTL_CLASS_DECL(DiscreteResource, Resource);

public:
    /**
       Constructor.
       \param schedule related Schedule
    */
    DiscreteResource(Schedule* schedule)
        : Resource(schedule)
    {
        init();
        _timetable.setManager(manager());
    }

    /** Initialize. */
    void initialize();

    /** Add a timetable bound. */
    void
    addTimetableBound(utl::Object* bound)
    {
        _timetableBounds += bound;
    }

    /** Add a composite-resource-id. */
    void
    addCRid(uint_t crId)
    {
        _crIds.push_back(crId);
    }

    /// \name Accessors (const)
    //@{
    /** Unary resource? */
    bool
    isUnary() const
    {
        return _unary;
    }

    /** Get the timetable. */
    const DiscreteTimetable&
    timetable() const
    {
        return _timetable;
    }

    /** Get activities sorted by start-time. */
    const act_set_es_t&
    actsByStartTime() const
    {
        return _actsByStartTime;
    }

    /** Get min required capacity */
    uint_t
    minReqCap() const
    {
        return _minReqCap;
    }

    /** Get max required capacity */
    uint_t
    maxReqCap() const
    {
        return _maxReqCap;
    }

    /** Get list of composite-resource-ids. */
    const uint_vector_t&
    crIds() const
    {
        return _crIds;
    }
    //@}

    /// \name Accessors (non-const)
    //@{
    /** Get the timetable. */
    DiscreteTimetable&
    timetable()
    {
        return _timetable;
    }

    /** Get activities sorted by start-time. */
    act_set_es_t&
    actsByStartTime()
    {
        return _actsByStartTime;
    }

    /** Set minimum required capacity. */
    void
    setMinReqCap(uint_t minReqCap)
    {
        _minReqCap = minReqCap;
    }

    /** Get maximum required capacity. */
    void
    setMaxReqCap(uint_t maxReqCap)
    {
        _maxReqCap = maxReqCap;
    }

    /** Get list of composite-resource-ids. */
    uint_vector_t&
    crIds()
    {
        return _crIds;
    }
    //@}

    /// Timetable
    //@{
    /** Add provided capacity. */
    void addProvidedCapacity(int startTime, int endTime, uint_t cap);

    /**
       Allocate capacity.
       \param min start of allocation span
       \param max end of allocation span
       \param cap allocated capacity
       \param act responsible Activity
       \param updateComposite also allocate capacity in CompositeResource%s? (default: true)
       \see CompositeResource::allocate
       \see DiscreteTimetable::allocate
    */
    void allocate(int min, int max, uint_t cap, PtActivity* act, bool updateComposite = true);

    /**
       Deallocate capacity.
       \param min start of deallocation span
       \param max end of deallocation span
       \param cap deallocated capacity
       \param updateComposite also allocate capacity in CompositeResource%s? (default: true)
       \see CompositeResource::add
       \see DiscreteTimetable::add
    */
    void deallocate(int min, int max, uint_t cap, bool updateComposite = true);

    /** Set resource's capacity to the given cap. */
    void selectCapacity(uint_t cap, uint_t maxCap);

    /** Double provided capacity for every span. */
    void doubleProvidedCap();

    /** Halve provided capacity for every span. */
    void halveProvidedCap();
    //@}

    /// \name Calendar
    //@{
    /** Get the calendar. */
    const ResourceCalendar*
    calendar() const
    {
        return _calendar;
    }

    /** Get the calendar. */
    const ResourceCalendar*&
    calendar()
    {
        return (const ResourceCalendar*&)_calendar;
    }

    /** Set the availability schedule. */
    void
    setCalendar(ResourceCalendar* calendar)
    {
        _calendar = calendar;
    }

    /** Add the given break. */
    void
    addBreak(int begin, int end)
    {
        _calendar->addBreak(begin, end);
    }

    /** Get non-break time in the given time span. */
    uint_t
    getNonBreakTime(int begin, int end) const
    {
        return ((end - begin) + 1 - getBreakTime(begin, end));
    }

    /** Get break time during the given span. */
    uint_t
    getBreakTime(int begin, int end) const
    {
        return _calendar->getBreakTime(begin, end);
    }

    /** Get maximum break time-slot <= t. */
    int
    getBreakTimePrev(int t) const
    {
        return _calendar->getBreakTimePrev(t);
    }

    /** Get minimum break time-slot >= t. */
    int
    getBreakTimeNext(int t) const
    {
        return _calendar->getBreakTimeNext(t);
    }

    /** Get maximum non-break time-slot <= t. */
    int
    getNonBreakTimePrev(int t) const
    {
        return _calendar->getNonBreakTimePrev(t);
    }

    /** Get minimum non-break time-slot >= t. */
    int
    getNonBreakTimeNext(int t) const
    {
        return _calendar->getNonBreakTimeNext(t);
    }

    /** Get start-time corresponding to end time. */
    int
    getStartTimeForEndTime(int endTime, uint_t pt) const
    {
        return _calendar->getStartTimeForEndTime(endTime, pt);
    }

    /** Get end-time corresponding to start time. */
    int
    getEndTimeForStartTime(int startTime, uint_t pt) const
    {
        return _calendar->getEndTimeForStartTime(startTime, pt);
    }
    //@}

protected:
    bool _unary;
    DiscreteTimetable _timetable;
    act_set_es_t _actsByStartTime;
    uint_t _minReqCap;
    uint_t _maxReqCap;
    utl::RBtree _timetableBounds;
    ResourceCalendar* _calendar;
    uint_vector_t _crIds;

private:
    void init();
    void deInit();
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_END;
