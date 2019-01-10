#include "libmrp.h"
#include <libutl/MemStream.h>
#include <libutl/Float.h>
#include <libutl/Time.h>
#include "InventoryInterval.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;
CSE_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(mrp::InventoryInterval, utl::Object);

////////////////////////////////////////////////////////////////////////////////////////////////////

MRP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

InventoryInterval::InventoryInterval(InventoryTransaction* trans)
{
    init();
    _startTime = trans->transTime();
    _cap = trans->quantity();
    _net = _cap;
    _debit = _cap;
    // probably we need to remember the op too in the future
    // probably we need to create an inventoryOperation Class.
    // Joe, March 9, 2009
}

////////////////////////////////////////////////////////////////////////////////////////////////////

InventoryInterval::InventoryInterval(JobOp* op)
{
    init();
    //     _startTime = op->scheduledStartTime();
    // _cap =
}

////////////////////////////////////////////////////////////////////////////////////////////////////

InventoryInterval::InventoryInterval(PurchaseOrder* po)
{
    init();
    _startTime = po->receiveDate();
    _cap = -1 * po->quantity();
    _net = _cap;
    _debit = _cap;
    addPO(po);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
InventoryInterval::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(InventoryInterval));
    const InventoryInterval& ii = (const InventoryInterval&)rhs;
    _startTime = ii._startTime;
    _endTime = ii._endTime;
    _cap = ii._cap;
    _net = ii._net;
    _debit = ii._debit;
    _ops = ii._ops;
    _pos = ii._pos;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int
InventoryInterval::compare(const utl::Object& rhs) const
{
    if (!rhs.isA(InventoryInterval))
    {
        return Object::compare(rhs);
    }
    const InventoryInterval& ir = (const InventoryInterval&)rhs;
    return utl::compare(_startTime, ir._startTime);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
InventoryInterval::serialize(Stream& stream, uint_t io, uint_t mode)
{
    lut::serialize(_startTime, stream, io);
    lut::serialize(_endTime, stream, io);
    utl::serialize(_cap, stream, io);
    utl::serialize(_net, stream, io);
    utl::serialize(_debit, stream, io);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////

void
InventoryInterval::dump(utl::Stream& os, uint_t level) const
{
    String indent;
    for (uint_t i = 0; i < 2 * level; i++)
    {
        indent += ' ';
    }
    os << indent;
    //     os << "st:" << Time(_startTime).toString("$yyyy/$m/$d")
    //        << ", et:" << Time(_endTime).toString("$yyyy/$m/$d")
    if (this == left() && this == right())
    {
        os << "leaf" << utl::endl;
        return;
    }
    else
    {
        os << "st:" << _startTime << ", et:" << _endTime << ", cap:" << _cap << ", net:" << _net
           << ", debit:" << _debit << ", color:";
        if (color() == nodecolor_black)
            os << "black";
        else
            os << "red";
        os << utl::endl;
    }

    left()->dump(os, level + 1);
    right()->dump(os, level + 1);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

String
InventoryInterval::toString() const
{
    MemStream str;
    str << "st:" << Time(_startTime).toString("$yyyy/$m/$d") << "(" << _startTime << ")"
        << ", et:" << Time(_endTime).toString("$yyyy/$m/$d") << "(" << _endTime << ")"
        << ", cap:" << _cap << ", net:" << _net << ", debit:" << _debit << ", ops:";
    jobop_set_id_t::iterator it;
    for (it = _ops.begin(); it != _ops.end(); it++)
    {
        if (it != _ops.begin())
            str << ",";
        str << (*it)->id();
    }
    str << ", pos:";
    purchaseorder_set_id_t::iterator poIt;
    for (poIt = _pos.begin(); poIt != _pos.end(); poIt++)
    {
        if (poIt != _pos.begin())
            str << ",";
        str << (*poIt)->id();
    }
    str << '\0';
    return String((char*)str.get());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////

void
InventoryInterval::init()
{
    _startTime = 0;
    _endTime = int_t_max;
    _cap = 0;
    _net = 0;
    _debit = 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
InventoryInterval::deInit()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////

MRP_NS_END;
