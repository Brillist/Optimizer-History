#include "libmrp.h"
#include <libutl/MemStream.h>
#include <libutl/Float.h>
#include <mrp/ProcessPlan.h>
#include <mrp/ProcessStep.h>
#include "SetupGroup.h"

//////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;

//////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(mrp::SetupGroup, utl::Object);

//////////////////////////////////////////////////////////////////////////////

MRP_NS_BEGIN;

/////////////////////////////////////////////////////////////////////////////

bool
SetupGroupOrderingIncId::operator()(
    SetupGroup* lhs,
    SetupGroup* rhs) const
{
    return (lhs->id() < rhs->id());
}

/////////////////////////////////////////////////////////////////////////////

void
SetupGroup::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(SetupGroup));
    const SetupGroup& sg = (const SetupGroup&)rhs;
    _id = sg._id;
    _setups = sg._setups;
}

//////////////////////////////////////////////////////////////////////////////

String
SetupGroup::toString() const
{
    MemStream str;
    str << "setupGroup:" << _id;
    setup_set_id_t::iterator it;
    for (it = _setups.begin(); it != _setups.end(); it++)
    {
        Setup* setup = (*it);
        str << '\n' << "   " << setup->toString();
    }
    str << '\0';
    return String((char*)str.get());
}

//////////////////////////////////////////////////////////////////////////////

void
SetupGroup::serialize(Stream& stream, uint_t io, uint_t mode)
{
    utl::serialize(_id, stream, io);
    lut::serialize<Setup*>(_setups, stream, io);
}

//////////////////////////////////////////////////////////////////////////////

void
SetupGroup::deInit()
{
    deleteCont(_setups);
}

//////////////////////////////////////////////////////////////////////////////

MRP_NS_END;
