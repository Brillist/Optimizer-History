#include "libcls.h"
#include <clp/Bound.h>
#include <clp/FailEx.h>
#include <clp/Manager.h>
#include "DiscreteResource.h"
#include "DiscreteTimetable.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;
CLP_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(cls::DiscreteTimetable);

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

void
DiscreteTimetable::energy(uint_t& required, uint_t& provided) const
{
    uint_t totRequired = 0;
    uint_t totProvided = 0;
    const IntSpan* ts;
    for (ts = head()->next(); ts != tail(); ts = ts->next())
    {
        totRequired += ts->v0();
        totProvided += ts->v1();
    }
    required = totRequired;
    provided = totProvided;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
DiscreteTimetable::energy(const Span<int>& span, uint_t& required, uint_t& provided) const
{
    if (span.isNil())
    {
        return;
    }

    uint_t totRequired = 0;
    uint_t totProvided = 0;
    const IntSpan* ts = _domain.find(span.begin());
    while (ts != tail())
    {
        Span<int> tsSpan = ts->span();
        uint_t overlapSize = tsSpan.overlapSize(span);
        if (overlapSize == 0)
        {
            break;
        }
        totRequired += ts->v0() * overlapSize;
        totProvided += ts->v1() * overlapSize;
    }
    required = totRequired;
    provided = totProvided;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
DiscreteTimetable::setManager(Manager* mgr)
{
    if (mgr == _mgr)
    {
        return;
    }
    _mgr = mgr;
    _domain.initialize(_mgr);
    _rangeBounds.initialize(_mgr);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

uint_t
DiscreteTimetable::max(int t) const
{
    const IntSpan* ts = _domain.find(t);
    ASSERTD(ts->min() <= t);
    if (ts->min() > t)
    {
        return 0;
    }
    return ts->v1();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

const IntSpan*
DiscreteTimetable::find(int val) const
{
    const IntSpan* ts = _domain.find(val);
    ASSERTD(ts->span().contains(val));
    return ts;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

IntSpan*
DiscreteTimetable::find(int val)
{
    IntSpan* ts = _domain.find(val);
    ASSERTD(ts->span().contains(val));
    return ts;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

uint_t
DiscreteTimetable::add(int min, int max, int minReq, int maxPrv)
{
    uint_t minCap = _domain.add(min, max, minReq, maxPrv);
    if (_domain.anyEvent())
        raiseEvents();
    return minCap;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

uint_t
DiscreteTimetable::subtract(int min, int max, int minReq, int maxPrv)
{
    uint_t minCap = _domain.add(min, max, minReq, maxPrv);
    _domain.add(min, max, -minReq, -maxPrv);
    if (_domain.anyEvent())
        raiseEvents();
    return minCap;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
DiscreteTimetable::init()
{
    _res = nullptr;
    _mgr = nullptr;
    _managed = false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
DiscreteTimetable::deInit()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
DiscreteTimetable::raiseEvents()
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
        String& str = *new String();
        str = "res-" + Uint(_res->id()).toString() + ": " + "capacity overallocation";
        throw FailEx(str);
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
