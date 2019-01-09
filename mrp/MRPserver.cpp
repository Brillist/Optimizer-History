#include "libmrp.h"
#include <libutl/Bool.h>
#include <clp/FailEx.h>
#include <gop/ConfigEx.h>
#include <cse/Server.h>
#include "MRPdataSet.h"
#include "MRPserver.h"
#include "MRPclient.h"
#include <cse/Scheduler.h>
#include <cse/ScheduleEvaluatorConfiguration.h>
#include <cse/SchedulingRun.h>

///////////////////////////////////////////////////////////////////////////////

#ifdef DEBUG
#define DEBUG_UNIT
#endif

///////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;
GOP_NS_USE;
CSE_NS_USE;
CLP_NS_USE;

///////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(mrp::MRPserver, cse::Server);

///////////////////////////////////////////////////////////////////////////////

MRP_NS_BEGIN;

///////////////////////////////////////////////////////////////////////////////

NetServerClient*
MRPserver::clientMake(
    FDstream* socket,
    const InetHostAddress& addr)
{
    MRPserver* const server = this;
    return new MRPclient(server, socket, addr, _recording);
}

///////////////////////////////////////////////////////////////////////////////

void
MRPserver::addHandler(const char* cmd, MRPhfn handler)
{
    Server::addHandler(cmd, (hfn)handler);
}

///////////////////////////////////////////////////////////////////////////////

void
MRPserver::handle_initMRP(MRPclient* client, const Array& cmd)
{
#ifdef DEBUG_UNIT
    utl::cout << utl::endl
              << "MRPserver::handle_initMRP."
              << "cmd.size():" << cmd.size()
              << utl::endlf;
#endif

    if ((cmd.size() != 11)
        || !cmd(1).isA(Array)
        || !allAre(cmd(1), CLASS(SetupGroup))

        || !cmd(2).isA(Array)
        || !allAre(cmd(2), CLASS(Resource))

        || !cmd(3).isA(Array)
        || !allAre(cmd(3), CLASS(ResourceGroup))

        || !cmd(4).isA(Array)
        || !allAre(cmd(4), CLASS(Item))

        || !cmd(5).isA(Array)
        || !allAre(cmd(5), CLASS(InventoryRecord))

        || !cmd(6).isA(Array)
        || !allAre(cmd(6), CLASS(BOM))

        || !cmd(7).isA(Array)
        || !allAre(cmd(7), CLASS(ProcessPlan))

        || !cmd(8).isA(Array)
        || !allAre(cmd(8), CLASS(ItemPlanRelation))

        || !cmd(9).isA(Array)
        || !allAre(cmd(9), CLASS(ProcessStep))

        || !cmd(10).isA(Array)
        || !allAre(cmd(10), CLASS(PlanStepRelation)))
    {
        Bool(false).serializeOut(client->socket());
        utl::String str = "initMRP error: wrong cmd.";
        str.serializeOut(client->socket());
        finishCmd(client);
        clientDisconnect(client);//?
        return;
    }
    Array& setupGroups = (Array&)cmd(1);
    Array& resources = (Array&)cmd(2);
    Array& resourceGroups = (Array&)cmd(3);
    Array& items = (Array&)cmd(4);
    Array& records = (Array&)cmd(5);
    Array& boms = (Array&)cmd(6);
    Array& plans = (Array&)cmd(7);
    Array& itemPlans = (Array&)cmd(8);
    Array& steps = (Array&)cmd(9);
    Array& planSteps = (Array&)cmd(10);

    bool result = true;
    utl::String errStr = "initMRP error: ";
    MRPdataSet* dataSet;
    if (client->mrpRun()->dataSet() == nullptr)
    {
        dataSet = new MRPdataSet();
    }
    else
    {
        dataSet = client->mrpRun()->dataSet();
    }
    try
    {
        dataSet->initialize(
                setupGroups,
                resources,
                resourceGroups,
                items,
                records,
                boms,
                plans,
                itemPlans,
                steps,
                planSteps);
        client->mrpRun()->initialize(dataSet);
    }
    catch (...)
    {
        result = false;
        errStr += "MRPdataSet initialization failed.";
    }
    Bool(result).serializeOut(client->socket());
    if (!result)
    {
        errStr.serializeOut(client->socket());
        finishCmd(client);
        clientDisconnect(client);
    }
    else
    {
        finishCmd(client);
    }
    return;
}

