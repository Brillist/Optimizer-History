#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <cls/ResourceCalendar.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

class Schedule;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   ResourceCalendar manager.

   ResourceCalendarMgr provides central management of ResourceCalendar%s.

   There are two types of ResourceCalendar:

   - **simple**: applying only to a single resource
   - **composite**: intersecting calendars for multiple resources
     (recording a break in availability when *any* of the resources is on break)

   \ingroup cls
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class ResourceCalendarMgr : public utl::Object
{
    UTL_CLASS_DECL(ResourceCalendarMgr, utl::Object);
    UTL_CLASS_NO_COPY;

public:
    ResourceCalendarMgr(Schedule* schedule)
    {
        init();
        _schedule = schedule;
    }

    /** Get horizon time-slot. */
    uint_t
    horizonTS() const
    {
        return _horizonTS;
    }

    /** Set horizon time-slot. */
    void
    setHorizonTS(uint_t horizonTS)
    {
        _horizonTS = horizonTS;
    }

    /** Find a calendar by unique id. */
    const ResourceCalendar* find(uint_t id) const;

    /** Add the given simple calendar. */
    ResourceCalendar* add(ResourceCalendar* cal);

    /** Add (or find) a composite calendar for the given specification. */
    ResourceCalendar* add(const ResourceCalendarSpec& spec);

private:
    void init();
    void deInit();
    ResourceCalendar* build(const ResourceCalendarSpec& spec);

private:
    using rescal_set_t = std::set<ResourceCalendar*, ResourceCalendarOrdering>;
    using rescal_map_t = std::map<ResourceCalendarSpec, ResourceCalendar*>;

private:
    Schedule* _schedule;
    uint_t _horizonTS;
    rescal_set_t _calendars;
    rescal_map_t _specCalendars;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_END;
