#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <libutl/Object.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

MPS_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/** MPSitemPeriod status. */
enum mpsitemperiod_status_t
{
    periodstatus_unplanned = 0, /**< not planned */
    periodstatus_planned = 1,   /** planned by MPS system */
    periodstatus_confirmed = 2, /** confirmed by user */
    periodstatus_undefined
};

////////////////////////////////////////////////////////////////////////////////////////////////////

class MPSitem;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Every MPS item's MPS period and all kinds of quantities of the period.

   \author Joe Zhou
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class MPSitemPeriod : public utl::Object
{
    UTL_CLASS_DECL(MPSitemPeriod, utl::Object);

public:
    /** Constructor. */
    MPSitemPeriod(MPSitem* item,
                  uint_t id,
                  time_t startTime,
                  time_t endTime,
                  uint_t forecast,
                  uint_t customerOrder,
                  utl::int_t pab,
                  uint_t mpsQuantity,
                  uint_t atp,
                  mpsitemperiod_status_t status,
                  uint_t onHand = uint_t_max);

    virtual void copy(const utl::Object& rhs);

    virtual void
    serialize(utl::Stream& stream, uint_t io, uint_t mode = utl::ser_default);

    /// \name Accessors
    //@{
    /** Item. */
    const MPSitem*
    item() const
    {
        return _item;
    }

    /** ID. */
    uint_t
    id() const
    {
        return _id;
    }

    /** ID. */
    uint_t&
    id()
    {
        return _id;
    }

    /** Period start time. */
    time_t
    startTime() const
    {
        return _startTime;
    }

    /** Period start time. */
    time_t&
    startTime()
    {
        return _startTime;
    }

    /** Period end time. */
    time_t
    endTime() const
    {
        return _endTime;
    }

    /** Period end time. */
    time_t&
    endTime()
    {
        return _endTime;
    }

    /** Forecast quantity. */
    uint_t
    forecast() const
    {
        return _forecast;
    }

    /** Forecast quantity. */
    uint_t&
    forecast()
    {
        return _forecast;
    }

    /** Customer order quantity. */
    uint_t
    customerOrder() const
    {
        return _customerOrder;
    }

    /** Customer order quantity. */
    uint_t&
    customerOrder()
    {
        return _customerOrder;
    }

    /** Projected available balance. */
    utl::int_t
    pab() const
    {
        return _pab;
    }

    /** Projected available balance. */
    utl::int_t&
    pab()
    {
        return _pab;
    }

    /** MPS quantity. */
    uint_t
    mpsQuantity() const
    {
        return _mpsQuantity;
    }

    /** MPS quantity. */
    uint_t&
    mpsQuantity()
    {
        return _mpsQuantity;
    }

    /** Available to promise. */
    uint_t
    atp() const
    {
        return _atp;
    }

    /** Available to promise. */
    uint_t&
    atp()
    {
        return _atp;
    }

    /** Quantity on hand. */
    uint_t
    onHand() const
    {
        return _onHand;
    }

    /** Quantity on hand. */
    uint_t&
    onHand()
    {
        return _onHand;
    }

    /** Status. */
    mpsitemperiod_status_t
    status() const
    {
        return _status;
    }

    /** Status. */
    mpsitemperiod_status_t&
    status()
    {
        return _status;
    }

    MPSitemPeriod*&
    prev()
    {
        return _prev;
    }

    MPSitemPeriod*&
    next()
    {
        return _next;
    }

    double
    opportunityCost() const
    {
        return _opportunityCost;
    }

    double&
    opportunityCost()
    {
        return _opportunityCost;
    }

    lut::period_t
    opportunityCostPeriod() const
    {
        return _opportunityCostPeriod;
    }

    lut::period_t&
    opportunityCostPeriod()
    {
        return _opportunityCostPeriod;
    }

    double
    latenessCost() const
    {
        return _latenessCost;
    }

    double&
    latenessCost()
    {
        return _latenessCost;
    }

    lut::period_t
    latenessCostPeriod() const
    {
        return _latenessCostPeriod;
    }

    lut::period_t&
    latenessCostPeriod()
    {
        return _latenessCostPeriod;
    }
    //@}
    /** Reset MPS item. */
    void setItem(MPSitem* item, bool owner = false);

    // Calculate _pab and _mpsQuantity
    void calculate1();

    // Calculate _atp
    void calculate2();

    String toString() const;

private:
    void init();
    void deInit();

private:
    MPSitem* _item;
    uint_t _id;
    time_t _startTime;
    time_t _endTime;
    uint_t _forecast;
    uint_t _customerOrder;
    utl::int_t _pab; // Projected available balance
    uint_t _mpsQuantity;
    uint_t _atp; // Available to promise
    uint_t _onHand;
    mpsitemperiod_status_t _status;

    bool _itemOwner; // only used for removing init dummy _item object

    MPSitemPeriod* _prev;
    MPSitemPeriod* _next;

    double _opportunityCost;
    lut::period_t _opportunityCostPeriod;
    double _latenessCost;
    lut::period_t _latenessCostPeriod;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

/** Order MPSitemPeriod  objects by Id. */
struct MPSitemPeriodOrderingIncId
    : public std::binary_function<MPSitemPeriod*, MPSitemPeriod*, bool>
{
    bool operator()(const MPSitemPeriod* lhs, const MPSitemPeriod* rhs) const;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

/** Order MPSitemPeriod objects by both ItemId and Id. */
struct MPSitemPeriodOrderingIncItemId
    : public std::binary_function<MPSitemPeriod*, MPSitemPeriod*, bool>
{
    bool operator()(const MPSitemPeriod* lhs, const MPSitemPeriod* rhs) const;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

/** Order MPSitemPeriod objects by _startTime. */
struct MPSitemPeriodOrderingIncST
    : public std::binary_function<MPSitemPeriod*, MPSitemPeriod*, bool>
{
    bool operator()(const MPSitemPeriod* lhs, const MPSitemPeriod* rhs) const;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

typedef std::set<MPSitemPeriod*, MPSitemPeriodOrderingIncId> mpsitemperiod_set_id_t;
typedef std::set<MPSitemPeriod*, MPSitemPeriodOrderingIncItemId> mpsitemperiod_set_itemid_t;
typedef std::set<MPSitemPeriod*, MPSitemPeriodOrderingIncST> mpsitemperiod_set_st_t;
typedef std::vector<MPSitemPeriod*> mpsitemperiod_vector_t;

////////////////////////////////////////////////////////////////////////////////////////////////////

MPS_NS_END;
