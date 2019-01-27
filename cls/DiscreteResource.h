#ifndef CLS_DISCRETERESOURCE_H
#define CLS_DISCRETERESOURCE_H

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

   The availability of a discrete resource is described by a time-table.

   \see DiscreteTimetable
   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class DiscreteResource : public Resource
{
    UTL_CLASS_DECL(DiscreteResource, Resource);
    /* public: */
    /*     typedef std::vector<uint_t> uint_vector_t; */
public:
    /** Constructor. */
    DiscreteResource(Schedule* schedule)
        : Resource(schedule)
    {
        init();
        _timetable.setManager(manager());
    }

    /** Initialize. */
    void initialize();

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

    /** Get the timetable. */
    DiscreteTimetable&
    timetable()
    {
        return _timetable;
    }

    /** Get activities sorted by start-time. */
    const act_set_es_t&
    actsByStartTime() const
    {
        return _actsByStartTime;
    }

    /** Get activities sorted by start-time. */
    act_set_es_t&
    actsByStartTime()
    {
        return _actsByStartTime;
    }

    /** Add provided capacity. */
    void addProvidedCapacity(int startTime, int endTime, uint_t cap);

    /** Allocate capacity. */
    void allocate(int min, int max, uint_t cap, PtActivity* act, bool updateComposite = true);

    /** Deallocate capacity. */
    void deallocate(int min, int max, uint_t cap, bool updateComposite = true);

    /** Set resource's capacity to the given cap. */
    void selectCapacity(uint_t cap, uint_t maxCap);

    /** Get min required capacity */
    uint_t
    minReqCap() const
    {
        return _minReqCap;
    }

    /** Get min required capacity. */
    uint_t&
    minReqCap()
    {
        return _minReqCap;
    }

    /** Get max required capacity */
    uint_t
    maxReqCap() const
    {
        return _maxReqCap;
    }

    /** Get max required capacity. */
    uint_t&
    maxReqCap()
    {
        return _maxReqCap;
    }

    /** Add a timetable bound. */
    void
    addTimetableBound(utl::Object* bound)
    {
        _timetableBounds += bound;
    }

    /// \name Composite Resource Ids
    //@{
    /** Get list of composite-resource-ids. */
    const lut::uint_vect_t&
    crIds() const
    {
        return _crIds;
    }

    /** Get list of composite-resource-ids. */
    lut::uint_vect_t&
    crIds()
    {
        return _crIds;
    }

    /** Add a composite-resource-id. */
    void
    addCRid(uint_t crId)
    {
        _crIds.push_back(crId);
    }
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

    /** Double and halve provided capacity for every span.
        this two functions are added for shifting scheduled 
        operations at the end of scheduling, because of moving
        time and cost. */
    void doubleProvidedCap();
    void halveProvidedCap();

protected:
    bool _unary;
    DiscreteTimetable _timetable;
    act_set_es_t _actsByStartTime;
    uint_t _minReqCap;
    uint_t _maxReqCap;
    utl::RBtree _timetableBounds;
    ResourceCalendar* _calendar;
    lut::uint_vect_t _crIds;

private:
    typedef std::set<uint_t> uint_set_t;

private:
    void init();
    void deInit();
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_END;

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
