#include "libmrp.h"
#include <cse/DiscreteResource.h>
#include "DiscreteResource.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(mrp::DiscreteResource, mrp::Resource);

////////////////////////////////////////////////////////////////////////////////////////////////////

MRP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

void
DiscreteResource::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(Resource));
    const DiscreteResource& dr = (const DiscreteResource&)rhs;
    Resource::copy(dr);
    _capacity = dr._capacity;
    _setupGroupOwner = dr._setupGroupOwner;
    if (_setupGroupOwner)
    {
        _setupGroup = utl::clone(dr._setupGroup);
    }
    else
    {
        _setupGroup = dr._setupGroup;
    }
    _defaultCalendar = dr._defaultCalendar;
    _detailedCalendar = dr._detailedCalendar;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
DiscreteResource::serialize(Stream& stream, uint_t io, uint_t)
{
    Resource::serialize(stream, io);
    utl::serialize(_capacity, stream, io);
    utl::serialize(_setupGroup->id(), stream, io);
    utl::serializeNullable(_cost, stream, io);
    utl::serializeNullable(_defaultCalendar, stream, io);
    utl::serializeNullable(_detailedCalendar, stream, io);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
DiscreteResource::setSetupGroup(SetupGroup* group, bool owner)
{
    if (_setupGroupOwner)
        delete _setupGroup;
    _setupGroup = group;
    _setupGroupOwner = owner;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

cse::DiscreteResource*
DiscreteResource::createDiscreteResource()
{
    // refer to cse::DiscreteResource::serialize()
    cse::DiscreteResource* res = new cse::DiscreteResource();
    res->id() = _id;
    res->name() = _name;
    if (_setupGroup)
        res->sequenceId() = _setupGroup->id();
    else
        res->sequenceId() = uint_t_max;
    res->minCap() = _capacity;
    res->existingCap() = 0;
    //     res->existingCap() = _capacity;
    res->maxCap() = _capacity;
    res->stepCap() = 100;
    res->setCost(utl::clone(_cost));
    res->setDefaultCalendar(utl::clone(_defaultCalendar));
    res->setDetailedCalendar(utl::clone(_detailedCalendar));
    return res;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

String
DiscreteResource::toString() const
{
    MemStream str;
    str << Resource::toString() << ", type:Discrete"
        << ", capacity:" << _capacity << ", setupGroup:";
    if (_setupGroup)
        str << _setupGroup->id() << '\0';
    else
        str << "null" << '\0';
    return String((char*)str.get());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
DiscreteResource::init()
{
    _capacity = uint_t_max;
    _setupGroup = new SetupGroup();
    _setupGroupOwner = true;
    _cost = nullptr;
    _defaultCalendar = nullptr;
    _detailedCalendar = nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
DiscreteResource::deInit()
{
    setSetupGroup(nullptr);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

MRP_NS_END;
