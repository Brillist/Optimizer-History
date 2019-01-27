#ifndef MRP_PURCHASEORDER_H
#define MRP_PURCHASEORDER_H

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <libutl/String.h>
#include <mrp/PurchaseItem.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

MRP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/** Purchase status. */
enum purchaseorder_status_t
{
    postatus_planned = 0, /**< planned for the future */
    postatus_confirmed = 1,
    postatus_started = 2,
    postatus_complete = 3,
    postatus_undefined
};

////////////////////////////////////////////////////////////////////////////////////////////////////

/** Requester types. */
enum requester_type_t
{
    requestertype_job = 0,
    requestertype_jobgroup = 1,
    requestertype_undefined
};

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Purchase Order

   \author Joe Zhou
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class PurchaseOrder : public utl::Object
{
    UTL_CLASS_DECL(PurchaseOrder, utl::Object);

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

    /** Item id. */
    const PurchaseItem*
    item() const
    {
        return _item;
    }

    /** Item id. */
    PurchaseItem*&
    item()
    {
        return _item;
    }

    /** Quantity. */
    uint_t
    quantity() const
    {
        return _quantity;
    }

    /** Quantity. */
    uint_t&
    quantity()
    {
        return _quantity;
    }

    /** Order date. */
    time_t
    orderDate() const
    {
        return _orderDate;
    }

    /** Order date. */
    time_t&
    orderDate()
    {
        return _orderDate;
    }

    /** Due time. */
    time_t
    receiveDate() const
    {
        return _receiveDate;
    }

    /** Due time. */
    time_t&
    receiveDate()
    {
        return _receiveDate;
    }

    /** Get the status. */
    purchaseorder_status_t
    status() const
    {
        return _status;
    }

    /** Get the status. */
    purchaseorder_status_t&
    status()
    {
        return _status;
    }

    /** Get the requester id. */
    uint_t
    requesterId() const
    {
        return _requesterId;
    }

    /** Get the requester id. */
    uint_t&
    requesterId()
    {
        return _requesterId;
    }

    /** Get the requester type. */
    requester_type_t
    requesterType() const
    {
        return _requesterType;
    }

    /** Get the requester type. */
    requester_type_t&
    requesterType()
    {
        return _requesterType;
    }
    //@}

    /** reset item. */
    void setItem(PurchaseItem* item, bool owner = false);

    String toString() const;

private:
    void init();
    void deInit();

private:
    uint_t _id;
    std::string _name;
    PurchaseItem* _item;
    bool _itemOwner;
    purchaseorder_status_t _status;
    uint_t _quantity;
    time_t _orderDate;
    time_t _receiveDate;
    uint_t _requesterId;
    requester_type_t _requesterType;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

struct PurchaseOrderIdOrdering : public std::binary_function<PurchaseOrder*, PurchaseOrder*, bool>
{
    bool operator()(const PurchaseOrder* lhs, const PurchaseOrder* rhs) const;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

struct PurchaseOrderOdateOrdering
    : public std::binary_function<PurchaseOrder*, PurchaseOrder*, bool>
{
    bool operator()(const PurchaseOrder* lhs, const PurchaseOrder* rhs) const;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

typedef std::set<PurchaseOrder*, PurchaseOrderIdOrdering> purchaseorder_set_id_t;

////////////////////////////////////////////////////////////////////////////////////////////////////

MRP_NS_END;

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