///////////////////////////////////////////////////////////////////////////////

void
MRPserver::handle_generateJobs(MRPclient* client, const Array& cmd)
{
#ifdef DEBUG_UNIT
    utl::cout << utl::endl
              << "MRPserver::handle_generateJobs."
              << "cmd.size():" << cmd.size()
              << utl::endlf;
#endif

    MRPdataSet* dataSet = client->mrpRun()->dataSet();
    if ((cmd.size() != 6)
        || !cmd(1).isA(Array)
        || !allAre(cmd(1), CLASS(WorkOrder))

        || !cmd(2).isA(Array)
        || !allAre(cmd(2), CLASS(Uint))

        || !cmd(3).isA(Array)
        || !allAre(cmd(3), CLASS(Uint))

        || !cmd(4).isA(Array)
        || !allAre(cmd(4), CLASS(Uint))

        || !cmd(5).isA(Array)
        || !allAre(cmd(5), CLASS(Uint))

        || dataSet == nullptr)
    {
        Bool(false).serializeOut(client->socket());
        utl::String str = "generatorJobs error: wrong cmd.";
        str.serializeOut(client->socket());
        finishCmd(client);
        clientDisconnect(client);
        return;
    }
    Array& workOrders = (Array&)cmd(1);
    Array& jobIds = (Array&)cmd(2);
    Array& opIds = (Array&)cmd(3);
    Array& jobGroupIds = (Array&)cmd(4);
    Array& purchaseOrderIds = (Array&)cmd(5);
    bool result = true;
    utl::String errStr = "generateJobs error: ";
    try
    {
        dataSet->resetWorkOrders(workOrders);
        dataSet->resetJobIds(jobIds);
        dataSet->resetOpIds(opIds);
        dataSet->resetJobGroupIds(jobGroupIds);
        dataSet->resetPurchaseOrderIds(purchaseOrderIds);

#ifdef DEBUG_UNIT
        utl::cout << "   Input Work Orders: size:"
                  << workOrders.size()
                  << utl::endlf;
        forEachIt(Array, workOrders, WorkOrder, wo)
            utl::cout << "   " << wo.toString() << utl::endlf;
        endForEach
        utl::cout << "   Input existing jobIds: size:"
                  << jobIds.size()
                  << ", Ids:";
        forEachIt(Array, jobIds, Uint, jobId)
            utl::cout << jobId << ",";
        endForEach
        utl::cout << utl::endlf;
        utl::cout << "   Input existing opIds: size:"
                  << opIds.size()
                  << ", Ids:";
        forEachIt(Array, opIds, Uint, opId)
            utl::cout << opId << ",";
        endForEach
        utl::cout << utl::endlf;
        utl::cout << "   Input existing jobGroupIds: size:"
                  << jobGroupIds.size()
                  << ", Ids:";
        forEachIt(Array, jobGroupIds, Uint, jobGroupId)
            utl::cout << jobGroupId << ",";
        endForEach
        utl::cout << utl::endl << utl::endlf;
        utl::cout << "   Input existing purchaseOrderIds: size:"
                  << purchaseOrderIds.size()
                  << ", Ids:";
        forEachIt(Array, purchaseOrderIds, Uint, purchaseOrderId)
            utl::cout << purchaseOrderId << ",";
        endForEach
        utl::cout << utl::endl << utl::endlf;
#endif

        client->mrpRun()->run();
    }
    catch (clp::FailEx& ex)
    {
        result = false;
        errStr += *ex.str();
    }
    catch (...)
    {
        result = false;
        errStr += "unknown error.";
    }
    Bool(result).serializeOut(client->socket());
    if(!result)
    {
        errStr.serializeOut(client->socket());
        finishCmd(client);
        clientDisconnect(client);
    }
    else
    {
        finishCmd(client);
    }
    return;
}

///////////////////////////////////////////////////////////////////////////////

