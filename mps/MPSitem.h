#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <mrp/Item.h>
#include <mps/MPSitemPeriod.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

MPS_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   MPS item.

   \author Joe Zhou
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class MPSitem : public mrp::Item
{
    UTL_CLASS_DECL(MPSitem, mrp::Item);

public:
    virtual void copy(const utl::Object& rhs);

    virtual void
    serialize(utl::Stream& stream, uint_t io, uint_t mode = utl::ser_default);

    /// \name Accessors
    //@{
    /** Economic order quantity. */
    uint_t
    eoq() const
    {
        return _eoq;
    }

    /** Economic order quantity. */
    uint_t&
    eoq()
    {
        return _eoq;
    }

    /** Demand time fence. */
    time_t
    dtf() const
    {
        return _dtf;
    }

    /** Demand time fence. */
    time_t&
    dtf()
    {
        return _dtf;
    }

    /** Planning time fence. */
    time_t
    ptf() const
    {
        return _ptf;
    }

    /** Planning time fence. */
    time_t&
    ptf()
    {
        return _ptf;
    }

    /** Planning horizon. */
    /*     time_t planningHorizon() const */
    /*     { return _planningHorizon; } */

    /** Planning horizon. */
    /*     time_t& planningHorizon() */
    /*     { return _planningHorizon; } */

    /** Inventory Cost */
    double
    inventoryCost() const
    {
        return _inventoryCost;
    }

    /** Inventory Cost */
    double&
    inventoryCost()
    {
        return _inventoryCost;
    }

    /** Inventory Cost Period */
    lut::period_t
    inventoryCostPeriod() const
    {
        return _inventoryCostPeriod;
    }

    /** Inventory Cost Period */
    lut::period_t&
    inventoryCostPeriod()
    {
        return _inventoryCostPeriod;
    }

    /** Whether allow to split MPS quantity */
    bool
    allowMPSquantitySplit() const
    {
        return _allowMPSquantitySplit;
    }

    /** Whether allow to split MPS quantity */
    bool&
    allowMPSquantitySplit()
    {
        return _allowMPSquantitySplit;
    }

    /** all MPS periods. */
    const mpsitemperiod_set_st_t&
    periods() const
    {
        return _periods;
    }

    /** all MPS periods. */
    mpsitemperiod_set_st_t&
    periods()
    {
        return _periods;
    }
    //@}

    void linkPeriods();

    String toString() const;

private:
    void init();
    void deInit();

private:
    uint_t _eoq; // Economic order quantity
    time_t _dtf;      // Demand time fence
    time_t _ptf;      // Planning time fence
                      /*     time_t _planningHorizon; */
    double _inventoryCost;
    lut::period_t _inventoryCostPeriod;
    bool _allowMPSquantitySplit;

    mpsitemperiod_set_st_t _periods;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

typedef std::vector<MPSitem*> mpsitem_vector_t;
typedef std::set<MPSitem*, mrp::ItemOrderingIncId> mpsitem_set_id_t;

////////////////////////////////////////////////////////////////////////////////////////////////////

MPS_NS_END;
