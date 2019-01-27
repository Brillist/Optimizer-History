#include "libcls.h"
#include "ResourceCalendarSpan.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;
CLP_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(cls::ResourceCalendarSpan);

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ResourceCalendarSpan::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(ResourceCalendarSpan));
    const ResourceCalendarSpan& rcs = (const ResourceCalendarSpan&)rhs;
    Span<int>::copy(rcs);
    _type = rcs._type;
    _status = rcs._status;
    _cap = rcs._cap;
    _prev = nullptr;
    _next = nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int
ResourceCalendarSpan::compare(const Object& rhs) const
{
    int res = Span<int>::compare(rhs);
    if ((res != 0) || !rhs.isA(ResourceCalendarSpan))
    {
        return res;
    }

    const ResourceCalendarSpan& rcs = (const ResourceCalendarSpan&)rhs;
    res = utl::compare(_status, rcs._status);
    if (res != 0)
        return res;
    res = utl::compare(_cap, rcs._cap);
    return res;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ResourceCalendarSpan::serialize(utl::Stream& stream, uint_t io, uint_t)
{
    Span<int>::serialize(stream, io);
    utl::serialize((uint_t&)_type, stream, io);
    utl::serialize((uint_t&)_status, stream, io);
    utl::serialize(_cap, stream, io);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

String
ResourceCalendarSpan::toString() const
{
    return toString(-1, 0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

String
ResourceCalendarSpan::toString(time_t originTime, uint_t timeStep) const
{
    std::ostringstream ss;
    ss << "[";
    if (_begin >= 1000000000)
    {
        time_t bt = _begin;
        time_t et = _end;
        ss << _begin << ":" << time_str(bt) << ", " << _end << ":" << time_str(et);
    }
    else if ((originTime >= 0) && (timeStep > 0))
    {
        time_t bt = originTime + (_begin * timeStep);
        time_t et = originTime + (_end * timeStep);
        ss << _begin << ":" << time_str(bt) << "," << _end << ":" << time_str(et);
    }
    else
    {
        ss << _begin << "," << _end;
    }
    ss << "): ";
    if (_status == rcss_onBreak)
        ss << "onBreak";
    else
        ss << "available";
    if (_cap != uint_t_max)
    {
        ss << ":  capacity: " << _cap;
    }
    return String(ss.str().c_str());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ResourceCalendarSpan::init(rcs_t type, rcs_status_t status, uint_t cap)
{
    _type = type;
    _status = status;
    _cap = cap;
    _cumPt = uint_t_max;
    _prev = nullptr;
    _next = nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_END;