void
MRPserver::handle_getJobs(MRPclient* client, const Array& cmd)
{
#ifdef DEBUG_UNIT
    utl::cout << utl::endl
              << "MRPserver::handle_getJobs."
              << "cmd.size():" << cmd.size()
              << utl::endlf;
#endif

    if ((cmd.size() != 1))
    {
        Bool(false).serializeOut(client->socket());
        utl::String str = "getJobs error: wrong cmd.";
        str.serializeOut(client->socket());
        finishCmd(client);
        clientDisconnect(client);//?
        return;
    }
    MRPdataSet* dataSet = client->mrpRun()->dataSet();
    ASSERTD(dataSet != nullptr);

#ifdef DEBUG_UNIT
    utl::cout << "   Output jobs: size:"
              << Uint(dataSet->jobs().size())
              << utl::endlf;
    for (job_set_id_t::const_iterator it = dataSet->jobs().begin();
         it != dataSet->jobs().end(); it++)
    {
        utl::cout << (*it)->toString() << utl::endlf;
    }
#endif

    Bool(true).serializeOut(client->socket());

    lut::serialize<Job*>(dataSet->jobs(), client->socket(), io_wr);

    lut::serialize<JobGroup*>(dataSet->jobGroups(), client->socket(), io_wr);

    lut::serialize<PrecedenceCt*>(dataSet->precedenceCts(), client->socket(), io_wr);

    lut::serialize<cse::Resource*>(dataSet->ClevorDataSet::resources(), client->socket(), io_wr);

    lut::serialize<cse::ResourceGroup*>(dataSet->ClevorDataSet::resourceGroups(), client->socket(), io_wr);

    lut::serialize<ResourceSequenceList*>(dataSet->resourceSequenceLists(),
                                          client->socket(), io_wr);
    lut::serialize<InventoryRecord*>(dataSet->inventoryRecords(),
                                     client->socket(), io_wr);
    lut::serialize<PurchaseOrder*>(dataSet->purchaseOrders(),
                                   client->socket(), io_wr);

    finishCmd(client);
}

///////////////////////////////////////////////////////////////////////////////

// the main reason for MRPserver doesn't use cse::Server's initScheduler
// is because MRPserver doesn't need to create a new dataSet.
void
MRPserver::handle_initScheduler(MRPclient* client, const Array& cmd)
{
#ifdef DEBUG_UNIT
    utl::cout << utl::endl
              << "MRPserver::handle_initScheduler."
              << "cmd.size():" << cmd.size()
              << utl::endlf;
#endif

    if ((cmd.size() != 11)

        || !cmd(1).isA(SchedulerConfiguration)

        || !cmd(2).isA(Scheduler)

        || !cmd(3).isA(Array)
        || !allAre(cmd(3), CLASS(Objective))

        || !cmd(4).isA(Array)
        || !allAre(cmd(4), CLASS(ScheduleEvaluatorConfiguration))

        || !cmd(5).isA(Array)
        || !allAre(cmd(5), CLASS(Job))

        || !cmd(6).isA(Array)
        || !allAre(cmd(6), CLASS(JobGroup))

        || !cmd(7).isA(Array)
        || !allAre(cmd(7), CLASS(PrecedenceCt))

        || !cmd(8).isA(Array)
        || !allAre(cmd(8), CLASS(cse::Resource))

        || !cmd(9).isA(Array)
        || !allAre(cmd(9), CLASS(cse::ResourceGroup))

        || !cmd(10).isA(Array)
        || !allAre(cmd(10), CLASS(ResourceSequenceList)))
    {
        Bool(false).serializeOut(client->socket());
        utl::String str = "initScheduler error: wrong cmd.";
        str = str + Uint(cmd.size()).toString();
        str.serializeOut(client->socket());
        finishCmd(client);
        clientDisconnect(client);
        return;
    }

    SchedulerConfiguration* schedulerConfig = (SchedulerConfiguration*)cmd[1];
    Scheduler* scheduler = (Scheduler*)cmd[2];
    Array& objectives = (Array&)cmd(3);
    Array& evalConfigs = (Array&)cmd(4);
    Array& jobs = (Array&)cmd(5);
    Array& jobGroups = (Array&)cmd(6);
    Array& precedenceCts = (Array&)cmd(7);
    Array& resources = (Array&)cmd(8);
    Array& resourceGroups = (Array&)cmd(9);
    Array& resourceSequenceLists = (Array&)cmd(10);

    // so we don't have to clone objectives
    objectives.setOwner(false);

    // init Clevor data-set
    //the following line is
    // the main difference from cse::Server::handle_initSimpleRun
    MRPdataSet* dataSet = client->mrpRun()->dataSet();
    ASSERTD(dataSet != nullptr);
    dataSet->clearProblemData();
    initClevorDataSet(
        schedulerConfig,
        jobs,
        jobGroups,
        precedenceCts,
        resources,
        resourceGroups,
        resourceSequenceLists,
        *dataSet);

    // configure scheduler
    scheduler = scheduler->clone();
    scheduler->setConfig(schedulerConfig->clone());

    // init objectives
    std::vector<Objective*> objectiveVector;
    copyCollection(objectiveVector, objectives);
    initObjectives(schedulerConfig, objectiveVector, evalConfigs);

#ifdef DEBUG_UNIT
        utl::cout << "   Input Jobs: size:"
                  << jobs.size()
                  << utl::endlf;
        forEachIt(Array, jobs, Job, job)
            utl::cout << "   " << job.toString() << utl::endlf;
        endForEach
#endif

    // initialize simple run
    bool result = true;
    utl::String str;
    try
    {
        client->run()->initialize(dataSet, scheduler, objectiveVector);
        //the following line is
        // the main difference from cse::Server::handle_initSimpleRun
        client->run()->context()->setDataSetOwner(false);
    }
    catch (ConfigEx&)
    {
        result = false;
        str = "configuration error";
    }
    catch (FailEx& failEx)
    {
        result = false;
        if (failEx.str() != nullptr) str = *failEx.str();
    }

    // write result (and error message if initialization failed)
    Bool(result).serializeOut(client->socket());
    if (!result)
    {
        str.serializeOut(client->socket());
        finishCmd(client);
        clientDisconnect(client);
    }
    else
    {
        finishCmd(client);
    }
    return;
}

