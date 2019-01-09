#include "libmrp.h"
#include <libutl/MemStream.h>
#include <libutl/Time.h>
#include <mrp/InventoryRecord.h>
#include "Item.h"

//////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;

//////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL_ABC(mrp::Item, utl::Object);

//////////////////////////////////////////////////////////////////////////////

MRP_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////
bool
ItemOrderingIncId::operator()(const Item* lhs, const Item* rhs) const
{
    return (lhs->id() < rhs->id());
}

//////////////////////////////////////////////////////////////////////////////

void
Item::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(Item));
    const Item& item = (const Item&)rhs;
    _id = item._id;
    _name = item._name;
    _MPSitem = item._MPSitem;
    *_inventoryRecord = *item._inventoryRecord;
}

//////////////////////////////////////////////////////////////////////////////

int
Item::compare(const utl::Object& rhs) const
{
    if (!rhs.isA(Item))
    {
        return Object::compare(rhs);
    }
    const Item& it = (const Item&)rhs;
    return utl::compare(_id, it._id);
}

//////////////////////////////////////////////////////////////////////////////

void
Item::serialize(Stream& stream, uint_t io, uint_t mode)
{
    utl::serialize(_id, stream, io);
    lut::serialize(_name, stream, io);
    utl::serialize(_MPSitem, stream, io);
}

//////////////////////////////////////////////////////////////////////////////

uint_t Item::checkAndConsumeInventory(
    inv_transaction_op_type_t opType,
    uint_t opId,
    time_t t,
    uint_t reqQuantity,
    inv_transaction_status_t status)
{
    // deal with special cases
    if (t < 0) return reqQuantity;
    if (_inventoryRecord == nullptr) return reqQuantity;
    int_t availQuantity =
        _inventoryRecord->availableCapacity(t);
    if (availQuantity <= 0) return reqQuantity;

    int_t diffQuantity = reqQuantity - availQuantity;
    if (diffQuantity <= 0)
    {
        // more than enough available quantity
        InventoryTransaction* trans = new InventoryTransaction(
            t, status, -1 * (int_t)reqQuantity, opType, opId);
        _inventoryRecord->add(trans);
        return 0;
    }
    else
    {
        // not enough available quantity
        InventoryTransaction* trans = new InventoryTransaction(
            t, status, -1 * (int_t)availQuantity, opType, opId);
        _inventoryRecord->add(trans);
        return diffQuantity;
    }
}

//////////////////////////////////////////////////////////////////////////////

void Item::consumeInventory(
    inv_transaction_op_type_t opType,
    uint_t opId,
    time_t t,
    uint_t reqQuantity,
    inv_transaction_status_t status)
{
    if (_inventoryRecord == nullptr) return;
    if (t < 0)
    {
        String* str = new String();
        *str = "Cannot consume inventory at a negative time, t = ";
        *str += Time(t).toString();
        *str += ", opId" + Uint(opId).toString();
        throw clp::FailEx(str);
    }
    InventoryTransaction* trans = new InventoryTransaction(
        t, status, -1 * (int_t)reqQuantity, opType, opId);
    _inventoryRecord->add(trans);
}

//////////////////////////////////////////////////////////////////////////////

void Item::produceInventory(
    inv_transaction_op_type_t opType,
    uint_t opId,
    time_t t,
    uint_t quantity,
    inv_transaction_status_t status)
{
    ASSERTD(quantity >= 0);
    // deal with special cases
    if (t < 0 || quantity ==0 || _inventoryRecord == nullptr)
        return;

    InventoryTransaction* trans = new InventoryTransaction(
        t, status, quantity, opType, opId);
    _inventoryRecord->add(trans);
}

//////////////////////////////////////////////////////////////////////////////

String Item::toString() const
{
    MemStream str;
    str << "item:" << _id
        << ", name:" << _name.c_str()
        << ", MPSitem:" << _MPSitem;
    for (bom_vector_t::const_iterator it = _boms.begin();
         it != _boms.end(); it++)
    {
        str << '\n' << "   "
            << (*it)->toString();
    }
//     if (_ir->id() != uint_t_max)
//     {
//         str << '\n' << "   " << _ir->toString();
//     }
    str << '\0';
    return String((char*)str.get());
}

//////////////////////////////////////////////////////////////////////////////

void Item::init()
{
    _id = utl::uint_t_max;
    _MPSitem = false;
    _inventoryRecord = nullptr;
    // tempory code
//     _ir = new InventoryRecord();
}

//////////////////////////////////////////////////////////////////////////////

void Item::deInit()
{}

//////////////////////////////////////////////////////////////////////////////

MRP_NS_END;
