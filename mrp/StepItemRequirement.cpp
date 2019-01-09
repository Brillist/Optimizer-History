#include "libmrp.h"
#include <libutl/MemStream.h>
#include "StepItemRequirement.h"

//////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;

//////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(mrp::StepItemRequirement, utl::Object);

//////////////////////////////////////////////////////////////////////////////

MRP_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////

void
StepItemRequirement::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(StepItemRequirement));
    const StepItemRequirement& psir
        = (const StepItemRequirement&) rhs;
    _itemId = psir._itemId;
    _quantity = psir._quantity;
}

//////////////////////////////////////////////////////////////////////////////

void
StepItemRequirement::serialize(Stream& stream, uint_t io, uint_t)
{
    utl::serialize(_itemId, stream, io);
    utl::serialize(_quantity, stream, io);
}

//////////////////////////////////////////////////////////////////////////////

String
StepItemRequirement::toString() const
{
    MemStream str;
    str << "stepItemReq"
        << ", item:" << _itemId
        << ", quantity:" << _quantity
        << '\0';
    return String((char*)str.get());
}

//////////////////////////////////////////////////////////////////////////////

MRP_NS_END;
