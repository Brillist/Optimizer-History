#include "libcse.h"
#include <cse/JobOp.h>
#include "ResourceGroupRequirement.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;
CLS_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(cse::ResourceGroupRequirement);

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ResourceGroupRequirement::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(ResourceGroupRequirement));
    const ResourceGroupRequirement& rgr = (const ResourceGroupRequirement&)rhs;
    _resourceGroupId = rgr._resourceGroupId;
    _capacity = rgr._capacity;
    _scheduledResourceId = rgr._scheduledResourceId;
    _scheduledCapacity = rgr._scheduledCapacity;
    _clsResReq = rgr._clsResReq;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ResourceGroupRequirement::serialize(Stream& stream, uint_t io, uint_t)
{
    utl::serialize(_resourceGroupId, stream, io);
    utl::serialize(_capacity, stream, io);
    utl::serialize(_scheduledResourceId, stream, io);
    utl::serialize(_scheduledCapacity, stream, io);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool
ResourceGroupRequirement::hasNonZeroCapPt(const JobOp* op, const ResourceGroup* resGroup) const
{
    if ((_capacity != 0) && (_capacity != uint_t_max))
        return true;

    const std::set<uint_t>& resIds = resGroup->resIds();
    std::set<uint_t>::const_iterator it;
    for (it = resIds.begin(); it != resIds.end(); ++it)
    {
        uint_t resId = *it;
        const ResourceCapPts* resCapPts = op->resCapPts(resId);
        if ((resCapPts != nullptr) && resCapPts->hasNonZeroCapPt())
        {
            return true;
        }
    }

    return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ResourceGroupRequirement::init()
{
    _resourceGroupId = uint_t_max;
    _capacity = uint_t_max;
    _scheduledResourceId = uint_t_max;
    _scheduledCapacity = uint_t_max;
    _clsResReq = nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;
