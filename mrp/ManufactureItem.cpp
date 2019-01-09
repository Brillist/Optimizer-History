#include "libmrp.h"
#include <libutl/BufferedFDstream.h>
#include <libutl/MemStream.h>
#include "ManufactureItem.h"

//////////////////////////////////////////////////////////////////////////////

#ifdef DEBUG
// #define DEBUG_UNIT
#endif

//////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;
CSE_NS_USE;

//////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(mrp::ManufactureItem, mrp::Item);

//////////////////////////////////////////////////////////////////////////////

MRP_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////

void
ManufactureItem::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(ManufactureItem));
    const ManufactureItem& mi = (const ManufactureItem&)rhs;
    Item::copy(mi);
    _batchSize = mi._batchSize;
    copySet(_itemPlans, mi._itemPlans);
}

//////////////////////////////////////////////////////////////////////////////

void
ManufactureItem::serialize(Stream& stream, utl::uint_t io, utl::uint_t)
{
    Item::serialize(stream, io);
    utl::serialize(_batchSize, stream, io);
}

//////////////////////////////////////////////////////////////////////////////

ProcessPlan*
ManufactureItem::preferredPlan()
{
    if (_itemPlans.size() == 0)
        return nullptr;
    ItemPlanRelation* itemPlan = *_itemPlans.begin();
    return itemPlan->plan();
}

//////////////////////////////////////////////////////////////////////////////

String
ManufactureItem::toString() const
{
    MemStream str;
    str << Item::toString()
        << '\n' << "   "
        << "itemType:Manufacture"
        << ", batchSize:" << _batchSize
        << ", #plans:" << _itemPlans.size();
    itemplan_set_t::iterator it;
    for (it = _itemPlans.begin(); it != _itemPlans.end();
         it++)
    {
        ItemPlanRelation* itemPlan = (*it);
        str << '\n' << "      "
            << itemPlan->toString();
    }
    str << '\0';
    return String((char*)str.get());
}

//////////////////////////////////////////////////////////////////////////////

void
ManufactureItem::init()
{
    _batchSize = uint_t_max;
}

//////////////////////////////////////////////////////////////////////////////

MRP_NS_END;
