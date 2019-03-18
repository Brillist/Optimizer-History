#pragma once

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

   A breakable activity executes on its required resources when all of them are available and
   have the required capacity (based on the selected processing time).  The total duration of a
   breakable activity is divided between processing time (when the activity is actually being
   executed) and break time (when the activity's execution is on hold due to a break in at least
   one required resource's availability). Unlike an IntActivity, a BrkActivity's execution cannot
   be interrupted by any period of insufficient capacity for any selected resource.
      
   An activity's start and end times relate this way:

   \code
   duration = processingTime + breakTime
   startTime + duration = endTime
   \endcode

   \see DiscreteResource
   \see DiscreteResourceRequirement
   \see ESbound
   \see ResourceCalendar
   \see ResourceCalendarMgr
   \ingroup cls
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class BrkActivity : public PtActivity
{
    friend class DiscreteResourceRequirement;
    UTL_CLASS_DECL(BrkActivity, PtActivity);

public:
    /**
       Constructor.
       \param schedule parent schedule
    */
    BrkActivity(Schedule* schedule)
        : PtActivity(schedule)
    {
        init();
    }

    virtual bool selectResource(uint_t resId);

    virtual void selectPt(uint_t pt);

    /** Add a resource requirement. */
    void add(DiscreteResourceRequirement* drr);

    /** Initialize requirements. */
    void initRequirements();

    /// \name Accessors (const)
    //@{
    /** Get the break-list. */
    virtual clp::IntExp* breakList() const;

    /** Forward scheduling? */
    bool forward() const;

    /** Backward scheduling? */
    bool
    backward() const
    {
        return !forward();
    }

    /** 
       Get this activity's ResourceCalendar.
       \see ResourceCalendarMgr::add
    */
    const ResourceCalendar*
    calendar() const
    {
        return _calendar;
    }

    /** Get ownership flag for resource requirements. */
    bool
    resReqOwner() const
    {
        return _discreteReqs.isOwner();
    }
    //@}

    /// \name Accessors (non-const)
    //@{
    /** Set resource calendar. */
    void
    setCalendar(ResourceCalendar* calendar)
    {
        _calendar = calendar;
    }

    /** Get resource-requirement ownership flag. */
    void
    setResReqOwner(bool owner)
    {
        _discreteReqs.setOwner(owner);
    }
    //@}

private:
    void init();
    void deInit();

    void decrementUnknownReqs();

    void selectResource(uint_t resId, DiscreteResourceRequirement* rr);

    void addTimetableBounds();

private:
    const ResourceCalendar* _calendar;
    clp::IntVar* _selectedResources;
    uint_t _numUnknownReqs;
    utl::Array _discreteReqs;
    bool _addedTimetableBounds;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_END;
