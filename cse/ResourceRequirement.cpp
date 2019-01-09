#include "libcse.h"
#include "ResourceRequirement.h"

//////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;
CLS_NS_USE;

//////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(cse::ResourceRequirement, utl::Object);

//////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////

void
ResourceRequirement::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(ResourceRequirement));
    const ResourceRequirement& rr = (const ResourceRequirement&)rhs;
    _resourceId = rr._resourceId;
    _capacity = rr._capacity;
    _maxCapacity = rr._maxCapacity;
    _scheduledCapacity = rr._scheduledCapacity;
    _beginTime = rr._beginTime;
    _endTime = rr._endTime;
    _isSystem = rr._isSystem;
    delete _preferredResources;
    _preferredResources = utl::clone(rr._preferredResources);
    _clsResReq = rr._clsResReq;
}

//////////////////////////////////////////////////////////////////////////////

void
ResourceRequirement::serialize(Stream& stream, uint_t io, uint_t)
{
    utl::serialize(_resourceId, stream, io);
    utl::serialize(_capacity, stream, io);
    utl::serialize(_maxCapacity, stream, io);
    utl::serialize(_scheduledCapacity, stream, io);
    lut::serialize(_beginTime, stream, io);
    lut::serialize(_endTime, stream, io);
    utl::serialize(_isSystem, stream, io);

    // _preferredResources
    utl::Object* pr = _preferredResources;
    serializeNullable(pr, stream, io);
    _preferredResources = (PreferredResources*)pr;
}

//////////////////////////////////////////////////////////////////////////////

bool
ResourceRequirement::hasNonZeroCapPt(const ResourceCapPts* resCapPts) const
{
    if ((_capacity != 0) && (_capacity != uint_t_max)) return true;
    if (resCapPts == nullptr) return false;
    return resCapPts->hasNonZeroCapPt();
}

//////////////////////////////////////////////////////////////////////////////

void
ResourceRequirement::init()
{
    _resourceId = uint_t_max;
    _capacity = uint_t_max;
    _maxCapacity = uint_t_max;
    _scheduledCapacity = uint_t_max;
    _beginTime = -1;
    _endTime = -1;
    _isSystem = false;
    _preferredResources = nullptr;
    _clsResReq = nullptr;
}

//////////////////////////////////////////////////////////////////////////////

void
ResourceRequirement::deInit()
{
    delete _preferredResources;
}

//////////////////////////////////////////////////////////////////////////////

CSE_NS_END;
