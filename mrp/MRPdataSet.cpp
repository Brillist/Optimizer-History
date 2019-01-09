#include "libmrp.h"
#include <libutl/BufferedFDstream.h>
#include <clp/FailEx.h>
#include "MRPdataSet.h"
#include <mrp/DiscreteResource.h>
#include <mrp/PurchaseItem.h>

//////////////////////////////////////////////////////////////////////////////

#ifdef DEBUG
#define DEBUG_UNIT
#endif

//////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;
CSE_NS_USE;

//////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(mrp::MRPdataSet, cse::ClevorDataSet);

//////////////////////////////////////////////////////////////////////////////

MRP_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////

void
MRPdataSet::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(MRPdataSet));
    const MRPdataSet& ds = (const MRPdataSet&)rhs;
    ClevorDataSet::copy(ds);
    copySet(_setupGroups, ds._setupGroups);
    copySet(_resources, ds._resources);
    copySet(_items, ds._items);
    copySet(_inventoryRecords, ds._inventoryRecords);
    copySet(_plans, ds._plans);
    copyVector(_itemPlans, ds._itemPlans);
    copySet(_steps, ds._steps);
    copyVector(_planSteps, ds._planSteps);
    copySet(_purchaseOrders, ds._purchaseOrders);
    copySet(_workOrders, ds._workOrders);
    _jobIds = ds._jobIds;
    _opIds = ds._opIds;
    _jobGroupIds = ds._jobGroupIds;
    _purchaseOrderIds = ds._purchaseOrderIds;
}

//////////////////////////////////////////////////////////////////////////////

void
MRPdataSet::add(SetupGroup* setupGroup)
{
    _setupGroups.insert(setupGroup);
}

//////////////////////////////////////////////////////////////////////////////

void
MRPdataSet::add(Resource* resource)
{
    _resources.insert(resource);
    if (dynamic_cast<DiscreteResource*>(resource) != nullptr)
    {
        DiscreteResource* disRes = (DiscreteResource*)resource;
        SetupGroup* realResGroup
            = lut::setFind(_setupGroups, disRes->setupGroup());
        disRes->setSetupGroup(realResGroup);
    }
}

//////////////////////////////////////////////////////////////////////////////

void
MRPdataSet::add(ResourceGroup* resourceGroup)
{
    _resourceGroups.insert(resourceGroup);
    for (uint_set_t::const_iterator it = resourceGroup->resIds().begin();
         it != resourceGroup->resIds().end(); it++)
    {
        Resource* res = findMRPresource(*it);
        if (res == nullptr)
        {
            String* str = new String();
            *str = "ResourceGroup(" + Uint(resourceGroup->id()).toString();
            *str += ") contains an unknow resource id:" + Uint(*it).toString();
            throw clp::FailEx(str);
        }
        // more check can be put here to check whether res is a unary res
    }
}

//////////////////////////////////////////////////////////////////////////////

void
MRPdataSet::add(Item* item)
{
    _items.insert(item);
}

//////////////////////////////////////////////////////////////////////////////

void
MRPdataSet::add(InventoryRecord* record)
{
    _inventoryRecords.insert(record);
    // record->id() = item->id()
    Item* item = findMRPitem(record->itemId());
    if (item == nullptr)
    {
        String* str = new String();
        *str = "InventoryRecord contains an unknown item id:";
        *str += Uint(record->id()).toString();
        throw clp::FailEx(str);
    }
    item->inventoryRecord() = record;
}

//////////////////////////////////////////////////////////////////////////////

void
MRPdataSet::add(BOM* bom)
{
    _boms.push_back(bom);
    Item* item = lut::setFind(_items, bom->item());
    Item* childItem = lut::setFind(_items, bom->childItem());
    if ((item == nullptr) || (childItem == nullptr))
    {
        String* str = new String();
        *str = "BOM contains an unknown item";
        if (item == nullptr) *str += Uint(bom->item()->id()).toString();
        else *str += Uint(bom->childItem()->id()).toString();
        throw clp::FailEx(str);
    }
    item->boms().push_back(bom);
    bom->setItem(item);
    bom->setChildItem(childItem);
}

//////////////////////////////////////////////////////////////////////////////

void
MRPdataSet::add(ProcessPlan* plan)
{
    _plans.insert(plan);
}

//////////////////////////////////////////////////////////////////////////////

void
MRPdataSet::add(ItemPlanRelation* itemPlan)
{
    _itemPlans.push_back(itemPlan);
    Item* item = lut::setFind(_items, (Item*)itemPlan->item());
    ASSERTD(item != nullptr);
    ASSERTD(dynamic_cast<ManufactureItem*>(item) != nullptr);
    ManufactureItem* mItem = (ManufactureItem*)item;
    mItem->itemPlans().insert(itemPlan);

    ProcessPlan* plan = lut::setFind(_plans, itemPlan->plan());
    ASSERTD(plan != nullptr);

    itemPlan->setItem(mItem);
    itemPlan->setPlan(plan);
}

