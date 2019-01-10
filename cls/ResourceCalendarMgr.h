#ifndef CLS_RESOURCECALENDARMGR_H
#define CLS_RESOURCECALENDARMGR_H

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <cls/ResourceCalendar.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

class Schedule;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Resource calendar manager.

   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class ResourceCalendarMgr : public utl::Object
{
    UTL_CLASS_DECL(ResourceCalendarMgr);
    UTL_CLASS_NO_COPY;

public:
    ResourceCalendarMgr(Schedule* schedule)
    {
        init();
        _schedule = schedule;
    }

    /** Get horizon time-slot. */
    utl::uint_t&
    horizonTS()
    {
        return _horizonTS;
    }

    /** Find a calendar by unique id. */
    const ResourceCalendar* find(utl::uint_t id) const;

    /** Add the given simple calendar. */
    ResourceCalendar* add(ResourceCalendar* cal);

    /** Add the given composite calendar. */
    ResourceCalendar* add(const ResourceCalendarSpec& spec);

private:
    void init();
    void deInit();
    ResourceCalendar* build(const ResourceCalendarSpec& spec);

private:
    typedef std::set<ResourceCalendar*, ResourceCalendarOrdering> rescal_set_t;
    typedef std::map<ResourceCalendarSpec, ResourceCalendar*> rescal_map_t;

private:
    Schedule* _schedule;
    utl::uint_t _horizonTS;
    rescal_set_t _calendars;
    rescal_map_t _specCalendars;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_END;

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
