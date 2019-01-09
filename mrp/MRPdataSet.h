#ifndef MRP_MRPDATASET_H
#define MRP_MRPDATASET_H

//////////////////////////////////////////////////////////////////////////////

#include <cse/ClevorDataSet.h>
#include <mrp/SetupGroup.h>
#include <mrp/Resource.h>
#include <mrp/ResourceGroup.h>
#include <mrp/Item.h>
#include <mrp/InventoryRecord.h>
#include <mrp/BOM.h>
#include <mrp/ProcessPlan.h>
#include <mrp/ItemPlanRelation.h>
#include <mrp/ProcessStep.h>
#include <mrp/PlanStepRelation.h>
#include <mrp/WorkOrder.h>


//////////////////////////////////////////////////////////////////////////////

MRP_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////

/**
   MRP data-set.

   \author Adam McKee
*/

//////////////////////////////////////////////////////////////////////////////

class MRPdataSet : public cse::ClevorDataSet
{
    UTL_CLASS_DECL(MRPdataSet);
public:
    virtual void copy(const utl::Object& rhs);

    //// \name Add objects
    //@{
    /** Add a setup group. */
    void add(SetupGroup* setupGroup);

    /** Add a resource. */
    void add(Resource* resource);

    /** Add a resource group. */
    void add(ResourceGroup* resourceGroup);

    /** Add an item. */
    void add(Item* item);

    /** Add an inventory record. */
    void add(InventoryRecord* record);

    /** Add a BOM item. */
    void add(BOM* bom);

    /** Add a process plan. */
    void add(ProcessPlan* plan);

    /** Add an item_plan_relation. */
    void add(ItemPlanRelation* itemPlan);

    /** Add a process step. */
    void add(ProcessStep* step);

    /** Add a process plan and process step relation. */
    void add(PlanStepRelation* planStep);

    /** Add a purchase order. */
    void add(PurchaseOrder* po);

    /** Add a workorder. */
    void add(WorkOrder* wo);
    //@}

    //// \name Find objects
    //@{
    /** Find a MRPresource. */
    Resource* findMRPresource(utl::uint_t id);

    /** Find a MRPresourceGroup. */
    ResourceGroup* findMRPresourceGroup(utl::uint_t id);

    /** Find a Item. */
    Item* findMRPitem(utl::uint_t id);
    //@}
    
    //// \name Accessors
    //{
    /** SetupGroups */
    const setupgroup_set_id_t& setupGroups() const
    { return _setupGroups; }

    /** SetupGroups */
    setupgroup_set_id_t& setupGroups()
    { return _setupGroups; }

    /** Resources. */
    const resource_set_id_t& resources() const
    { return _resources; }

    /** Resources. */
    resource_set_id_t& resources()
    { return _resources; }

    /** Resource groups. */
    const resourcegroup_set_id_t& resourceGroups() const
    { return _resourceGroups; }

    /** Resource groups. */
    resourcegroup_set_id_t& resourceGroups()
    { return _resourceGroups; }

    /** Items. */
    const item_set_id_t& items() const
    { return _items; }

    /** Items. */
    item_set_id_t& items()
    { return _items; }

    /** Inventoyr records. */
    const inventoryrecord_set_id_t& inventoryRecords() const
    { return _inventoryRecords; }

    /** Inventory records. */
    inventoryrecord_set_id_t& inventoryRecords()
    { return _inventoryRecords; }

    /** Process plans. */
    const plan_set_id_t& plans() const
    { return _plans; }

    /** Process plans. */
    plan_set_id_t& plans()
    { return _plans; }

    /** Manufacture item and Process plan relations. */
    const itemplan_vector_t& itemPlans() const
    { return _itemPlans; }

    /** Manufacture item and Process plan relations. */
    itemplan_vector_t& itemPlans()
    { return _itemPlans; }

    /** Process steps. */
    const step_set_id_t& steps() const
    { return _steps; }

    /** Process steps. */
    step_set_id_t& steps()
    { return _steps; }

    /** Process plan and process step relations. */
    const planstep_vector_t& planSteps() const
    { return _planSteps; }

    /** Process plan and process step relations. */
    planstep_vector_t& planSteps()
    { return _planSteps; }

    /** Purchase Orders. */
    const purchaseorder_set_id_t& purchaseOrders() const
    { return _purchaseOrders; }

    /** Purchase Orders. */
    purchaseorder_set_id_t& purchaseOrders()
    { return _purchaseOrders; }

    /** Work Orders. */
    const workorder_set_duetime_t& workOrders() const
    { return _workOrders; }

    /** Work Orders. */
    workorder_set_duetime_t& workOrders()
    { return _workOrders; }

    /** Existing job ids. */
    lut::uint_set_t jobIds() const
    { return _jobIds; }

    /** Existing job ids. */
    lut::uint_set_t& jobIds()
    { return _jobIds; }

    /** Existing op ids. */
    lut::uint_set_t opIds() const
    { return _opIds; }

    /** Existing op ids. */
    lut::uint_set_t& opIds()
    { return _opIds; }

    /** Existing job group ids. */
    lut::uint_set_t jobGroupIds() const
    { return _jobGroupIds; }

    /** Existing job group ids. */
    lut::uint_set_t& jobGroupIds()
    { return _jobGroupIds; }

    /** Existing purchase order ids. */
    lut::uint_set_t purchaseOrderIds() const
    { return _purchaseOrderIds; }

    /** Existing purchase order ids. */
    lut::uint_set_t& purchaseOrderIds()
    { return _purchaseOrderIds; }
    //@}

    /* initialize a MRPdataSet from a list of Arrays sent from 
       the front-end */
    void initialize(
        utl::Array& setupGroups,
        utl::Array& resources,
        utl::Array& resourceGroups,
        utl::Array& items,
        utl::Array& records,
        utl::Array& boms,
        utl::Array& plans,
        utl::Array& itemPlans,
        utl::Array& steps,
        utl::Array& planSteps);

    /** reset all items (_items). */
    void resetItems(utl::Array& items);

    /** reset all inventory records (_inventoryRecords). */
    void resetInventoryRecords(utl::Array& records);

    /** reset all workorders (_workOrders). */
    void resetWorkOrders(utl::Array& workOrders);

    /** reset all jobIds (_jobIds). */
    void resetJobIds(utl::Array& jobIds);

    /** reset all opIds (_opIds). */
    void resetOpIds(utl::Array& opIds);

    /** reset all jobGroupIds (_jobGroupIds). */
    void resetJobGroupIds(utl::Array& jobGroupIds);

    /** reset all purchaseOrderIds (_purchaseOrderIds). */
    void resetPurchaseOrderIds(utl::Array& purchaseOrderIds);

    void checkData() const;
private:
    void init() {}
    void deInit();
private:
    setupgroup_set_id_t _setupGroups;
    resource_set_id_t _resources;
    resourcegroup_set_id_t _resourceGroups;
    item_set_id_t _items;
    inventoryrecord_set_id_t _inventoryRecords;
    bom_vector_t _boms;
    plan_set_id_t _plans;
    itemplan_vector_t _itemPlans;
    step_set_id_t _steps;
    planstep_vector_t _planSteps;
    purchaseorder_set_id_t _purchaseOrders;


    workorder_set_duetime_t _workOrders;
    lut::uint_set_t _jobIds;
    lut::uint_set_t _opIds;
    lut::uint_set_t _jobGroupIds;
    lut::uint_set_t _purchaseOrderIds;
};

//////////////////////////////////////////////////////////////////////////////

MRP_NS_END;

//////////////////////////////////////////////////////////////////////////////

#endif
