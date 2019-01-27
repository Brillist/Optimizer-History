#ifndef MRP_ITEM_H
#define MRP_ITEM_H

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <libutl/String.h>
#include <mrp/BOM.h>
#include <mrp/InventoryTransaction.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

MRP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

// use forward declaration to avoid a circular dependency
class InventoryRecord;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Item (abstract).

   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class Item : public utl::Object
{
    UTL_CLASS_DECL_ABC(Item, utl::Object);

public:
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

    /** MPS-item flag. */
    bool
    MPSitem() const
    {
        return _MPSitem;
    }

    /** MPS-item flag. */
    bool&
    MPSitem()
    {
        return _MPSitem;
    }

    /** BOMs. */
    const bom_vector_t&
    boms() const
    {
        return _boms;
    }

    /** BOMs. */
    bom_vector_t&
    boms()
    {
        return _boms;
    }

    /** Inventory Record. */
    const InventoryRecord*
    inventoryRecord() const
    {
        return _inventoryRecord;
    }

    /** Inventory Record. */
    InventoryRecord*&
    inventoryRecord()
    {
        return _inventoryRecord;
    }
    //@}

    uint_t checkAndConsumeInventory(inv_transaction_op_type_t opType,
                                         uint_t opId,
                                         time_t t,
                                         uint_t reqQuantity,
                                         inv_transaction_status_t status);

    void consumeInventory(inv_transaction_op_type_t opType,
                          uint_t opId,
                          time_t t,
                          uint_t reqQuantity,
                          inv_transaction_status_t status);

    void produceInventory(inv_transaction_op_type_t opType,
                          uint_t opId,
                          time_t t,
                          uint_t quantity,
                          inv_transaction_status_t status);

    String toString() const;

private:
    void init();
    void deInit();

private:
    uint_t _id;
    std::string _name;
    bool _MPSitem;
    bom_vector_t _boms;
    InventoryRecord* _inventoryRecord;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

/** Order item objects by id. */
struct ItemOrderingIncId : public std::binary_function<Item*, Item*, bool>
{
    bool operator()(const Item* lhs, const Item* rhs) const;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

typedef std::set<Item*, ItemOrderingIncId> item_set_id_t;

////////////////////////////////////////////////////////////////////////////////////////////////////

MRP_NS_END;

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
