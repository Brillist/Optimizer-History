#include "libcse.h"
#include "TimeSlot.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(cse::TimeSlot);

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

void
TimeSlot::copy(const Object& rhs)
{
    auto& ts = utl::cast<TimeSlot>(rhs);
    _begin = ts._begin;
    _end = ts._end;
    _reqCap = ts._reqCap;
    _prvCap = ts._prvCap;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
TimeSlot::serialize(Stream& stream, uint_t io, uint_t)
{
    lut::serialize(_begin, stream, io);
    lut::serialize(_end, stream, io);
    utl::serialize(_reqCap, stream, io);
    utl::serialize(_prvCap, stream, io);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;
