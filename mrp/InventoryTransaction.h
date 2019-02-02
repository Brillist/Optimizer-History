#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

/* #include <libutl/String.h> */

////////////////////////////////////////////////////////////////////////////////////////////////////

MRP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

enum inv_transaction_status_t
{
    transstatus_planned = 0,
    transstatus_confirmed = 1,
    transstatus_complete = 2,
    transstatus_undefined
};

////////////////////////////////////////////////////////////////////////////////////////////////////

enum inv_transaction_op_type_t
{
    transoptype_workorder = 0,
    transoptype_jobgroup = 1,
    transoptype_job = 2,
    transoptype_purchaseorder = 3,
    transoptype_undefined
};

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Inventory Record Period

   The MRP inventory record is consisted of a list of inventory record period.
   The time length of an inventory record period is usually a week, but it can
   be a day or a month too.

   \author Joe Zhou
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class InventoryTransaction : public utl::Object
{
    UTL_CLASS_DECL(InventoryTransaction, utl::Object);

public:
    InventoryTransaction(time_t transTime,
                         inv_transaction_status_t status,
                         utl::int_t quantity,
                         inv_transaction_op_type_t opType,
                         uint_t opId)
    {
        _transTime = transTime;
        _status = status;
        _quantity = quantity;
        _opType = opType;
        _opId = opId;
    }

    virtual void copy(const utl::Object& rhs);

    virtual int compare(const utl::Object& rhs) const;

    virtual void serialize(utl::Stream& stream, uint_t io, uint_t mode = utl::ser_default);

    /// \name Accessors
    //@{
    /** Period start time. */
    time_t
    transTime() const
    {
        return _transTime;
    }

    /** Period start time. */
    time_t&
    transTime()
    {
        return _transTime;
    }

    /** Status. */
    inv_transaction_status_t
    status() const
    {
        return _status;
    }

    /** Status. */
    inv_transaction_status_t&
    status()
    {
        return _status;
    }

    /** Quantity. */
    utl::int_t
    quantity() const
    {
        return _quantity;
    }

    /** Quantity. */
    utl::int_t&
    quantity()
    {
        return _quantity;
    }

    /** Operation Type. */
    inv_transaction_op_type_t
    opType() const
    {
        return _opType;
    }

    /** Operation Type. */
    inv_transaction_op_type_t&
    opType()
    {
        return _opType;
    }

    /** Operation Id. */
    uint_t
    opId() const
    {
        return _opId;
    }

    /** Operation Id. */
    uint_t&
    opId()
    {
        return _opId;
    }
    //@}

    String toString() const;

private:
    void init();
    void deInit();

private:
    time_t _transTime;
    inv_transaction_status_t _status;
    utl::int_t _quantity;
    inv_transaction_op_type_t _opType;
    uint_t _opId;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

struct InventoryTransactionOrderingIncT
{
    bool operator()(InventoryTransaction* lhs, InventoryTransaction* rhs) const;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

typedef std::set<InventoryTransaction*, InventoryTransactionOrderingIncT>
    inventorytransaction_set_time_t;

////////////////////////////////////////////////////////////////////////////////////////////////////

MRP_NS_END;
