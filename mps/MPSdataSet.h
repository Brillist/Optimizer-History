#ifndef MPS_MPSDATASET_H
#define MPS_MPSDATASET_H

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <mrp/MRPdataSet.h>
#include <mps/MPSitem.h>
#include <mps/MPSitemPeriod.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

MPS_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   MPS data-set.

   \author Joe Zhou
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class MPSdataSet : public mrp::MRPdataSet
{
    UTL_CLASS_DECL(MPSdataSet, mrp::MRPdataSet);

public:
    virtual void copy(const utl::Object& rhs);

    //// \name Add objects
    //@{
    /** Add a MPS item. */
    void add(MPSitem* item);

    /** Add a MPS item period. */
    void add(MPSitemPeriod* period);
    //@}

    //// \name Accessors
    //{

    /** MPS Items. */
    const mpsitem_set_id_t&
    mpsItems() const
    {
        return _mpsItems;
    }

    /** MPS Items. */
    mpsitem_set_id_t&
    mpsItems()
    {
        return _mpsItems;
    }

    /** MPS item period. */
    const mpsitemperiod_set_itemid_t&
    periods() const
    {
        return _mpsItemPeriods;
    }

    /** Process plans. */
    mpsitemperiod_set_itemid_t&
    periods()
    {
        return _mpsItemPeriods;
    }

    /** Existing work order ids. */
    lut::uint_set_t
    woIds() const
    {
        return _woIds;
    }

    /** Existing work order ids. */
    lut::uint_set_t&
    woIds()
    {
        return _woIds;
    }
    //@}

    /* initialize a MPSdataSet from a list of Arrays sent from 
       the front-end */
    void initialize(utl::Array& mpsItems, utl::Array& mpsItemPeriods);

    /** reset all MPS item periods (_mpsItemPeriods). */
    void resetMPSitemPeriods(utl::Array& mpsItemPeriods);

    /** reset all work order Ids (_woIds). */
    void resetWorkOrderIds(utl::Array& woIds);

    void checkData() const;

private:
    void init();
    void deInit();

private:
    mpsitem_set_id_t _mpsItems;
    mpsitemperiod_set_itemid_t _mpsItemPeriods;

    lut::uint_set_t _woIds;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

MPS_NS_END;

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
