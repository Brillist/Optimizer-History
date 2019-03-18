#include "libcls.h"
#include <cls/Schedule.h>
#include "ResourceCalendarMgr.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;
CLP_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(cls::ResourceCalendarMgr);

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

const ResourceCalendar*
ResourceCalendarMgr::find(uint_t id) const
{
    uint_set_t calIds;
    calIds.insert(id);
    ResourceCalendarSpec spec(rc_simple, calIds);
    auto it = _specCalendars.find(spec);
    if (it == _specCalendars.end())
        return nullptr;
    return (*it).second;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

ResourceCalendar*
ResourceCalendarMgr::add(ResourceCalendar* cal)
{
    ASSERTD(_horizonTS != uint_t_max);

    // compile the calendar and find a match
    cal->compile(_horizonTS);
    auto it = _calendars.find(cal);

    // not found => add it
    if (it == _calendars.end())
    {
        // set the calendar-spec
        uint_set_t calIds;
        uint_t calId = _calendars.size();
        calIds.insert(calId);
        ResourceCalendarSpec spec(rc_simple, calIds);
        cal->serialId() = calId;
        cal->spec() = spec;

        // build break-list
        cal->makeBreakList(_schedule->manager());

        // store the calendar
        _calendars.insert(cal);
        _specCalendars.insert(rescal_map_t::value_type(cal->spec(), cal));
    }
    else // return found calendar
    {
        delete cal;
        cal = (*it);
    }

    return cal;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

ResourceCalendar*
ResourceCalendarMgr::add(const ResourceCalendarSpec& spec)
{
    ASSERTD(_horizonTS != uint_t_max);
    auto it = _specCalendars.find(spec);

    // not found -> add it
    ResourceCalendar* cal;
    if (it == _specCalendars.end())
    {
        cal = build(spec);
        _specCalendars.insert(rescal_map_t::value_type(spec, cal));
    }
    else
    {
        cal = (*it).second;
    }

    return cal;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ResourceCalendarMgr::init()
{
    _schedule = nullptr;
    _horizonTS = uint_t_max;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ResourceCalendarMgr::deInit()
{
    deleteMapSecond(_specCalendars);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

ResourceCalendar*
ResourceCalendarMgr::build(const ResourceCalendarSpec& spec)
{
    // only support all-available currently
    ASSERTD(spec.type() == rc_allAvailable);
    auto& calIds = spec.calIds();

    // make new calendar
    auto cal = new ResourceCalendar();
    cal->spec() = spec;

    // add break spans from all calendars
    // (on break when _any_ resource is on break)
    for (auto calId : calIds)
    {
        auto idCal = find(calId);
        ASSERTD(idCal != nullptr);
        idCal->addCompiledSpansTo(cal, rcss_onBreak);
    }

    cal->compile(_horizonTS);
    cal->makeBreakList(_schedule->manager());
    return cal;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_END;
