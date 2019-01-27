#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <libutl/RBtree.h>
/* #include <mrp/AltJobsGroup.h> */
#include <mrp/Item.h>
#include <mrp/ItemPlanRelation.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

MRP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Manufactured item.

   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class ManufactureItem : public Item
{
    UTL_CLASS_DECL(ManufactureItem, Item);

public:
    virtual void copy(const utl::Object& rhs);

    virtual void serialize(utl::Stream& stream, uint_t io, uint_t mode = utl::ser_default);

    /// \name Accessors
    //@{
    /** Batch size. */
    uint_t
    batchSize() const
    {
        return _batchSize;
    }

    /** Batch size. */
    uint_t&
    batchSize()
    {
        return _batchSize;
    }

    /** Item Plan Relations. */
    const itemplan_set_t&
    itemPlans() const
    {
        return _itemPlans;
    }

    /** Item Plan Relations. */
    itemplan_set_t&
    itemPlans()
    {
        return _itemPlans;
    }

    /** Alternative jobs. */
    /*     const altjobsgroup_vector_t& jobGroups() const */
    /*     { return _jobGroups; } */

    /** Alternative jobs. */
    /*     altjobsgroup_vector_t& jobGroups() */
    /*     { return _jobGroups; } */
    //@}

    /** get the ProcessPlan with highest preference. */
    ProcessPlan* preferredPlan();

    String toString() const;

private:
    void init();
    void
    deInit()
    {
    }

private:
    uint_t _batchSize;
    itemplan_set_t _itemPlans;

    // alternative jobs groups
    /*     altjobsgroup_vector_t _jobGroups; */
};

////////////////////////////////////////////////////////////////////////////////////////////////////

typedef std::vector<ManufactureItem*> manufitem_vector_t;
typedef std::set<ManufactureItem*, ItemOrderingIncId> manufitem_set_id_t;

////////////////////////////////////////////////////////////////////////////////////////////////////

MRP_NS_END;
