#ifndef MRP_WORKORDER_H
#define MRP_WORKORDER_H

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <libutl/String.h>
#include <mrp/ManufactureItem.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

MRP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/** WorkOrder status. */
enum workorder_status_t
{
    wostatus_unplanned = 0, /**< not planned */
    wostatus_planned = 1,   /**< planned by MRP system */
    wostatus_confirmed = 2, /**< confirmed by user */
    wostatus_undefined
};

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Work order.

   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class WorkOrder : public utl::Object
{
    UTL_CLASS_DECL(WorkOrder);

public:
    virtual void copy(const utl::Object& rhs);

    virtual int compare(const utl::Object& rhs) const;

    virtual void
    serialize(utl::Stream& stream, utl::uint_t io, utl::uint_t mode = utl::ser_default);

    /// \name Accessors
    //@{
    /** Id. */
    utl::uint_t
    id() const
    {
        return _id;
    }

    /** Id. */
    utl::uint_t&
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
    const ManufactureItem*
    item() const
    {
        return _item;
    }

    /** Item id. */
    ManufactureItem*
    item()
    {
        return _item;
    }

    /** MPS item period ids. */
    lut::uint_set_t
    mpsItemPeriodIds() const
    {
        return _mpsItemPeriodIds;
    }

    /** MPS item period ids. */
    lut::uint_set_t&
    mpsItemPeriodIds()
    {
        return _mpsItemPeriodIds;
    }

    /** Quantity. */
    utl::uint_t
    quantity() const
    {
        return _quantity;
    }

    /** Quantity. */
    utl::uint_t&
    quantity()
    {
        return _quantity;
    }

    /** Scheduled start time. */
    time_t
    releaseTime() const
    {
        return _releaseTime;
    }

    /** Scheduled start time. */
    time_t&
    releaseTime()
    {
        return _releaseTime;
    }

    /** Due time. */
    time_t
    dueTime() const
    {
        return _dueTime;
    }

    /** Due time. */
    time_t&
    dueTime()
    {
        return _dueTime;
    }

    /** Get the status. */
    workorder_status_t
    status() const
    {
        return _status;
    }

    /** Get the status. */
    workorder_status_t&
    status()
    {
        return _status;
    }

    /** Opportunity cost. */
    double
    opportunityCost() const
    {
        return _opportunityCost;
    }

    /** Opportunity cost. */
    double&
    opportunityCost()
    {
        return _opportunityCost;
    }

    /** Opportunity cost period. */
    lut::period_t
    opportunityCostPeriod() const
    {
        return _opportunityCostPeriod;
    }

    /** Opportunity cost period. */
    lut::period_t&
    opportunityCostPeriod()
    {
        return _opportunityCostPeriod;
    }

    /** Lateness cost. */
    double
    latenessCost() const
    {
        return _latenessCost;
    }

    /** Lateness cost. */
    double&
    latenessCost()
    {
        return _latenessCost;
    }

    /** Lateness cost period. */
    lut::period_t
    latenessCostPeriod() const
    {
        return _latenessCostPeriod;
    }

    /** Lateness cost period. */
    lut::period_t&
    latenessCostPeriod()
    {
        return _latenessCostPeriod;
    }

    /** Inventory cost. */
    double
    inventoryCost() const
    {
        return _inventoryCost;
    }

    /** Inventory cost. */
    double&
    inventoryCost()
    {
        return _inventoryCost;
    }

    /** Inventory cost period. */
    lut::period_t
    inventoryCostPeriod() const
    {
        return _inventoryCostPeriod;
    }

    /** Inventory cost period. */
    lut::period_t&
    inventoryCostPeriod()
    {
        return _inventoryCostPeriod;
    }
    //@}

    /** reset item. */
    void setItem(ManufactureItem* item, bool owner = false);

    utl::String toString() const;

private:
    void init();
    void deInit();

private:
    utl::uint_t _id;
    std::string _name;
    ManufactureItem* _item;
    lut::uint_set_t _mpsItemPeriodIds;
    bool _itemOwner;
    utl::uint_t _quantity;
    time_t _releaseTime;
    time_t _dueTime;
    workorder_status_t _status;
    double _opportunityCost;
    lut::period_t _opportunityCostPeriod;
    double _latenessCost;
    lut::period_t _latenessCostPeriod;
    double _inventoryCost;
    lut::period_t _inventoryCostPeriod;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

struct WorkOrderIdOrdering : public std::binary_function<WorkOrder*, WorkOrder*, bool>
{
    bool
    operator()(const WorkOrder* lhs, const WorkOrder* rhs) const
    {
        return (lhs->id() < rhs->id());
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////

struct WorkOrderDueTimeOrdering : public std::binary_function<WorkOrder*, WorkOrder*, bool>
{
    bool
    operator()(const WorkOrder* lhs, const WorkOrder* rhs) const
    {
        return (lhs->dueTime() < rhs->dueTime());
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////

typedef std::set<WorkOrder*, WorkOrderIdOrdering> workorder_set_id_t;
typedef std::set<WorkOrder*, WorkOrderDueTimeOrdering> workorder_set_duetime_t;

////////////////////////////////////////////////////////////////////////////////////////////////////

MRP_NS_END;

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