//////////////////////////////////////////////////////////////////////////////

void
MRPdataSet::add(ProcessStep* step)
{
    _steps.insert(step);
}

//////////////////////////////////////////////////////////////////////////////

void
MRPdataSet::add(PlanStepRelation* planStep)
{
    _planSteps.push_back(planStep);
    ProcessPlan* plan = lut::setFind(_plans, planStep->plan());
    ProcessStep* step = lut::setFind(_steps, planStep->step());
    ProcessStep* succStep = lut::setFind(_steps, planStep->succStep());
    ASSERTD(plan != nullptr);
    ASSERTD(step != nullptr);
    ASSERTD(step != succStep);
    planStep->setPlan(plan);
    planStep->setStep(step);
    planStep->setSuccStep(succStep);

    plan->planSteps().push_back(planStep);
}

//////////////////////////////////////////////////////////////////////////////

void
MRPdataSet::add(PurchaseOrder* po)
{
    _purchaseOrders.insert(po);
}

//////////////////////////////////////////////////////////////////////////////

void
MRPdataSet::add(WorkOrder* wo)
{
    _workOrders.insert(wo);
    Item* item = lut::setFind(_items, (Item*)wo->item());
    if (item == nullptr)
    {
        String* str = new String();
        *str = "WorkOrder (" + Uint(wo->id()).toString()
            + ") requests an unknown item ("
            + Uint(wo->item()->id()).toString() + ")";
        throw clp::FailEx(str);
    }
    if (dynamic_cast<ManufactureItem*>(item) == nullptr)
    {
        String* str = new String();
        *str = "WorkOrder (" + Uint(wo->id()).toString()
            + ") requests a non-manufactureItem ("
            + Uint(wo->item()->id()).toString() + ")";
        throw clp::FailEx(str);
    }
    ManufactureItem* mItem = (ManufactureItem*)item;
    wo->setItem(mItem);
}

//////////////////////////////////////////////////////////////////////////////

void
MRPdataSet::initialize(
    Array& setupGroups,
    Array& resources,
    Array& resourceGroups,
    Array& items,
    Array& records,
    Array& boms,
    Array& plans,
    Array& itemPlans,
    Array& steps,
    Array& planSteps)
{
    // clear out existing data that could be results of a MPS run.
    deInit();

    // so that we don't have to make copies.
    setupGroups.setOwner(false);
    resources.setOwner(false);
    resourceGroups.setOwner(false);
    items.setOwner(false);
    records.setOwner(false);
    boms.setOwner(false);
    plans.setOwner(false);
    itemPlans.setOwner(false);
    steps.setOwner(false);
    planSteps.setOwner(false);


    //SetupGroups and their Setups
    forEachIt(Array, setupGroups, SetupGroup, setupGroup)
        add(setupGroup);
    endForEach

    //Resources, including discrete, composite and alternateResources
    forEachIt(Array, resources, Resource, resource)
        add(resource);
    endForEach

    forEachIt(Array, resourceGroups, ResourceGroup, resourceGroup)
        add(resourceGroup);
    endForEach

    forEachIt(Array, items, Item, item)
       add(item);
    endForEach

    forEachIt(Array, records, InventoryRecord, record)
        add(record);
    endForEach

    forEachIt(Array, boms, BOM, bom)
        add(bom);
    endForEach

    forEachIt(Array, plans, ProcessPlan, plan)
        add(plan);
    endForEach

    forEachIt(Array, itemPlans, ItemPlanRelation, itemPlan)
        add(itemPlan);
    endForEach

    forEachIt(Array, steps, ProcessStep, step)
        add(step);
    endForEach

    forEachIt(Array, planSteps, PlanStepRelation, planStep)
        add(planStep);
    endForEach

#ifdef DEBUG_UNIT
    checkData();
#endif
}

//////////////////////////////////////////////////////////////////////////////

void
MRPdataSet::resetItems(Array& items)
{
    lut::deleteCont(_items);

    // so we don't have to make copies.
    items.setOwner(false);
    forEachIt(Array, items, Item, item)
        add(item);
    endForEach
}

//////////////////////////////////////////////////////////////////////////////

void
MRPdataSet::resetWorkOrders(Array& workOrders)
{
    lut::deleteCont(_workOrders);

    // so we don't have to make copies.
    workOrders.setOwner(false);
    forEachIt(Array, workOrders, WorkOrder, workOrder)
        add(workOrder);
    endForEach
}

//////////////////////////////////////////////////////////////////////////////

void
MRPdataSet::resetJobIds(Array& jobIds)
{
    _jobIds.clear();

    // so we don't have to make copies.
    jobIds.setOwner(false);
    forEachIt(Array, jobIds, Uint, jobId)
        _jobIds.insert((uint_t)jobId);
    endForEach
}

