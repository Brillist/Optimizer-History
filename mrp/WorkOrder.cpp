#include "libmrp.h"
#include <libutl/MemStream.h>
#include <libutl/Float.h>
#include <libutl/Time.h>
#include "WorkOrder.h"

//////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;

//////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(mrp::WorkOrder, utl::Object);

//////////////////////////////////////////////////////////////////////////////

MRP_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////

// bool WorkOrderIdOrdering::operator()(
//     const WorkOrder* lhs,
//     const WorkOrder* rhs) const
// {
//     return (lhs->id() < rhs->id());
// }

//////////////////////////////////////////////////////////////////////////////

// bool WorkOrderDueTimeOrdering::operator()(
//     const WorkOrder* lhs,
//     const WorkOrder* rhs) const
// {
//     return (lhs->dueTime() < rhs->dueTime());
// }

//////////////////////////////////////////////////////////////////////////////

void
WorkOrder::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(WorkOrder));
    const WorkOrder& wo = (const WorkOrder&)rhs;
    _id = wo._id;
    _name = wo._name;
    _itemOwner = wo._itemOwner;
    if (_itemOwner)
    {
        _item = utl::clone(wo._item);
    }
    else
    {
        _item = wo._item;
    }
    _mpsItemPeriodIds = wo._mpsItemPeriodIds;
    _quantity = wo._quantity;
    _releaseTime = wo._releaseTime;
    _dueTime = wo._dueTime;
    _status = wo._status;
    _opportunityCost = wo._opportunityCost;
    _opportunityCostPeriod = wo._opportunityCostPeriod;
    _latenessCost = wo._latenessCost;
    _latenessCostPeriod = wo._latenessCostPeriod;
    _inventoryCost = wo._inventoryCost;
    _inventoryCostPeriod = wo._inventoryCostPeriod;
}

//////////////////////////////////////////////////////////////////////////////

int
WorkOrder::compare(const utl::Object& rhs) const
{
    if (!rhs.isA(WorkOrder))
    {
        return Object::compare(rhs);
    }
    const WorkOrder& wo = (const WorkOrder&)rhs;
    return utl::compare(_id, wo._id);
}

//////////////////////////////////////////////////////////////////////////////

void
WorkOrder::serialize(Stream& stream, uint_t io, uint_t mode)
{
    utl::serialize(_id, stream, io);
    lut::serialize(_name, stream, io);
    utl::serialize(_item->id(), stream, io);
    lut::serialize<uint_t>(_mpsItemPeriodIds, stream, io);
    utl::serialize(_quantity, stream, io);
    lut::serialize(_releaseTime, stream, io);
    lut::serialize(_dueTime, stream, io);
    utl::serialize((uint_t&)_status, stream, io);
    utl::serialize(_opportunityCost, stream, io);
    utl::serialize((uint_t&)_opportunityCostPeriod, stream, io);
    utl::serialize(_latenessCost, stream, io);
    utl::serialize((uint_t&)_latenessCostPeriod, stream, io);
    utl::serialize(_inventoryCost, stream, io);
    utl::serialize((uint_t&)_inventoryCostPeriod, stream, io);
}

//////////////////////////////////////////////////////////////////////////////

void
WorkOrder::setItem(ManufactureItem* item, bool owner)
{
    if (_itemOwner) delete _item;
    _item = item;
    _itemOwner = owner;
}

//////////////////////////////////////////////////////////////////////////////

String
WorkOrder::toString() const
{
    MemStream str;
    str << "wo:" << _id
//         << ", name:" << _name.c_str()
        << ", item:" << _item->id()
        << ", qnty:" << _quantity
        << ", status:" << _status
        << ", rlsT:" << Time(_releaseTime).toString("$yyyy/$m/$d $h:$nn")
        << ", dueT:" << Time(_dueTime).toString("$yyyy/$m/$d $h:$nn")
        << ", opporC:" << Float(_opportunityCost).toString("precision:2")
        << "/" << _opportunityCostPeriod
        << ", lateC:" << Float(_latenessCost).toString("precision:2")
        << "/" << _latenessCostPeriod
        << ", ivntC:" << Float(_inventoryCost).toString("precision:2")
        << "/" << _inventoryCostPeriod
        << '\0';
    return String((char*)str.get());
}

//////////////////////////////////////////////////////////////////////////////

void
WorkOrder::init()
{
    _id = uint_t_max;
    _item = new ManufactureItem();
    _itemOwner = true;
    _quantity = uint_t_max;
    _releaseTime = -1;
    _dueTime = -1;
    _status = wostatus_undefined;
    _opportunityCost = 0.0;
    _opportunityCostPeriod = period_undefined;
    _latenessCost = 0.0;
    _latenessCostPeriod = period_undefined;
    _inventoryCost = 0.0;
    _inventoryCostPeriod = period_undefined;
}

//////////////////////////////////////////////////////////////////////////////

void
WorkOrder::deInit()
{
    setItem(nullptr);
}

//////////////////////////////////////////////////////////////////////////////

MRP_NS_END;
