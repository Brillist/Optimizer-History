#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <libutl/Object.h>
#include <cse/PrecedenceCt.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

MRP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

class ManufactureItem;
class ProcessPlan;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Every manufacture item's alternative process plans and preferences.

   \author Joe Zhou
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class ItemPlanRelation : public utl::Object
{
    UTL_CLASS_DECL(ItemPlanRelation, utl::Object);

public:
    /** Constructor. */
    ItemPlanRelation(ManufactureItem* item, ProcessPlan* plan, uint_t preference = uint_t_max);

    virtual void copy(const utl::Object& rhs);

    virtual void serialize(utl::Stream& stream, uint_t io, uint_t mode = utl::ser_default);

    /// \name Accessors
    //@{
    /** Item. */
    const ManufactureItem*
    item() const
    {
        return _item;
    }

    /** Item. */
    ManufactureItem*
    item()
    {
        return _item;
    }

    /** Process plan. */
    const ProcessPlan*
    plan() const
    {
        return _plan;
    }

    /** Process plan. */
    ProcessPlan*
    plan()
    {
        return _plan;
    }

    /** Preference. */
    uint_t
    preference() const
    {
        return _preference;
    }

    /** Preference. */
    uint_t&
    preference()
    {
        return _preference;
    }
    //@}

    /** reset item. */
    void setItem(ManufactureItem* item, bool owner = false);

    /** reset plan. */
    void setPlan(ProcessPlan* plan, bool owner = false);

    String toString() const;

private:
    void init();
    void deInit();

private:
    ManufactureItem* _item;
    ProcessPlan* _plan;
    uint_t _preference;

    bool _itemOwner;
    bool _planOwner;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

/** Order ItemPlanRelation objects by preference. */
struct PlanOrderingDecPref : public std::binary_function<ItemPlanRelation*, ItemPlanRelation*, bool>
{
    bool operator()(const ItemPlanRelation* lhs, const ItemPlanRelation* rhs) const;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

typedef std::set<ItemPlanRelation*, PlanOrderingDecPref> itemplan_set_t;
typedef std::vector<ItemPlanRelation*> itemplan_vector_t;

////////////////////////////////////////////////////////////////////////////////////////////////////

MRP_NS_END;
