#include "libcse.h"
#include "UnaryCt.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(cse::UnaryCt, utl::Object);

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

void
UnaryCt::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(UnaryCt));
    const UnaryCt& uct = (const UnaryCt&)rhs;
    _type = uct._type;
    _time = uct._time;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
UnaryCt::serialize(Stream& stream, uint_t io, uint_t)
{
    utl::serialize((uint_t&)_type, stream, io);
    lut::serialize(_time, stream, io);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
UnaryCt::init()
{
    _type = uct_undefined;
    _time = -1;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;
