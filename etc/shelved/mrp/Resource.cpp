#include "libmrp.h"
#include <libutl/MemStream.h>
#include "Resource.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(mrp::Resource);

////////////////////////////////////////////////////////////////////////////////////////////////////

MRP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Resource::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(Resource));
    const Resource& res = (const Resource&)rhs;
    _id = res._id;
    _name = res._name;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int
Resource::compare(const Object& rhs) const
{
    if (!rhs.isA(Resource))
    {
        return Object::compare(rhs);
    }
    const Resource& res = (const Resource&)rhs;
    return utl::compare(_id, res._id);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Resource::serialize(Stream& stream, uint_t io, uint_t)
{
    utl::serialize(_id, stream, io);
    lut::serialize(_name, stream, io);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

String
Resource::toString() const
{
    MemStream str;
    str << "resource:" << _id << ", name:" << _name.c_str() << '\0';
    return String((char*)str.get());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Resource::init()
{
    _id = uint_t_max;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Resource::deInit()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////

MRP_NS_END;