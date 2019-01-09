#include "libmrp.h"
#include <libutl/MemStream.h>
#include "ResourceGroup.h"

//////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;

//////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(mrp::ResourceGroup, utl::Object);

//////////////////////////////////////////////////////////////////////////////

MRP_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////

void
ResourceGroup::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(ResourceGroup));
    const ResourceGroup& resGroup = (const ResourceGroup&)rhs;
    _id = resGroup._id;
    _name = resGroup._name;
    _resIds = resGroup._resIds;
}

//////////////////////////////////////////////////////////////////////////////

int
ResourceGroup::compare(const Object& rhs) const
{
    if (!rhs.isA(ResourceGroup))
    {
        return Object::compare(rhs);
    }
    const ResourceGroup& resGroup = (const ResourceGroup&)rhs;
    return utl::compare(_id, resGroup._id);
}

//////////////////////////////////////////////////////////////////////////////

void
ResourceGroup::serialize(Stream& stream, uint_t io, uint_t)
{
    utl::serialize(_id, stream, io);
    lut::serialize(_name, stream, io);
    lut::serialize<uint_t>(_resIds, stream, io);
}

//////////////////////////////////////////////////////////////////////////////

cse::ResourceGroup*
ResourceGroup::createResourceGroup()
{
    cse::ResourceGroup* resGroup = new cse::ResourceGroup();
    resGroup->id() = _id;
    resGroup->name() = _name;
    resGroup->resIds() = _resIds;
    return resGroup;
}

//////////////////////////////////////////////////////////////////////////////

String
ResourceGroup::toString() const
{
    MemStream str;
    str << "resourceGroup:" << _id
        << ", name:" << _name.c_str();
    for (uint_set_t::const_iterator it = _resIds.begin();
         it != _resIds.end(); it++)
    {
        if (it != _resIds.begin())
            str << ',';
        str << *it;
    }
    str << '\0';
    return String((char*)str.get());
}

//////////////////////////////////////////////////////////////////////////////

void
ResourceGroup::init()
{
    _id = uint_t_max;
}

//////////////////////////////////////////////////////////////////////////////

void
ResourceGroup::deInit()
{
}

//////////////////////////////////////////////////////////////////////////////

MRP_NS_END;
