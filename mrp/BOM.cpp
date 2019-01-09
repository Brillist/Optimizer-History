#include "libmrp.h"
#include <libutl/MemStream.h>
#include <mrp/Item.h>
#include "BOM.h"

//////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;

//////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(mrp::BOM, utl::Object);

//////////////////////////////////////////////////////////////////////////////

MRP_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////

void
BOM::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(BOM));
    const BOM& bom = (const BOM&)rhs;
    _itemOwner = bom._itemOwner;
    if (_itemOwner)
        _item = utl::clone(bom._item);
    else
        _item = bom._item;
    _childItemOwner = bom._childItemOwner;
    if (_childItemOwner)
        _childItem = utl::clone(bom._childItem);
    else
        _childItem = bom._childItem;
    _quantity = bom._quantity;
}

//////////////////////////////////////////////////////////////////////////////

void
BOM::serialize(Stream& stream, uint_t io, uint_t mode)
{
    utl::serialize(_item->id(), stream, io);
    utl::serialize(_childItem->id(), stream, io);
    utl::serialize(_quantity, stream, io);
}

//////////////////////////////////////////////////////////////////////////////

void
BOM::setItem(Item* item, bool owner)
{
    if (_itemOwner) delete _item;
    _item = item;
    _itemOwner = owner;
}

//////////////////////////////////////////////////////////////////////////////

void
BOM::setChildItem(Item* item, bool owner)
{
    if (_childItemOwner) delete _childItem;
    _childItem = item;
    _childItemOwner = owner;
}

//////////////////////////////////////////////////////////////////////////////

String
BOM::toString() const
{
    MemStream str;
    str << "BOM:"
        << " item:" << _item->id()
        << ", childItem:" << _childItem->id()
        << ", name:" << _childItem->name().c_str()
        << ", quantity:" << _quantity
        << '\0';
    return String((char*)str.get());
}

//////////////////////////////////////////////////////////////////////////////

void
BOM::init()
{
    _item = new Item();
    _childItem = new Item();
    _quantity = 0;
    _itemOwner = true;
    _childItemOwner = true;
}

//////////////////////////////////////////////////////////////////////////////

void
BOM::deInit()
{
    setItem(nullptr);
    setChildItem(nullptr);
}

//////////////////////////////////////////////////////////////////////////////

MRP_NS_END;
