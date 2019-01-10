#ifndef CLS_BREAKABLEACTIVITY_H
#define CLS_BREAKABLEACTIVITY_H

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <libutl/Array.h>
#include <clp/Bound.h>
#include <cls/DiscreteResourceRequirement.h>
#include <cls/PtActivity.h>
#include <cls/ResourceCalendar.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

class ESbound;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Breakable activity.

   A breakable activity is an activity whose execution can be
   interrupted by \b breaks in resource availability
   (see Resource::addBreak()).

   The duration of a breakable activity is divided between processing time
   (when the activity is actually being executed) and break time (when
   the activity's execution is on hold due to a break in a required
   resource's availability).

   The following statements sum up the situation:

   \code
   duration = processingTime + breakTime
   startTime + duration = endTime
   startTime + processingTime + breakTime = endTime
   \endcode

   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class BrkActivity : public PtActivity
{
    friend class DiscreteResourceRequirement;
    UTL_CLASS_DECL(BrkActivity);

public:
    typedef std::set<utl::uint_t> uint_set_t;

public:
    /**
       Constructor.
       \param schedule owning schedule
    */
    BrkActivity(Schedule* schedule)
        : PtActivity(schedule)
    {
        init();
    }

    /** Add a requirement. */
    void add(DiscreteResourceRequirement* drr);

    /** Initialize requirements. */
    void initRequirements();

    /**
       Select a resource.
       DEPRECATED: what if two requirements share the same resource?
    */
    bool selectResource(utl::uint_t resId);

    /** Get the break-list. */
    virtual clp::IntExp* breakList() const;

    /** Select the given processing-time. */
    virtual void selectPt(utl::uint_t pt);

    /** Forward scheduling? */
    bool forward() const;

    /** Backward scheduling? */
    bool
    backward() const
    {
        return !forward();
    }

    /** Get calendar. */
    const ResourceCalendar*
    calendar() const
    {
        return _calendar;
    }

    /** Get calendar. */
    const ResourceCalendar*&
    calendar()
    {
        return _calendar;
    }

    /** Get the list of all resource-ids. */
    /*     const uint_set_t& allResIds() const */
    /*     { return _allResIds; } */

    /** Get resource-requirement ownership flag. */
    bool
    resReqOwner() const
    {
        return _discreteReqs.isOwner();
    }

    /** Get resource-requirement ownership flag. */
    void
    setResReqOwner(bool owner)
    {
        _discreteReqs.setOwner(owner);
    }

private:
    void init();
    void deInit();

    void decrementUnknownReqs();

    void selectResource(utl::uint_t resId, DiscreteResourceRequirement* rr);

    void addTimetableBounds();

private:
    const ResourceCalendar* _calendar;
    /*     uint_set_t _allResIds; */
    clp::IntVar* _selectedResources;
    utl::uint_t _numUnknownReqs;
    utl::Array _discreteReqs;
    bool _addedTimetableBounds;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_END;

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
