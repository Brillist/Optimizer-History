#include "libmrp.h"
#include <libutl/BufferedFDstream.h>
#include <libutl/Int.h>
#include <libutl/Float.h>
#include <clp/FailEx.h>
#include <cse/Job.h>
#include <cse/JobOp.h>
#include <cse/SummaryOp.h>
#include <mrp/StepResourceRequirement.h>
#include <mrp/StepAltResCapPts.h>
#include <mrp/DiscreteResource.h>
#include <mrp/CompositeResource.h>
#include <mrp/AlternateResources.h>
#include "MRPrun.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef DEBUG
#define DEBUG_UNIT
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;
CSE_NS_USE;
CLS_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(mrp::MRPrun, utl::Object);

////////////////////////////////////////////////////////////////////////////////////////////////////

MRP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

void
MRPrun::initialize(MRPdataSet* dataSet)
{
    if (_dataSetOwner)
        delete _dataSet;
    _dataSet = dataSet;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool
MRPrun::run()
{
    // test code area
    //      utl::cout << "Start test code.." << utl::endlf;
    //     PurchaseItem* item;
    //     item_set_id_t::const_iterator itemIt;
    //     for (itemIt = _dataSet->items().begin();
    //          itemIt != _dataSet->items().end(); itemIt++)
    //     {
    //         Item* item0 = *itemIt;
    //         if (dynamic_cast<PurchaseItem*>(item0) != nullptr)
    //         {
    //             item = (PurchaseItem*)item0;
    //             break;
    //         }
    //     }
    //     cout << item->toString() << endl;

    //     PurchaseOrder* po1 = new PurchaseOrder();
    //     po1->id() = 1;
    //     po1->item() = item;
    //     po1->quantity() = 5;
    //     po1->receiveDate() = 0;

    //     PurchaseOrder* po2 = new PurchaseOrder();
    //     po2->id() = 2;
    //     po2->item() = item;
    //     po2->quantity() = 5;
    //     po2->receiveDate() = 10;

    //     PurchaseOrder* po3 = new PurchaseOrder();
    //     po3->id() = 3;
    //     po3->item() = item;
    //     po3->quantity() = 5;
    //     po3->receiveDate() = 15;

    //     PurchaseOrder* po4 = new PurchaseOrder();
    //     po4->id() = 4;
    //     po4->item() = item;
    //     po4->quantity() = 5;
    //     po4->receiveDate() = 20;

    //     PurchaseOrder* po5 = new PurchaseOrder();
    //     po5->id() = 5;
    //     po5->item() = item;
    //     po5->quantity() = 5;
    //     po5->receiveDate() = 30;

    //     PurchaseOrder* po6 = new PurchaseOrder();
    //     po6->id() = 6;
    //     po6->item() = item;
    //     po6->quantity() = 5;
    //     po6->receiveDate() = 40;

    //     InventoryInterval* intvl1 = new InventoryInterval(po1);
    //     InventoryInterval* intvl2 = new InventoryInterval(po2);
    //     InventoryInterval* intvl3 = new InventoryInterval(po3);
    //     InventoryInterval* intvl4 = new InventoryInterval(po4);
    //     InventoryInterval* intvl5 = new InventoryInterval(po5);
    //     InventoryInterval* intvl6 = new InventoryInterval(po6);

    //     intvl1->cap() = -1 * intvl1->cap();
    //     intvl1->net() = -1 * intvl1->net();
    //     intvl1->debit() = -1 * intvl1->debit();
    //     intvl2->cap() = -1 * intvl2->cap();
    //     intvl2->net() = -1 * intvl2->net();
    //     intvl2->debit() = -1 * intvl2->debit();
    //     intvl4->cap() = -1 * intvl4->cap();
    //     intvl4->net() = -1 * intvl4->net();
    //     intvl4->debit() = -1 * intvl4->debit();
    //     intvl6->cap() = -1 * intvl6->cap();
    //     intvl6->net() = -1 * intvl6->net();
    //     intvl6->debit() = -1 * intvl6->debit();

    //     item->inventoryRecord()->insertNode(intvl1);
    //     item->inventoryRecord()->insertNode(intvl2);
    //     item->inventoryRecord()->insertNode(intvl3);
    //     item->inventoryRecord()->insertNode(intvl4);
    //     item->inventoryRecord()->insertNode(intvl5);
    //     item->inventoryRecord()->insertNode(intvl6);

    //     item->inventoryRecord()->dumpTree();
    //     utl::cout << utl::endlf;
    //     item->inventoryRecord()->checkTree();

    //     utl::cout << "Check available capacities..." << utl::endlf;
    //     for (int i = 0; i < 50; i++)
    //     {
    //         utl::cout << "At time " << i
    //                   << ", avail-cap = "
    //                   << item->inventoryRecord()->availableCapacity((time_t)i)
    //                   << utl::endlf;
    //     }

    //     utl::cout << "Remove nodes from the tree..." << utl::endlf;
    //     utl::cout << "Remove: " << intvl5->toString() << utl::endlf;
    //     item->inventoryRecord()->deleteNode(intvl5);

    //     item->inventoryRecord()->dumpTree();
    //     utl::cout << utl::endlf;
    //     item->inventoryRecord()->checkTree();

    //     utl::cout << "End of test code..." << utl::endl << utl::endlf;

    // do some clean-up
    //     item_set_id_t::const_iterator itemIt;
    //     for (itemIt = _dataSet->items().begin();
    //          itemIt != _dataSet->items().end(); itemIt++)
    //     {
    //         Item* item = *itemIt;
    //         if (dynamic_cast<ManufactureItem*>(item) == nullptr) continue;
    //     }

    workorder_set_duetime_t wos = _dataSet->workOrders();
    for (workorder_set_duetime_t::iterator woIt = wos.begin(); woIt != wos.end(); woIt++)
    {
        WorkOrder* wo = *woIt;
        // only generate jobs for unplanned wos.
        if (wo->status() != wostatus_unplanned)
            continue;

        // only generate jobs when there is not enough in stock
        uint_t woQuantity = wo->item()->checkAndConsumeInventory(
            transoptype_workorder, wo->id(), wo->dueTime(), wo->quantity(), transstatus_planned);
        if (woQuantity == 0)
            continue;

        // create all final assembly jobs for the item in the wo
        // note: jobs don't include jobs of children items.
        job_set_pref_t jobs;
        createJobs(wo->id(), wo->name(), woQuantity, wo->item(), wo->releaseTime(), jobs);

        wo->item()->consumeInventory(transoptype_workorder, wo->id(), wo->dueTime(), woQuantity,
                                     transstatus_planned);

        for (job_set_pref_t::const_iterator it = jobs.begin(); it != jobs.end(); it++)
        {
            Job* job = *it;
            job->dueTime() = wo->dueTime();
            job->opportunityCost() = wo->opportunityCost();
            job->opportunityCostPeriod() = wo->opportunityCostPeriod();
            job->latenessCost() = wo->latenessCost();
            job->latenessCostPeriod() = wo->latenessCostPeriod();
            job->inventoryCost() = wo->inventoryCost();
            job->inventoryCostPeriod() = wo->inventoryCostPeriod();

#ifdef DEBUG_UNIT
            utl::cout << "wo_job:" << job->id() << ", active:" << job->active()
                      << ", releaseT:" << getTimeString(job->releaseTime()).c_str()
                      << ", dueT:" << getTimeString(job->dueTime()).c_str()
                      << ", ivntC:" << Float(job->inventoryCost()).toString("precisioin:2")
                      << ", opporC:" << Float(job->opportunityCost()).toString("precision:2")
                      << ", lateC:" << Float(job->latenessCost()).toString("precision:2")
                      << utl::endlf;
#endif
        }
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool
MRPrun::createJobs(uint_t woId,
                   std::string jobName,
                   uint_t jobQuantity,
                   ManufactureItem* item,
                   time_t releaseTime,
                   job_set_pref_t& jobs)
{
    // create jobGroup
    JobGroup* jobGroup = nullptr;
    if (item->itemPlans().size() > 1)
    {
        // creat a new jobGroup and decide its Id
        jobGroup = new JobGroup();
        _initId++;
        while (_dataSet->jobGroupIds().find(_initId) != _dataSet->jobGroupIds().end())
        {
            _initId++;
        }
        jobGroup->id() = _initId;
    }
    // create multiple jobs in a group
    itemplan_set_t::const_iterator planIt;
    for (planIt = item->itemPlans().begin(); planIt != item->itemPlans().end(); planIt++)
    {
        ItemPlanRelation* itemplan = *planIt;
        // create a new job and decide its Id.
        Job* job = new Job();
        _initId++;
        while (_dataSet->jobIds().find(_initId) != _dataSet->jobIds().end() ||
               _dataSet->opIds().find(_initId) != _dataSet->opIds().end())
        {
            _initId++;
        }
        job->id() = _initId;
        job->name() = jobName;
        job->preference() = itemplan->preference();
        job->active() = false;
        job->status() = jobstatus_inactive;
        job->workOrderIds().insert(woId);
        job->itemId() = item->id();
        job->itemQuantity() = jobQuantity;

        // create the root op(rop) for the job, rop->id() = job->id()
        SummaryOp* rop = new SummaryOp();
        rop->id() = job->id();
        rop->name() = jobName;
        rop->type() = op_summary;
        rop->status() = opstatus_unstarted;
        rop->scheduledBy() = sa_clevor;
        job->add(rop);
        job->rootSummaryOp() = rop;
        if (item->boms().size() == 0 && releaseTime != -1)
        {
            job->releaseTime() = releaseTime;
            cse::UnaryCt* uct = new cse::UnaryCt(cse::uct_startNoSoonerThan, releaseTime);
            rop->add(uct);
        }
        ProcessPlan* plan = itemplan->plan();
        if (plan == item->preferredPlan())
        {
            job->active() = true;
            job->status() = jobstatus_planned;
        }
        createJobOps(job, plan);
        _dataSet->ClevorDataSet::add(job);
        jobs.insert(job);
        if (jobGroup != nullptr)
        {
            job->groupId() = jobGroup->id();
            jobGroup->jobs().insert(job);
            if (job->active())
            {
                jobGroup->setActiveJob(job);
                ;
            }
        }
    } // end of creating multiple jobs in a group
    if (jobGroup != nullptr)
    {
        _dataSet->ClevorDataSet::add(jobGroup);
        item->produceInventory(transoptype_jobgroup, jobGroup->id(), releaseTime, jobQuantity,
                               transstatus_planned);
    }
    else
    {
        item->produceInventory(transoptype_job, (*jobs.begin())->id(), releaseTime, jobQuantity,
                               transstatus_planned);
    }

    // BOM exploration => create child jobs
    for (uint_t i = 0; i < item->boms().size(); i++)
    {
        BOM* bom = item->boms()[i];
        Item* cItem = bom->childItem();

        //check inventory of childItem
        uint_t childJobQuantity = 0;
        if (jobGroup == nullptr)
        {
            ASSERTD(jobs.size() == 1);
            Job* job = *jobs.begin();
            childJobQuantity =
                cItem->checkAndConsumeInventory(transoptype_job, job->id(), releaseTime,
                                                jobQuantity * bom->quantity(), transstatus_planned);
        }
        else
        {
            childJobQuantity =
                cItem->checkAndConsumeInventory(transoptype_jobgroup, jobGroup->id(), releaseTime,
                                                jobQuantity * bom->quantity(), transstatus_planned);
        }
        if (childJobQuantity == 0)
            continue;

        // if childItem is a purchase item.
        job_set_pref_t childjobs;
        if (dynamic_cast<PurchaseItem*>(cItem) != nullptr)
        {
            if (jobGroup == nullptr)
            {
                Job* job = *jobs.begin();
                createPurchaseOrder(woId, cItem->name(), childJobQuantity, (PurchaseItem*)cItem,
                                    releaseTime, job->id(), requestertype_job);
            }
            else
            {
                createPurchaseOrder(woId, cItem->name(), childJobQuantity, (PurchaseItem*)cItem,
                                    releaseTime, jobGroup->id(), requestertype_jobgroup);
            }
        }
        else
        {
            createJobs(woId, cItem->name(), childJobQuantity, (ManufactureItem*)cItem, releaseTime,
                       childjobs);
        }

        if (jobGroup == nullptr)
        {
            ASSERTD(jobs.size() == 1);
            Job* job = *jobs.begin();
            cItem->consumeInventory(transoptype_job, job->id(), releaseTime, childJobQuantity,
                                    transstatus_planned);
        }
        else
        {
            cItem->consumeInventory(transoptype_jobgroup, jobGroup->id(), releaseTime,
                                    childJobQuantity, transstatus_planned);
        }

        job_set_pref_t::const_iterator jobIt;
        job_set_pref_t::const_iterator cjobIt;
        for (jobIt = jobs.begin(); jobIt != jobs.end(); jobIt++)
        {
            Job* job = *jobIt;
            for (cjobIt = childjobs.begin(); cjobIt != childjobs.end(); cjobIt++)
            {
                Job* childJob = *cjobIt;
                // add precedenceCt between job and childJob
                // note: no consideration for precedenceCt
                //       between two ops from different jobs
                cse::PrecedenceCt* pct = new cse::PrecedenceCt();
                pct->lhsOpId() = childJob->rootSummaryOp()->id();
                pct->rhsOpId() = job->rootSummaryOp()->id();
                pct->type() = pct_fs;
                _dataSet->ClevorDataSet::add(pct);
            }
        }
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
MRPrun::createPurchaseOrder(uint_t woId,
                            std::string poName,
                            uint_t poQuantity,
                            PurchaseItem* item,
                            time_t releaseTime,
                            uint_t requesterId,
                            requester_type_t reqType)
{
    PurchaseOrder* po = new PurchaseOrder();
    _initId++;
    while (_dataSet->purchaseOrderIds().find(_initId) != _dataSet->purchaseOrderIds().end())
    {
        _initId++;
    }
    po->id() = _initId;
    po->name() = poName;
    po->setItem(item, false);
    po->quantity() = poQuantity;
    po->status() = postatus_planned;
    po->orderDate() = releaseTime;
    po->receiveDate() = releaseTime + item->leadTime();
    po->requesterId() = requesterId;
    po->requesterType() = reqType;
    item->produceInventory(transoptype_purchaseorder, po->id(), po->receiveDate(), po->quantity(),
                           transstatus_planned);
    _dataSet->add(po);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
MRPrun::createJobOps(Job* job, ProcessPlan* plan)
{
    std::map<uint_t, uint_t> stepOpIdMap;
    const planstep_vector_t steps = plan->planSteps();
    planstep_vector_t::const_iterator stepIt;
    for (stepIt = steps.begin(); stepIt != steps.end(); stepIt++)
    {
        PlanStepRelation* planstep = *stepIt;
        ProcessStep* step = planstep->step();
        // if-check: when a step has more than one successors, continue
        if (stepOpIdMap.find(step->id()) != stepOpIdMap.end())
            continue;
        _initId++;
        while (_dataSet->opIds().find(_initId) != _dataSet->opIds().end())
        {
            _initId++;
        }
        JobOp* op = step->createJobOp(_initId, job->itemQuantity());
        stepOpIdMap.insert(std::pair<uint_t, uint_t>(step->id(), op->id()));

        // add ss and ff precedenceCts to root op (rop)
        cse::PrecedenceCt* ss = new cse::PrecedenceCt();
        cse::PrecedenceCt* ff = new cse::PrecedenceCt();
        ss->lhsOpId() = ff->rhsOpId() = job->rootSummaryOp()->id();
        ss->rhsOpId() = ff->lhsOpId() = op->id();
        ss->type() = pct_ss;
        ff->type() = pct_ff;
        _dataSet->ClevorDataSet::add(ss);
        _dataSet->ClevorDataSet::add(ff);
        job->add(op);

        createResourceRequirements(step, job, op);
#ifdef DEBUG_UNIT
        utl::cout << "   job:" << job->id();
        //                           << ", woId:" << woId
        //                           << ", jGrp:";
        //                 if (jobGroup != nullptr) utl::cout << jobGroup->id();
        //                 else utl::cout << "null";
        utl::cout << ", active:" << job->active()
                  << ", releaseT:" << getTimeString(job->releaseTime()).c_str()
                  << ", dueT:" << getTimeString(job->dueTime()).c_str()
                  << ", item:" << job->itemId() << ", qnty:" << job->itemQuantity()
                  << ", status:" << job->status() << ", plan:" << plan->id()
                  << ", step:" << step->id() << ", op:" << op->id()
                  << ", #resReqs:" << step->resourceRequirements().size() << utl::endlf;
#endif
    } // end of creating all normal ops

    // create cse::PrecedenceCt between ops of the job and add it to ClevorDataSet
    for (stepIt = steps.begin(); stepIt != steps.end(); stepIt++)
    {
        PlanStepRelation* planstep = *stepIt;
        if (planstep->succStep())
        {
            cse::PrecedenceCt* pct = planstep->createPrecedenceCt();
            std::map<uint_t, uint_t>::iterator it;
            ASSERTD(stepOpIdMap.find(pct->lhsOpId()) != stepOpIdMap.end());
            ASSERTD(stepOpIdMap.find(pct->rhsOpId()) != stepOpIdMap.end());
            it = stepOpIdMap.find(pct->lhsOpId());
            pct->lhsOpId() = (*it).second;
            it = stepOpIdMap.find(pct->rhsOpId());
            pct->rhsOpId() = (*it).second;
            _dataSet->ClevorDataSet::add(pct);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
MRPrun::addResourceToClevorDS(Resource* res)
{
    if (res->isA(DiscreteResource))
    {
        if (_dataSet->findResource(res->id()) == nullptr)
        {
            cse::DiscreteResource* disRes = ((DiscreteResource*)res)->createDiscreteResource();
            _dataSet->ClevorDataSet::add(disRes);
        }
    }
    else if (res->isA(CompositeResource))
    {
        if (_dataSet->findResource(res->id()) == nullptr)
        {
            //// add composite resource
            cse::CompositeResource* cmpRes = ((CompositeResource*)res)->createCompositeResource();
            _dataSet->ClevorDataSet::add(cmpRes);
            //// add resource group
            uint_t groupId = ((CompositeResource*)res)->resourceGroupId();
            if (_dataSet->findResourceGroup(groupId) == nullptr)
            {
                ResourceGroup* rGroup = _dataSet->findMRPresourceGroup(groupId);
                ASSERTD(rGroup != nullptr);
                cse::ResourceGroup* resGroup = rGroup->createResourceGroup();
                _dataSet->ClevorDataSet::add(resGroup);
                //// add member discrete resources
                for (uint_set_t::const_iterator it = resGroup->resIds().begin();
                     it != resGroup->resIds().end(); it++)
                {
                    uint_t resId = *it;
                    if (_dataSet->findResource(resId) == nullptr)
                    {
                        Resource* res = _dataSet->findMRPresource(resId);
                        ASSERTD(res != nullptr);
                        ASSERTD(res->isA(DiscreteResource));
                        cse::DiscreteResource* disRes =
                            ((DiscreteResource*)res)->createDiscreteResource();
                        _dataSet->ClevorDataSet::add(disRes);
                    }
                }
            }
        }
    }
    else if (res->isA(AlternateResources))
    {
        //// add resource group
        uint_t groupId = ((AlternateResources*)res)->resourceGroupId();
        if (_dataSet->findResourceGroup(groupId) == nullptr)
        {
            ResourceGroup* rGroup = _dataSet->findMRPresourceGroup(groupId);
            ASSERTD(rGroup != nullptr);
            cse::ResourceGroup* resGroup = rGroup->createResourceGroup();
            _dataSet->ClevorDataSet::add(resGroup);
            //// add member discrete resources
            for (uint_set_t::const_iterator it = resGroup->resIds().begin();
                 it != resGroup->resIds().end(); it++)
            {
                uint_t resId = *it;
                if (_dataSet->findResource(resId) == nullptr)
                {
                    Resource* res = _dataSet->findMRPresource(resId);
                    ASSERTD(res != nullptr);
                    ASSERTD(res->isA(DiscreteResource));
                    cse::DiscreteResource* disRes =
                        ((DiscreteResource*)res)->createDiscreteResource();
                    _dataSet->ClevorDataSet::add(disRes);
                }
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
MRPrun::createResourceRequirements(ProcessStep* step, Job* job, JobOp* op)
{
    forEachIt(Array, step->resourceRequirements(), StepResourceRequirement, resReq) Resource* res =
        _dataSet->findMRPresource(resReq.resourceId());
    ASSERTD(res != nullptr);

    // add the resource to Clevor DataSet if it's not there
    addResourceToClevorDS(res);

    if (res->isA(DiscreteResource) || res->isA(CompositeResource))
    {
        cse::ResourceRequirement* cseResReq = resReq.createResourceRequirement();
        // grab the corresponding StepAltResCapPts
        // Note: we should make StepAltResCapPts as an attribute of StepResourceRequirement
        //       in the future, just like what we did in MRPGui. So we can avoid this step.
        //       Joe, August 19, 2009
        StepAltResCapPts* capPts = nullptr;
        forEachIt(Array, step->resourceCapPts(), StepAltResCapPts,
                  resCapPts) if (res->id() == resCapPts.resourceId())
        {
            capPts = &resCapPts;
            break;
        }
        endForEach

            // add resReq.capacity(0 and step->processingTime() to CapPts if they are not there
            if (resReq.capacity() != uint_t_max && step->processingTime() != uint_t_max &&
                capPts != nullptr && capPts->numCapPts() > 0)
        {
            if (!capPts->hasCap(resReq.capacity()))
            {
                capPts->addCapPt(resReq.capacity(), step->processingTime(),
                                 (rescappt_pt_per_t)step->ptPer(), step->ptBatchSize(),
                                 resReq.setupId());
            }
        }
        if (res->isA(DiscreteResource))
        {
            if (capPts != nullptr && capPts->numCapPts() > 0)
            {
                // clean op's pt and cseResReq's cap
                op->processingTime() = uint_t_max;
                cseResReq->capacity() = uint_t_max;

                cls::ResourceCapPts* clsCapPts = capPts->createResourceCapPts(job->itemQuantity());
                op->add(clsCapPts);
            }
            else
            {
                ASSERTD(op->processingTime() != uint_t_max && cseResReq->capacity() != uint_t_max);
            }
        }
        else
        {
            if (capPts != nullptr && capPts->numCapPts() > 0)
            {
                cseResReq->capacity() = capPts->minCap();
                cseResReq->maxCapacity() = capPts->maxCap();
            }
            else
            {
                cseResReq->maxCapacity() = cseResReq->capacity();
            }
            ASSERTD(op->processingTime() != uint_t_max && cseResReq->capacity() != uint_t_max);
            op->type() = op_interruptible;
            op->scheduledProcessingTime() = op->processingTime();
            op->scheduledRemainingPt() = op->processingTime();
            cseResReq->scheduledCapacity() = cseResReq->capacity();
        }
        op->addResReq(cseResReq);
    }
    else if (res->isA(AlternateResources))
    {
        uint_t groupId = ((AlternateResources*)res)->resourceGroupId();
        ResourceGroup* group = _dataSet->findMRPresourceGroup(groupId);
        ASSERTD(group != nullptr);
        uint_set_t resIds = group->resIds();
        cse::ResourceGroupRequirement* cseResGrpReq =
            resReq.createResourceGroupRequirement(groupId);
        forEachIt(Array, step->resourceCapPts(), StepAltResCapPts,
                  capPts) if (resIds.find(capPts.resourceId()) != resIds.end())
        {
            if (resReq.capacity() != uint_t_max && step->processingTime() != uint_t_max &&
                capPts.numCapPts() > 0)
            {
                if (!capPts.hasCap(resReq.capacity()))
                {
                    capPts.addCapPt(resReq.capacity(), step->processingTime(),
                                    (rescappt_pt_per_t)step->ptPer(), step->ptBatchSize(),
                                    resReq.setupId());
                }

                cls::ResourceCapPts* clsCapPts = capPts.createResourceCapPts(job->itemQuantity());
                op->add(clsCapPts);
            }
            if (capPts.numCapPts() > 0)
            {
                // clean op's pt and cseResGrpReq's cap
                op->processingTime() = uint_t_max;
                cseResGrpReq->capacity() = uint_t_max;
            }
        }
        endForEach op->addResGroupReq(cseResGrpReq);
    }
    endForEach
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
MRPrun::init()
{
    _dataSet = nullptr;
    _dataSetOwner = true;
    _initId = 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
MRPrun::deInit()
{
    if (_dataSetOwner)
        delete _dataSet;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

MRP_NS_END;