///////////////////////////////////////////////////////////////////////////////

void
MRPserver::handle_initOptimizer(MRPclient* client, const Array& cmd)
{
#ifdef DEBUG_UNIT
    utl::cout << utl::endl
              << "MRPserver::handle_initOptimizer."
              << "cmd.size():" << cmd.size()
              << utl::endlf;
#endif

    if ((cmd.size() != 11)

        || !cmd(1).isA(SchedulerConfiguration)

        || !cmd(2).isA(Optimizer)

        || !cmd(3).isA(OptimizerConfiguration)

        || !cmd(4).isA(Array)
        || !allAre(cmd(4), CLASS(ScheduleEvaluatorConfiguration))

        || !cmd(5).isA(Array)
        || !allAre(cmd(5), CLASS(Job))

        || !cmd(6).isA(Array)
        || !allAre(cmd(6), CLASS(JobGroup))

        || !cmd(7).isA(Array)
        || !allAre(cmd(7), CLASS(PrecedenceCt))

        || !cmd(8).isA(Array)
        || !allAre(cmd(8), CLASS(cse::Resource))

        || !cmd(9).isA(Array)
        || !allAre(cmd(9), CLASS(cse::ResourceGroup))

        || !cmd(10).isA(Array)
        || !allAre(cmd(10), CLASS(ResourceSequenceList)))
    {
        Bool(false).serializeOut(client->socket());
        utl::String str = "initOptimizer error: wrong cmd.";
        str.serializeOut(client->socket());
        finishCmd(client);
        clientDisconnect(client);
        return;
    }

    SchedulerConfiguration* schedulerConfig = (SchedulerConfiguration*)cmd[1];
    Optimizer* optimizer = (Optimizer*)cmd[2]->clone();
    OptimizerConfiguration* optimizerConfig = (OptimizerConfiguration*)cmd[3];
    optimizerConfig->setInd(new StringInd<uint_t>());
    Array& evalConfigs = (Array&)cmd(4);
    Array& jobs = (Array&)cmd(5);
    Array& jobGroups = (Array&)cmd(6);
    Array& precedenceCts = (Array&)cmd(7);
    Array& resources = (Array&)cmd(8);
    Array& resourceGroups = (Array&)cmd(9);
    Array& resourceSequenceLists = (Array&)cmd(10);

    // schedulerConfig
    MRPdataSet* dataSet = client->mrpRun()->dataSet();
    ASSERTD(dataSet != nullptr);
    initClevorDataSet(
        schedulerConfig,
        jobs,
        jobGroups,
        precedenceCts,
        resources,
        resourceGroups,
        resourceSequenceLists,
        *dataSet);

    // init scheduler
    Scheduler* scheduler = (Scheduler*)optimizerConfig->indBuilder();
    scheduler->setConfig(schedulerConfig->clone());

    // init objectives
    initObjectives(
        schedulerConfig,
        optimizerConfig->objectives(),
        evalConfigs);

#ifdef DEBUG_UNIT
        utl::cout << "   Input Jobs: size:"
                  << jobs.size()
                  << utl::endlf;
        forEachIt(Array, jobs, Job, job)
            utl::cout << "   " << job.toString() << utl::endlf;
        endForEach
#endif

    // initialize optimization run
    bool res = true;
    utl::String str;
    try
    {
        client->run()->initialize(dataSet, optimizer, optimizerConfig);
        client->run()->context()->setDataSetOwner(false);
    }
    catch (ConfigEx&)
    {
        res = false;
        str = "configuration error";
    }
    catch (FailEx& failEx)
    {
        res = false;
        if (failEx.str() != nullptr) str = *failEx.str();
    }

    Bool(res).serializeOut(client->socket());
    if (!res)
    {
        str.serializeOut(client->socket());
        finishCmd(client);
        clientDisconnect(client);
    }
    else
    {
        finishCmd(client);
    }
    return;
}