//////////////////////////////////////////////////////////////////////////////

void
MRPdataSet::resetOpIds(Array& opIds)
{
    _opIds.clear();

    // so we don't have to make copies.
    opIds.setOwner(false);
    forEachIt(Array, opIds, Uint, opId)
        _opIds.insert((uint_t)opId);
    endForEach
}
//////////////////////////////////////////////////////////////////////////////

void
MRPdataSet::resetJobGroupIds(Array& jobGroupIds)
{
    _jobGroupIds.clear();

    // so we don't have to make copies.
    jobGroupIds.setOwner(false);
    forEachIt(Array, jobGroupIds, Uint, jobGroupId)
        _jobGroupIds.insert((uint_t)jobGroupId);
    endForEach
}

//////////////////////////////////////////////////////////////////////////////

void
MRPdataSet::resetPurchaseOrderIds(Array& purchaseOrderIds)
{
    _purchaseOrderIds.clear();

    // so we don't have to make copies
    purchaseOrderIds.setOwner(false);
    forEachIt(Array, purchaseOrderIds, Uint, purchaseOrderId)
        _purchaseOrderIds.insert((uint_t)purchaseOrderId);
    endForEach
}

//////////////////////////////////////////////////////////////////////////////

void
MRPdataSet::checkData() const
{
    utl::cout << "SetupGroups:" << utl::endlf;
    for (setupgroup_set_id_t::iterator it = _setupGroups.begin();
         it != _setupGroups.end(); it++)
        utl::cout << (*it)->toString() << utl::endlf;

    utl::cout << utl::endl << "Resources:" << utl::endlf;
    for (resource_set_id_t::iterator it = _resources.begin();
         it != _resources.end(); it++)
        utl::cout << (*it)->toString() << utl::endlf;

    utl::cout << utl::endl << "Items:" << utl::endlf;
    for (item_set_id_t::iterator it = _items.begin();
         it != _items.end(); it++)
        utl::cout << (*it)->toString() << utl::endlf;

    utl::cout << utl::endl << "InventoryRecords:" << utl::endlf;
    for (inventoryrecord_set_id_t::iterator it = _inventoryRecords.begin();
         it != _inventoryRecords.end(); it++)
        utl::cout << (*it)->toString() << utl::endlf;

    utl::cout << utl::endl << "ProcessPlans:" << utl::endlf;
    for (plan_set_id_t::iterator it = _plans.begin();
         it != _plans.end(); it++)
        utl::cout << (*it)->toString() << utl::endlf;

    utl::cout << utl::endl << "ProcessSteps:" << utl::endlf;
    for (step_set_id_t::iterator it = _steps.begin();
         it != _steps.end(); it++)
        utl::cout << (*it)->toString() << utl::endlf;

    utl::cout << utl::endl << "WorkOrders:" << utl::endlf;
    for (workorder_set_duetime_t::iterator it = _workOrders.begin();
         it != _workOrders.end(); it++)
        utl::cout << (*it)->toString() << utl::endlf;
}

//////////////////////////////////////////////////////////////////////////////

Resource*
MRPdataSet::findMRPresource(uint_t id)
{
    Resource dummy;
    dummy.id() = id;
    resource_set_id_t::const_iterator it = _resources.find(&dummy);
    if (it == _resources.end()) return nullptr;
    return *it;
}

//////////////////////////////////////////////////////////////////////////////

ResourceGroup*
MRPdataSet::findMRPresourceGroup(uint_t id)
{
    ResourceGroup dummy;
    dummy.id() = id;
    resourcegroup_set_id_t::const_iterator it = _resourceGroups.find(&dummy);
    if (it == _resourceGroups.end()) return nullptr;
    return *it;
}

//////////////////////////////////////////////////////////////////////////////

Item*
MRPdataSet::findMRPitem(uint_t id)
{
    Item dummy;
    dummy.id() = id;
    item_set_id_t::const_iterator it = _items.find(&dummy);
    if (it == _items.end()) return nullptr;
    return *it;
}

//////////////////////////////////////////////////////////////////////////////

void
MRPdataSet::deInit()
{
    deleteCont(_setupGroups);
    deleteCont(_resources);
    deleteCont(_items);
    deleteCont(_inventoryRecords);
    deleteCont(_boms);
    deleteCont(_plans);
    deleteCont(_itemPlans);
    deleteCont(_steps);
    deleteCont(_planSteps);
    deleteCont(_purchaseOrders);
    deleteCont(_workOrders);

    _jobIds.clear();
    _opIds.clear();
    _jobGroupIds.clear();
    _purchaseOrderIds.clear();
}

//////////////////////////////////////////////////////////////////////////////

MRP_NS_END;
