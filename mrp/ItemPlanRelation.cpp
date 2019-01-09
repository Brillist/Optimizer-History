#include "libmrp.h"
#include <libutl/BufferedFDstream.h>
#include <libutl/Bool.h>
#include "ManufactureItem.h"
#include "ProcessPlan.h"
#include "ItemPlanRelation.h"

//////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;

//////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(mrp::ItemPlanRelation, utl::Object);

//////////////////////////////////////////////////////////////////////////////

MRP_NS_BEGIN;

/////////////////////////////////////////////////////////////////////////////

bool
PlanOrderingDecPref::operator()(
    const ItemPlanRelation* lhs,
    const ItemPlanRelation* rhs) const
{
    if (lhs->preference() == rhs->preference())
        return (lhs->plan()->id() < rhs->plan()->id());
    return (lhs->preference() > rhs->preference());
}

/////////////////////////////////////////////////////////////////////////////

ItemPlanRelation::ItemPlanRelation(
    ManufactureItem* item,
    ProcessPlan* plan,
    uint_t preference)
{
    init();
    _item = item;
    _plan = plan;
    _preference = preference;
}

/////////////////////////////////////////////////////////////////////////////

void
ItemPlanRelation::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(ItemPlanRelation));
    const ItemPlanRelation& ipr = (const ItemPlanRelation&)rhs;
    _itemOwner = ipr._itemOwner;
    if (_itemOwner)
        _item = utl::clone(ipr._item);
    else
        _item = ipr._item;
    _planOwner = ipr._planOwner;
    if (_planOwner)
        _plan = utl::clone(ipr._plan);
    else
        _plan = ipr._plan;
    _preference = ipr._preference;
}

//////////////////////////////////////////////////////////////////////////////

void
ItemPlanRelation::serialize(Stream& stream, uint_t io, uint_t mode)
{
    utl::serialize(_item->id(), stream, io);
    utl::serialize(_plan->id(), stream, io);
    utl::serialize(_preference, stream, io);
}

//////////////////////////////////////////////////////////////////////////////

void
ItemPlanRelation::setItem(ManufactureItem* item, bool owner)
{
    if (_itemOwner) delete _item;
    _item = item;
    _itemOwner = owner;
}

//////////////////////////////////////////////////////////////////////////////

void
ItemPlanRelation::setPlan(ProcessPlan* plan, bool owner)
{
    if (_planOwner) delete _plan;
    _plan = plan;
    _planOwner = owner;
}

//////////////////////////////////////////////////////////////////////////////

String
ItemPlanRelation::toString() const
{
    MemStream str;
    str << "ItemPlanRelation:"
        << ", item:" << _item->id()
        << ", plan:" << _plan->id()
        << ", preference:" << _preference
        << ", itemOwner:" << Bool(_itemOwner).toString()
        << ", planOwner:" << Bool(_planOwner).toString()
        << '\0';
    return String((char*)str.get());
}

//////////////////////////////////////////////////////////////////////////////

void
ItemPlanRelation::init()
{
    _item = new ManufactureItem();
    _plan = new ProcessPlan();
    _preference = uint_t_max;
    _itemOwner = true;
    _planOwner = true;
}

//////////////////////////////////////////////////////////////////////////////

void
ItemPlanRelation::deInit()
{
    setItem(nullptr);
    setPlan(nullptr);
}

//////////////////////////////////////////////////////////////////////////////

MRP_NS_END;
