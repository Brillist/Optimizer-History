#include "libmrp.h"
#include <libutl/MemStream.h>
#include <libutl/Float.h>
#include <libutl/Time.h>
#include "PurchaseOrder.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(mrp::PurchaseOrder);

////////////////////////////////////////////////////////////////////////////////////////////////////

MRP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

bool
PurchaseOrderIdOrdering::operator()(const PurchaseOrder* lhs, const PurchaseOrder* rhs) const
{
    return (lhs->id() < rhs->id());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool
PurchaseOrderOdateOrdering::operator()(const PurchaseOrder* lhs, const PurchaseOrder* rhs) const
{
    return (lhs->orderDate() < rhs->orderDate());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
PurchaseOrder::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(PurchaseOrder));
    const PurchaseOrder& po = (const PurchaseOrder&)rhs;
    _id = po._id;
    _name = po._name;
    _itemOwner = po._itemOwner;
    if (_itemOwner)
    {
        _item = utl::clone(po._item);
    }
    else
    {
        _item = po._item;
    }
    _quantity = po._quantity;
    _orderDate = po._orderDate;
    _receiveDate = po._receiveDate;
    _status = po._status;
    _requesterId = po._requesterId;
    _requesterType = po._requesterType;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int
PurchaseOrder::compare(const utl::Object& rhs) const
{
    if (!rhs.isA(PurchaseOrder))
    {
        return Object::compare(rhs);
    }
    const PurchaseOrder& po = (const PurchaseOrder&)rhs;
    return utl::compare(_id, po._id);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
PurchaseOrder::serialize(Stream& stream, uint_t io, uint_t mode)
{
    utl::serialize(_id, stream, io);
    lut::serialize(_name, stream, io);
    utl::serialize((uint_t&)_status, stream, io);
    utl::serialize(_item->id(), stream, io);
    utl::serialize(_quantity, stream, io);
    lut::serialize(_orderDate, stream, io);
    lut::serialize(_receiveDate, stream, io);
    utl::serialize(_requesterId, stream, io);
    utl::serialize((uint_t&)_requesterType, stream, io);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
PurchaseOrder::setItem(PurchaseItem* item, bool owner)
{
    if (_itemOwner)
        delete _item;
    _item = item;
    _itemOwner = owner;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

String
PurchaseOrder::toString() const
{
    MemStream str;
    str << "po:"
        << _id
        //         << ", name:" << _name.c_str()
        << ", item:" << _item->id() << ", qnty:" << _quantity << ", status:" << _status
        << ", ordD:" << Time(_orderDate).toString("$yyyy/$m/$d $h:$nn")
        << ", rcvD:" << Time(_receiveDate).toString("$yyyy/$m/$d $h:$nn")
        << ", reqId:" << _requesterId << ", reqType:" << _requesterType << '\0';
    return String((char*)str.get());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
PurchaseOrder::init()
{
    _id = uint_t_max;
    _item = new PurchaseItem();
    _itemOwner = true;
    _quantity = uint_t_max;
    _orderDate = -1;
    _receiveDate = -1;
    _status = postatus_undefined;
    _requesterId = uint_t_max;
    _requesterType = requestertype_undefined;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
PurchaseOrder::deInit()
{
    setItem(nullptr);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

MRP_NS_END;
