#include "libcse.h"
#include "ResourceCost.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(cse::ResourceCost, utl::Object);

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ResourceCost::clear()
{
    _cost = 0.0;
    _resolution = 0;
    _costPerUnitHired = 0.0;
    _costPerHour = 0.0;
    _costPerDay = 0.0;
    _costPerWeek = 0.0;
    _costPerMonth = 0.0;
    _minEmploymentTime = 0;
    _maxIdleTime = 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ResourceCost::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(ResourceCost));
    const ResourceCost& rc = (const ResourceCost&)rhs;
    _resolution = rc._resolution;
    _costPerUnitHired = rc._costPerUnitHired;
    _costPerHour = rc._costPerHour;
    _costPerDay = rc._costPerDay;
    _costPerWeek = rc._costPerWeek;
    _costPerMonth = rc._costPerMonth;
    _minEmploymentTime = rc._minEmploymentTime;
    _maxIdleTime = rc._maxIdleTime;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ResourceCost::serialize(Stream& stream, uint_t io, uint_t)
{
    utl::serialize(_resolution, stream, io);
    utl::serialize(_costPerUnitHired, stream, io);
    utl::serialize(_costPerHour, stream, io);
    utl::serialize(_costPerDay, stream, io);
    utl::serialize(_costPerWeek, stream, io);
    utl::serialize(_costPerMonth, stream, io);
    utl::serialize(_maxIdleTime, stream, io);
    utl::serialize(_minEmploymentTime, stream, io);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ResourceCost::init()
{
    clear();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;
