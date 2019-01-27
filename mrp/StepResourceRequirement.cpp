#include "libmrp.h"
#include <libutl/MemStream.h>
#include "StepResourceRequirement.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(mrp::StepResourceRequirement);

////////////////////////////////////////////////////////////////////////////////////////////////////

MRP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

void
StepResourceRequirement::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(StepResourceRequirement));
    const StepResourceRequirement& psrr = (const StepResourceRequirement&)rhs;
    _resourceId = psrr._resourceId;
    _capacity = psrr._capacity;
    _setupId = psrr._setupId;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
StepResourceRequirement::serialize(Stream& stream, uint_t io, uint_t)
{
    utl::serialize(_resourceId, stream, io);
    utl::serialize(_capacity, stream, io);
    utl::serialize(_setupId, stream, io);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

cse::ResourceRequirement*
StepResourceRequirement::createResourceRequirement() const
{
    cse::ResourceRequirement* resReq = new cse::ResourceRequirement();
    resReq->resourceId() = _resourceId;
    resReq->capacity() = _capacity;
    return resReq;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

cse::ResourceGroupRequirement*
StepResourceRequirement::createResourceGroupRequirement(uint_t groupId) const
{
    cse::ResourceGroupRequirement* resGroupReq = new cse::ResourceGroupRequirement();
    resGroupReq->resourceGroupId() = groupId;
    resGroupReq->capacity() = _capacity;
    return resGroupReq;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

String
StepResourceRequirement::toString() const
{
    MemStream str;
    str << "stepResourceRequirement"
        << ", resource:" << _resourceId << ", capacity:" << _capacity << ", setup:";
    if (_setupId == uint_t_max)
    {
        str << "null";
    }
    else
    {
        str << _setupId;
    }
    str << '\0';
    return String((char*)str.get());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
StepResourceRequirement::init()
{
    _resourceId = _capacity = _setupId = uint_t_max;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
StepResourceRequirement::deInit()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////

MRP_NS_END;