///////////////////////////////////////////////////////////////////////////////

// void
// MRPserver::handle_initOptimizer(MRPclient* client, const Array& cmd)
// {
//     // type checking
//     if ((cmd.size() != 5)

//         || !cmd(1).isA(SchedulerConfiguration)

//         || !cmd(2).isA(Optimizer)

//         || !cmd(3).isA(OptimizerConfiguration)

//         || !cmd(4).isA(Array)
//         || !allAre(cmd(4), CLASS(ScheduleEvaluatorConfiguration)))
//     {
//         Bool(false).serializeOut(client->socket());
//         utl::String str = "initScheduler error: wrong cmd.";
//         str.serializeOut(client->socket());
//         finishCmd(client);
//         clientDisconnect(client);//?
//         return;
//     }

//     SchedulerConfiguration* schedulerConfig = (SchedulerConfiguration*)cmd[1];
//     Optimizer* optimizer = (Optimizer*)cmd[2]->clone();
//     OptimizerConfiguration* optimizerConfig = (OptimizerConfiguration*)cmd[3];
//     optimizerConfig->setInd(new StringInd<uint_t>());
//     Array& evalConfigs = (Array&)cmd(4);

//     // schedulerConfig
//     MRPdataSet* dataSet = client->mrpRun()->dataSet();
//     ASSERTD(dataSet != nullptr);
//     dataSet->ClevorDataSet::set(schedulerConfig->clone());

//     // init scheduler
//     Scheduler* scheduler = (Scheduler*)optimizerConfig->indBuilder();
//     scheduler->setConfig(schedulerConfig->clone());

//     // init objectives
//     initObjectives(
//         schedulerConfig,
//         optimizerConfig->objectives(),
//         evalConfigs);

//     // initialize optimization run
//     bool res = true;
//     utl::String str;
//     try
//     {
//         client->run()->initialize(dataSet, optimizer, optimizerConfig);
//         client->run()->context()->setDataSetOwner(false);
//     }
//     catch (ConfigEx&)
//     {
//         res = false;
//         str = "configuration error";
//     }
//     catch (FailEx& failEx)
//     {
//         res = false;
//         if (failEx.str() != nullptr) str = *failEx.str();
//     }

//     Bool(res).serializeOut(client->socket());
//     if (!res) str.serializeOut(client->socket());
//     finishCmd(client);
// }

///////////////////////////////////////////////////////////////////////////////

void
MRPserver::init()
{
    addHandler("initMRP", &MRPserver::handle_initMRP);
    addHandler("generateJobs", &MRPserver::handle_generateJobs);
    addHandler("getJobs", &MRPserver::handle_getJobs);
    addHandler("initScheduler", &MRPserver::handle_initScheduler);
    addHandler("initOptimizer", &MRPserver::handle_initOptimizer);
}

///////////////////////////////////////////////////////////////////////////////

MRP_NS_END;
