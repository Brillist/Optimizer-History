#ifndef MRP_INVENTORYINTERVAL_H
#define MRP_INVENTORYINTERVAL_H

//////////////////////////////////////////////////////////////////////////////

#include <libutl/String.h>
#include <cse/JobOp.h>
#include <mrp/InventoryTransaction.h>
#include <mrp/PurchaseOrder.h>

//////////////////////////////////////////////////////////////////////////////

MRP_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

/**
   Inventory Interval

   An inventory interval represents a time period of an inventory profile, 
   in which the available capacity is same. Often an new interval is created or
   merged when an activity is added to or removed from the inventory profile 
   due to capacity level changes. So the length of an inventory interval is
   not fixed. (This is different from Inventory Record Period.)

   \author Joe Zhou
*/

//////////////////////////////////////////////////////////////////////////////

class InventoryInterval : public lut::RBtreeNode
{
    UTL_CLASS_DECL(InventoryInterval);
public:
    /** Constructors. */
    InventoryInterval(InventoryTransaction* trans);

    InventoryInterval(cse::JobOp* op);

    InventoryInterval(PurchaseOrder* po);

    virtual void copy(const utl::Object& rhs);

    virtual int compare(const utl::Object& rhs) const;

    virtual void serialize(
        utl::Stream& stream,
        utl::uint_t io,
        utl::uint_t mode = utl::ser_default);

    /// \name Accessors
    //@{
    /** Period start time. */
    time_t startTime() const
    { return _startTime; }

    /** Period start time. */
    time_t& startTime()
    { return _startTime; }

    /** Period end time. */
    time_t endTime() const
    { return _endTime; }

    /** Period end time. */
    time_t& endTime()
    { return _endTime;}

    /** Interval capacity. */
    utl::int_t cap() const
    { return _cap; }

    /** Interval capacity. */
    utl::int_t& cap()
    { return _cap; }

    /** Branch net capcity. */
    utl::int_t net() const
    { return _net; }

    /** Branch net capacity. */
    utl::int_t& net()
    { return _net; }

    /** Branch debit capacity. */
    utl::int_t debit() const
    { return _debit; }

    /** Branch debit capacity. */
    utl::int_t& debit() 
    { return _debit; }

    /** JobOps. */
    const cse::jobop_set_id_t& ops() const
    { return _ops; }

    /** Purchase orders. */
    const purchaseorder_set_id_t& pos() const
    { return _pos; }
    //@}

    void addJobOp(cse::JobOp* op)
    { _ops.insert(op); }

    void addPO(PurchaseOrder* po)
    { _pos.insert(po); }

    void dump(utl::Stream& os, utl::uint_t level) const;

    utl::String toString() const;
private:
    void init();
    void deInit();
private:
    time_t _startTime;
    time_t _endTime;
    utl::int_t _cap;
    utl::int_t _net;
    utl::int_t _debit;
    cse::jobop_set_id_t _ops;
    purchaseorder_set_id_t _pos;
};

//////////////////////////////////////////////////////////////////////////////

struct InventoryIntervalOrderingIncST 
    : public std::binary_function<
    InventoryInterval*, 
    InventoryInterval*, 
    bool>
{
    bool operator()(InventoryInterval* lhs, InventoryInterval* rhs) const
    {
        return (lhs->startTime() < rhs->startTime());
    }
};

//////////////////////////////////////////////////////////////////////////////

typedef std::set<InventoryInterval*, 
    InventoryIntervalOrderingIncST> inventoryinterval_set_st_t;

//////////////////////////////////////////////////////////////////////////////

MRP_NS_END;

//////////////////////////////////////////////////////////////////////////////

#endif
