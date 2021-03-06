#include "libcse.h"
#include "CompositeResource.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;
CLS_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(cse::CompositeResource);

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

void
CompositeResource::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(CompositeResource));
    const CompositeResource& cr = (const CompositeResource&)rhs;
    _resGroupId = cr._resGroupId;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
CompositeResource::serialize(Stream& stream, uint_t io, uint_t)
{
    Resource::serialize(stream, io);
    utl::serialize(_resGroupId, stream, io);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
CompositeResource::init()
{
    _resGroupId = uint_t_max;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
CompositeResource::deInit()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;
