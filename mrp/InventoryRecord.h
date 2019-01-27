#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <lut/RBtreeNode.h>
#include <mrp/InventoryTransaction.h>
#include <mrp/InventoryInterval.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

MRP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

enum inventory_period_t
{
    inventoryperiod_day = 0,
    inventoryperiod_week = 1,
    inventoryperiod_undefined
};

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Inventory Record

   The MRP inventory record  shows an item's planning factors, gross 
   requirements, scheduled receipts, project on-hand inventory, planned order 
   receipts, and planned order release.

   It is represented as a balanced tree with Inventory intervals as nodes.

   \author Joe Zhou
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class InventoryRecord : public lut::RBtree
{
    UTL_CLASS_DECL(InventoryRecord, lut::RBtree);

public:
    // initialize method used after all data are populated,
    // e.g. after serialization
    virtual void initialize();

    virtual void copy(const utl::Object& rhs);

    virtual int compare(const utl::Object& rhs) const;

    virtual void
    serialize(utl::Stream& stream, uint_t io, uint_t mode = utl::ser_default);

    /// \name Accessors
    //@{
    /** Id. */
    uint_t
    id() const
    {
        return _id;
    }

    /** Id. */
    uint_t&
    id()
    {
        return _id;
    }

    /** ItemId. */
    uint_t
    itemId() const
    {
        return _itemId;
    }

    /** ItemId. */
    uint_t&
    itemId()
    {
        return _itemId;
    }

    /** Name. */
    const std::string&
    name() const
    {
        return _name;
    }

    /** Name. */
    std::string&
    name()
    {
        return _name;
    }

    /** Period type. */
    inventory_period_t
    periodType() const
    {
        return _periodType;
    }

    /** Period type. */
    inventory_period_t&
    periodType()
    {
        return _periodType;
    }

    /** Number of periods. */
    uint_t
    numberOfPeriods() const
    {
        return _numberOfPeriods;
    }

    /** Number of periods. */
    uint_t&
    numberOfPeriods()
    {
        return _numberOfPeriods;
    }

    /** Safety stock. */
    uint_t
    safetyStock() const
    {
        return _safetyStock;
    }

    /** Safety stock. */
    uint_t&
    safetyStock()
    {
        return _safetyStock;
    }

    /** On-hand. */
    uint_t
    onHand() const
    {
        return _onHand;
    }

    /** On-hand. */
    uint_t&
    onHand()
    {
        return _onHand;
    }

    /** Start date. */
    time_t
    startDate() const
    {
        return _startDate;
    }

    /** Start date. */
    time_t&
    startDate()
    {
        return _startDate;
    }

    /** Inventory Record Periods. */
    const inventorytransaction_set_time_t&
    invTransactions() const
    {
        return _invTransactions;
    }

    /** Inventory Record Periods. */
    inventorytransaction_set_time_t&
    inventoryTransactions()
    {
        return _invTransactions;
    }

    /** Inventory Intervals. */
    const inventoryinterval_set_st_t&
    intervals() const
    {
        return _intervals;
    }

    /** Inventory Intervals. */
    inventoryinterval_set_st_t&
    intervals()
    {
        return _intervals;
    }
    //@}

    utl::int_t getNet(const InventoryInterval* intvl);

    utl::int_t getDebit(const InventoryInterval* intvl);

    void setNet(InventoryInterval* intvl);

    void setDebit(InventoryInterval* intvl);

    /** find available capacity at time t. */
    utl::int_t availableCapacity(time_t t);

    //** check whether there is enough cap for reqCap at time t. */
    bool
    checkAvailableCapacity(time_t t, uint_t reqCap)
    {
        return availableCapacity(t) >= (utl::int_t)reqCap;
    }

    /** find the earliest availbe time from which there is enough reqCap.*/
    time_t
    findEarliestAvailableTime(uint_t reqCap, time_t est = 0, time_t lst = utl::int_t_max)
    {
        return 0;
    }

    /** add an inventory transaction to the inventory record. */
    void add(InventoryTransaction* trans);

    /** add an op to the inventory record. */
    void add(cse::JobOp* op);

    /** add a purchase order to the inventory record. */
    void add(PurchaseOrder* po);

    // tempory code. it should be private
    void insertNode(InventoryInterval* intvl);
    void deleteNode(InventoryInterval* intvl);

    // 3 methods for debug
    String toString() const;

    void dumpTree() const;

    void checkTree();

private:
    /** some important inline methods. */
    utl::int_t netCapIncrement(InventoryInterval* intvl);

    utl::int_t minDebitCapacity(InventoryInterval* intvl, utl::int_t prevDebit);

    utl::int_t
    nodeAvailableCapacity(InventoryInterval* intvl, utl::int_t leftCap, utl::int_t rightCap);

    void propagateDebitChange(InventoryInterval* intvl);

    void propagateNetChange(InventoryInterval* intvl);

    void leftRotate(lut::RBtreeNode* node);

    void rightRotate(lut::RBtreeNode* node);

    void checkNode(InventoryInterval* node, uint_t numBlackNodes);

private:
    void init();
    void deInit();

private:
    uint_t _id;
    uint_t _itemId;
    std::string _name;
    inventory_period_t _periodType;
    uint_t _numberOfPeriods;
    uint_t _safetyStock;
    uint_t _onHand;
    time_t _startDate;
    inventorytransaction_set_time_t _invTransactions;
    inventoryinterval_set_st_t _intervals;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

struct InventoryRecordOrderingIncId
    : public std::binary_function<InventoryRecord*, InventoryRecord*, bool>
{
    bool
    operator()(InventoryRecord* lhs, InventoryRecord* rhs)
    {
        return (lhs->id() < rhs->id());
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////

typedef std::set<InventoryRecord*, InventoryRecordOrderingIncId> inventoryrecord_set_id_t;

////////////////////////////////////////////////////////////////////////////////////////////////////

MRP_NS_END;
