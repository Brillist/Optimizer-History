#include "libmrp.h"
#include "AlternateResources.h"

//////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;

//////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(mrp::AlternateResources, mrp::Resource);

//////////////////////////////////////////////////////////////////////////////

MRP_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////

void
AlternateResources::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(Resource));
    const AlternateResources& ar = (const AlternateResources&)rhs;
    Resource::copy(ar);
    _resourceGroupId = ar._resourceGroupId;
}

//////////////////////////////////////////////////////////////////////////////

void
AlternateResources::serialize(Stream& stream, uint_t io, uint_t)
{
    Resource::serialize(stream, io);
    utl::serialize(_resourceGroupId, stream, io);
}

//////////////////////////////////////////////////////////////////////////////

// ResourceGroup*
// AlternateResource::createResourceGroup()
// {
//     cse::ResourceGroup* resGroup = new cse::ResourceGroup();
//     resGroup->id() = _resourceGroupId;
//     /// more code??
//         return resGroup;
// }

//////////////////////////////////////////////////////////////////////////////

MRP_NS_END;
