#include "libmrp.h"
#include <libutl/MemStream.h>
#include <libutl/Float.h>
#include <libutl/Time.h>
#include "InventoryTransaction.h"

#include <libutl/BufferedFDstream.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(mrp::InventoryTransaction);

////////////////////////////////////////////////////////////////////////////////////////////////////

MRP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

bool
InventoryTransactionOrderingIncT::operator()(InventoryTransaction* lhs,
                                             InventoryTransaction* rhs) const
{
    // note: use "<=" here, because we allow more than two trans at a time
    return (lhs->transTime() <= rhs->transTime());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
InventoryTransaction::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(InventoryTransaction));
    const InventoryTransaction& invTrans = (const InventoryTransaction&)rhs;
    _transTime = invTrans._transTime;
    _status = invTrans._status;
    _quantity = invTrans._quantity;
    _opType = invTrans._opType;
    _opId = invTrans._opId;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int
InventoryTransaction::compare(const utl::Object& rhs) const
{
    utl::cout << "In InventoryTransaction::compare ..." << utl::endlf;
    if (!rhs.isA(InventoryTransaction))
    {
        return Object::compare(rhs);
    }
    const InventoryTransaction& invTrans = (const InventoryTransaction&)rhs;
    utl::cout << "   " << toString() << utl::endlf;
    utl::cout << "   " << invTrans.toString() << utl::endlf;
    int res = utl::compare(_transTime, invTrans._transTime);
    if (res != 0)
        return res;
    res = utl::compare(_status, invTrans._status);
    if (res != 0)
        return res;
    res = utl::compare(_quantity, invTrans._quantity);
    if (res != 0)
        return res;
    res = utl::compare(_opType, invTrans._opType);
    if (res != 0)
        return res;
    res = utl::compare(_opId, invTrans._opId);
    return res;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
InventoryTransaction::serialize(Stream& stream, uint_t io, uint_t mode)
{
    utl::serialize(_transTime, stream, io);
    utl::serialize((uint_t&)_status, stream, io);
    utl::serialize(_quantity, stream, io);
    utl::serialize((uint_t&)_opType, stream, io);
    utl::serialize(_opId, stream, io);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

String
InventoryTransaction::toString() const
{
    MemStream str;
    str << "invTrans:"
        << ", tranTime:" << Time(_transTime).toString() << ", status:" << _status
        << ", quantity:" << _quantity << ", opType:" << _opType << ", opId:" << _opId << '\0';
    return String((char*)str.get());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
InventoryTransaction::init()
{
    _transTime = -1;
    _status = transstatus_undefined;
    _quantity = 0;
    _opType = transoptype_undefined;
    _opId = uint_t_max;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
InventoryTransaction::deInit()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////

MRP_NS_END;
