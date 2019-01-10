#include "libcls.h"
#include "CompositeResource.h"
#include "DiscreteResource.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef DEBUG
//#define DEBUG_UNIT
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;
CLP_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(cls::CompositeResource, cls::Resource);

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

void
CompositeResource::initialize()
{
    // initialize timetable
    Schedule* sched = schedule();
    uint_set_t resIds;
    dres_vector_t::iterator resIt;
    for (resIt = _resources.begin(); resIt != _resources.end(); ++resIt)
    {
        DiscreteResource* res = *resIt;
        resIds.insert(res->serialId());
    }
    _timetable.initialize(this, sched, resIds);

    // add capacity from each resource
    for (resIt = _resources.begin(); resIt != _resources.end(); ++resIt)
    {
        DiscreteResource* res = *resIt;
        uint_t resId = res->serialId();
        const ResourceCalendar& cal = *res->calendar();
        const DiscreteTimetable& tt = res->timetable();
        ResourceCalendar::iterator calIt = cal.begin();
        ResourceCalendar::iterator calendarEnd = cal.end();
        const IntSpan* ttSpan = tt.head();

        int spanMin = int_t_max;
        int spanMax = int_t_max;
        while ((calIt != calendarEnd) && !ttSpan->isTail())
        {
            const ResourceCalendarSpan* calSpan = (ResourceCalendarSpan*)*calIt;

            // skip calSpan if not available
            if (calSpan->status() != rcss_available)
            {
                ++calIt;
                continue;
            }

            // skip ttSpan if no capacity
            if (ttSpan->capacity() < 100)
            {
                ttSpan = ttSpan->next();
                continue;
            }

            // check for overlap
            int calMin = calSpan->getBegin();
            int calMax = calSpan->getEnd() - 1;
            int ttMin = ttSpan->min();
            int ttMax = ttSpan->max();
            int overlapMin = utl::max(calMin, ttMin);
            int overlapMax = utl::min(calMax, ttMax);

            // if there is overlap, add capacity
            if (overlapMin <= overlapMax)
            {
                if (spanMin == int_t_max)
                {
                    spanMin = overlapMin;
                    spanMax = overlapMax;
                }
                else
                {
                    if (overlapMin == (spanMax + 1))
                    {
                        spanMax = overlapMax;
                    }
                    else
                    {
                        _timetable.add(spanMin, spanMax, resId);
                        spanMin = overlapMin;
                        spanMax = overlapMax;
                    }
                }
            }

            // move forward
            if (calMax < ttMax)
            {
                ++calIt;
            }
            else if (ttMax < calMax)
            {
                ttSpan = ttSpan->next();
            }
            else // calMax == ttMax
            {
                ++calIt;
                ttSpan = ttSpan->next();
            }
        }

        // add leftover span
        if (spanMin != int_t_max)
        {
            _timetable.add(spanMin, spanMax, resId);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
CompositeResource::add(int min, int max, uint_t resId)
{
    _timetable.add(min, max, resId);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
CompositeResource::allocate(int min,
                            int max,
                            uint_t cap,
                            Activity* act,
                            const PreferredResources* pr,
                            const IntExp* breakList,
                            uint_t resId,
                            bool updateDiscrete)
{
#ifdef DEBUG_UNIT
    static uint_t iter = 0;
    static uint_t debugId = 57;
    static uint_t debugTS = 64290;
    static uint_t debugMemberId = 44;

    if (id() == debugId)
    {
        ++iter;
        CompositeSpan* span = _timetable.find(debugTS);
        IntExpDomainAR* resIds = span->resIds();
        if (!resIds->has(debugMemberId))
        {
            BREAKPOINT;
        }
    }
#endif

    _timetable.allocate(min, max, cap, act, pr, breakList, resId, updateDiscrete);

#ifdef DEBUG_UNIT
    if (id() == debugId)
    {
        CompositeSpan* span = _timetable.find(debugTS);
        IntExpDomainAR* resIds = span->resIds();
        if (!resIds->has(debugMemberId))
        {
            BREAKPOINT;
        }
    }
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_END;
