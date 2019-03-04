#include "libcls.h"
#include <clp/Bound.h>
#include <clp/FailEx.h>
#include <clp/Manager.h>
#include "CompositeResource.h"
#include "CompositeTimetable.h"
#include "Schedule.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;
CLP_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(cls::CompositeTimetable);

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

void
CompositeTimetable::initialize(CompositeResource* res, Schedule* schedule, const uint_set_t& resIds)
{
    _res = res;
    ASSERTD(_mgr == nullptr);
    Manager* mgr = schedule->manager();
    _mgr = mgr;
    _domain.initialize(schedule, resIds);
    _rangeBounds.initialize(_mgr);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

const CompositeSpan*
CompositeTimetable::find(int val) const
{
    const CompositeSpan* ts = (const CompositeSpan*)_domain.find(val);
    ASSERTD(ts->span().contains(val));
    return ts;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CompositeSpan*
CompositeTimetable::find(int val)
{
    CompositeSpan* ts = (CompositeSpan*)_domain.find(val);
    ASSERTD(ts->span().contains(val));
    return ts;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
CompositeTimetable::add(int min, int max, uint_t resId)
{
    _domain.add(min, max, resId);
    if (_domain.anyEvent())
        raiseEvents();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
CompositeTimetable::allocate(int min,
                             int max,
                             uint_t cap,
                             Activity* act,
                             const PreferredResources* pr,
                             const IntExp* breakList,
                             uint_t resId,
                             bool updateDiscrete)
{
    _domain.allocate(min, max, cap, act, pr, breakList, resId, updateDiscrete);
    if (_domain.anyEvent())
        raiseEvents();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
CompositeTimetable::init()
{
    _res = nullptr;
    _mgr = nullptr;
    _managed = false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
CompositeTimetable::deInit()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
CompositeTimetable::raiseEvents()
{
    if (!_domain.anyEvent())
    {
        return;
    }

    // failure?
    if (_domain.emptyEvent())
    {
        _domain.clearEvents();
        ASSERTD(_res != nullptr);
        throw FailEx("res-" + Uint(_res->id()).toString() + ": " + "overallocation");
    }

    // something changed...
    ASSERTD(_domain.rangeEvent());
    _domain.clearEvents();
    cb_set_t::iterator it, endIt = _rangeBounds.end();
    for (it = _rangeBounds.begin(); it != endIt; ++it)
    {
        ConstrainedBound* cb = *it;
        cb->invalidate();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_END;
