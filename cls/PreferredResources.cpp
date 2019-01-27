#include "libcls.h"
#include "PreferredResources.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(cls::PreferredResources);

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

void
PreferredResources::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(PreferredResources));
    const PreferredResources& pr = (const PreferredResources&)rhs;
    _resIds = pr._resIds;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
PreferredResources::serialize(Stream& stream, uint_t io, uint_t)
{
    if (io == io_rd)
        _resIds.clear();
    lut::serialize<uint_t>(_resIds, stream, io);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_END;
