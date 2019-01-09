#include "libcse.h"
#include <libutl/Bool.h>
#include <libutl/Float.h>
#include <clp/FailEx.h>
#include <gop/ConfigEx.h>
#include <gop/Objective.h>
#include <gop/Optimizer.h>
#include <gop/Score.h>
#include "ClevorDataSet.h"
#include "DiscreteResource.h"
#include "Scheduler.h"
#include "ScheduleEvaluatorConfiguration.h"
#include "SchedulingRun.h"
#include "RunningThread.h"
#include "SEclient.h"
#include "TimeSlot.h"
#include "Server.h"

///////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;
GOP_NS_USE;
CLP_NS_USE;
CLS_NS_USE;

///////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(cse::Server, utl::NetCmdServer);

///////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

///////////////////////////////////////////////////////////////////////////////

void*
Server::run(void* arg)
{
    while (!_exit)
    {
        listen();
    }
    return nullptr;
}

///////////////////////////////////////////////////////////////////////////////

NetServerClient*
Server::clientMake(FDstream* socket, const InetHostAddress& addr)
{
    cse::Server* const server = this;
    return new SEclient(server, socket, addr, _recording);
}

///////////////////////////////////////////////////////////////////////////////

void
Server::clientWriteServerBusyMsg(Stream& os)
{
    os << "001 maximum number of clients would be exceeded." << endlf;
}

///////////////////////////////////////////////////////////////////////////////

void
Server::onClientConnect(NetServerClient* p_client)
{
    ASSERTD(p_client->isA(SEclient));
    SEclient* client = (SEclient*)p_client;
    client->setSocket(new BufferedStream(client->socket()));
    const InetHostAddress& clientAddr = client->addr();
    Stream& clientSocket = client->socket();
    clientSocket << "000 CSE-Server welcomes client at "
                 << clientAddr << endlf;
    // also write the challenge ...
    Uint(client->challenge()).serializeOut(clientSocket);
    clientSocket.flush();
}

///////////////////////////////////////////////////////////////////////////////

void
Server::onClientDisconnect(NetServerClient* client)
{
}

///////////////////////////////////////////////////////////////////////////////

void
Server::addHandler(const char* cmd, hfn handler)
{
    _handlers.insert(
        handler_map_t::value_type(
            std::string(cmd),
            handler));
}

///////////////////////////////////////////////////////////////////////////////

void
Server::handleCmd(NetServerClient* client, const Array& cmd)
{
    if (!cmd(0).isA(utl::String))
    {
        clientDisconnect(client);
        return;
    }

    ASSERTD(client->isA(SEclient));
    SEclient* seClient = (SEclient*)client;

    std::string cmdStr = ((const utl::String&)cmd(0)).get();

    // unauthorized client is only allowed to seek authorization
    if (!seClient->authorized() && (cmdStr != "authorizeClient"))
    {
        clientDisconnect(client);
        return;
    }

    // record the command
    if (_recording && seClient->hasCommandLog())
    {
        if ((cmdStr != "getRunStatus") && (cmdStr != "authorizeClient"))
        {
            cmd.serializeOut(seClient->commandLog());
            seClient->commandLog().flush();
        }
    }

    // no handler => unknown command
    handler_map_t::const_iterator it = _handlers.find(cmdStr);
    if (it == _handlers.end())
    {
        clientDisconnect(client);
        return;
    }

    // call the handler
    hfn handler = (*it).second;
    (this->*handler)(seClient, cmd);
}

///////////////////////////////////////////////////////////////////////////////

void
Server::handle_authorizeClient(SEclient* client, const utl::Array& cmd)
{
    // must be only one argument
    if ((cmd.size() != 2) || !cmd(1).isA(Uint))
    {
        clientDisconnect(client);
        return;
    }

    const Uint& response = (const Uint&)cmd(1);
    if (response == client->response())
    {
        client->authorized() = true;
        finishCmd(client);
    }
    else
    {
        // dismiss the client with no explanation
        clientDisconnect(client);
    }
}

///////////////////////////////////////////////////////////////////////////////

void
Server::handle_exit(SEclient* client, const utl::Array& cmd)
{
    _exit = true;
    finishCmd(client);
}

///////////////////////////////////////////////////////////////////////////////

void
Server::handle_initSimpleRun(SEclient* client, const Array& cmd)
{
    // type checking
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
        || !allAre(cmd(8), CLASS(Resource))

        || !cmd(9).isA(Array)
        || !allAre(cmd(9), CLASS(ResourceGroup))

        || !cmd(10).isA(Array)
        || !allAre(cmd(10), CLASS(ResourceSequenceList)))
    {
        Bool(false).serializeOut(client->socket());
        utl::String str = "initSimpleRun error: wrong cmd.";
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

    // create data-set
    ClevorDataSet* dataSet = new ClevorDataSet();
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

    // initialize simple run
    bool res = true;
    utl::String str;
    try
    {
        client->run()->initialize(dataSet, scheduler, objectiveVector);
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

    // write result (and error message if initialization failed)
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

void
Server::handle_initOptimizerRun(SEclient* client, const Array& cmd)
{
    // type checking
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
        || !allAre(cmd(8), CLASS(Resource))

        || !cmd(9).isA(Array)
        || !allAre(cmd(9), CLASS(ResourceGroup))

        || !cmd(10).isA(Array)
        || !allAre(cmd(10), CLASS(ResourceSequenceList)))
    {
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

    // create data-set
    ClevorDataSet* dataSet = new ClevorDataSet();
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

    // initialize optimization run
    bool res = true;
    utl::String str;
    try
    {
        client->run()->initialize(dataSet, optimizer, optimizerConfig);
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

void
Server::handle_run(SEclient* client, const utl::Array& cmd)
{
    if (cmd.size() != 1)
    {
        clientDisconnect(client);
        return;
    }

    bool res = true;
    utl::String str;

    // simple run
    if (client->run()->optimizer() == nullptr)
    {
        try
        {
//             client->run()->run(client);
            client->run()->run();
        }
        catch (FailEx& failEx)
        {
            res = false;
            if (failEx.str() != nullptr) str = *failEx.str();
        }
    }
    // optimization run
    else
    {
        ASSERTD(client->runningThread() == nullptr);
        client->createRunningThread();
        client->runningThread()->start(client->run());
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

void
Server::handle_getRunStatus(SEclient* client, const utl::Array& cmd)
{
    if (cmd.size() != 1)
    {
        clientDisconnect(client);
        return;
    }
    gop::Optimizer* optimizer = client->run()->optimizer();
    if (optimizer == nullptr)
    {
        Score* bestScore = client->run()->bestScore(0);
        Bool(true).serializeOut(client->socket());
        Uint(1).serializeOut(client->socket());
        Uint(1).serializeOut(client->socket());
        Score(*bestScore).serializeOut(client->socket());
    }
    else
    {
        bool complete;
        uint_t iteration;
        uint_t bestIter;
        Score* bestScore;
        optimizer->runStatus()->get(
            complete,
            iteration,
            bestIter,
            bestScore);
        Bool(complete).serializeOut(client->socket());
        Uint(iteration).serializeOut(client->socket());
        Uint(bestIter).serializeOut(client->socket());
        Score(*bestScore).serializeOut(client->socket());
        if (complete) client->deleteRunningThread();
    }
    finishCmd(client);
}

///////////////////////////////////////////////////////////////////////////////

void
Server::handle_getBestScore(SEclient* client, const utl::Array& cmd)
{
    if (cmd.size() != 2)
    {
        clientDisconnect(client);
        return;
    }

    Score* bestScore;

    // objective index?
    if (cmd(1).isA(Uint))
    {
        const Uint& objectiveIdx = (const Uint&)cmd(1);
        bestScore = client->run()->bestScore(objectiveIdx);
    }
    // objective name
    else
    {
        if (!cmd(1).isA(utl::String))
        {
            clientDisconnect(client);
            return;
        }
        const utl::String& objectiveName = (const utl::String&)cmd(1);
        bestScore = client->run()->bestScore(objectiveName.get());
    }

    bestScore->serializeOut(client->socket());
    finishCmd(client);
}

///////////////////////////////////////////////////////////////////////////////

void
Server::handle_getBestScoreAudit(SEclient* client, const utl::Array& cmd)
{
    if (cmd.size() != 1)
    {
        clientDisconnect(client);
        return;
    }

    // write the audit report (with trailing \0)
    const char* str = client->run()->bestScoreAudit().c_str();
    uint_t len = strlen(str) + 1;
    utl::serialize(len, client->socket(), io_wr);
    client->socket().write((byte_t*)str, len);

    finishCmd(client);
}

///////////////////////////////////////////////////////////////////////////////

void
Server::handle_getBestScoreAuditReport(
    SEclient* client,
    const utl::Array& cmd)
{
    if (cmd.size() != 1)
    {
        clientDisconnect(client);
        return;
    }

    const AuditReport* report = client->run()->bestScoreAuditReport();
    if (report == nullptr)
    {
        AuditReport().serializeOut(client->socket());
    }
    else
    {
        report->serializeOut(client->socket());
    }
    finishCmd(client);
}

///////////////////////////////////////////////////////////////////////////////

void
Server::handle_getBestScoreComponent(SEclient* client, const utl::Array& cmd)
{
    if ((cmd.size() != 3)
        || !cmd(1).isA(utl::String)
        || !cmd(2).isA(utl::String))
    {
        clientDisconnect(client);
        return;
    }

    const utl::String& objectiveName = (const utl::String&)cmd(1);
    const utl::String& componentName = (const utl::String&)cmd(2);

    Float bestScore;
    std::string objName = objectiveName.get();
    std::string compName = componentName.get();

    bestScore = client->run()->bestScoreComponent(objName, compName);
    bestScore.serializeOut(client->socket());
    finishCmd(client);
}

///////////////////////////////////////////////////////////////////////////////

void
Server::handle_getBestSchedule(
    SEclient* client,
    const utl::Array& cmd)
{
    if (cmd.size() != 1)
    {
        clientDisconnect(client);
        return;
    }

    const SchedulingRun* run = client->run();
    const SchedulingContext* context = run->context();
    const ClevorDataSet* dataSet = context->clevorDataSet();
    Stream& socket = client->socket();

    // write resource costs
    uint_t resId;
    double resCost;
    const res_set_id_t& resources = dataSet->resources();
    res_set_id_t::const_iterator resIt;
    for (resIt = resources.begin(); resIt != resources.end(); ++resIt)
    {
        cse::Resource* res = *resIt;
        if (!res->isA(cse::DiscreteResource))
        {
            continue;
        }
        cse::DiscreteResource* dres = (cse::DiscreteResource*)res;
        if (dres->cost() == nullptr)
        {
            continue;
        }
        resId = dres->id();
        resCost = dres->cost()->cost();
        utl::serialize(resId, socket, io_wr);
        utl::serialize(resCost, socket, io_wr);
    }
    resId = uint_t_max;
    resCost = -1.0;
    utl::serialize(resId, socket, io_wr);
    utl::serialize(resCost, socket, io_wr);

    const jobop_set_id_t& ops = dataSet->ops();
    jobop_set_id_t::const_iterator opIt;
    uint_t numOps = ops.size();
    utl::serialize(numOps, socket, io_wr);
    for (opIt = ops.begin(); opIt != ops.end(); ++opIt)
    {
        JobOp* op = *opIt;

        // op id
        utl::serialize(op->id(), socket, io_wr);

        // scheduled?
        bool scheduled
            = !op->ignorable()
            && op->isScheduled()
            && (op->scheduledBy() == sa_clevor);
        utl::serialize(scheduled, socket, io_wr);
        if (!scheduled)
        {
            continue;
        }

        // scheduling details
        utl::serialize((uint_t&)op->scheduledBy(), socket, io_wr);
        utl::serialize(op->frozen(), socket, io_wr);
        utl::serialize(op->scheduledProcessingTime(), socket, io_wr);
        utl::serialize(op->scheduledRemainingPt(), socket, io_wr);
        utl::serialize(op->scheduledStartTime(), socket, io_wr);
        utl::serialize(op->scheduledEndTime(), socket, io_wr);
        utl::serialize(op->scheduledResumeTime(), socket, io_wr);

        // resource requirements
        Array resReqs(false);
        uint_t numResReqs = op->numResReqs();
        uint_t i;
        for (i = 0; i < numResReqs; ++i)
        {
            resReqs += op->getResReq(i);
        }
        resReqs.serializeOut(socket);

        // resource group requirements
        uint_t numResGroupReqs = op->numResGroupReqs();
        for (i = 0; i < numResGroupReqs; ++i)
        {
            cse::ResourceGroupRequirement* rgr = op->getResGroupReq(i);
            utl::serialize(rgr->scheduledResourceId(), socket, io_wr);
            utl::serialize(rgr->scheduledCapacity(), socket, io_wr);
        }
    }
    finishCmd(client);
}

///////////////////////////////////////////////////////////////////////////////

void
Server::handle_getMakespan(SEclient* client, const utl::Array& cmd)
{
    if (cmd.size() != 1)
    {
        clientDisconnect(client);
        return;
    }

    // write makespan
    time_t makespan = client->run()->makespan();
    lut::serialize(makespan, client->socket(), io_wr);
    finishCmd(client);
}

///////////////////////////////////////////////////////////////////////////////

void
Server::handle_getTimetable(SEclient* client, const utl::Array& cmd)
{
    if ((cmd.size() != 2) || !cmd(1).isA(Uint))
    {
        clientDisconnect(client);
        return;
    }

    const Uint& resId = (const Uint&)cmd(1);
    const SchedulingRun* run = client->run();
    const SchedulingContext* context = run->context();
    const ClevorDataSet* dataSet = context->clevorDataSet();

    // find resource
    const cse::Resource* cseRes = dataSet->findResource(resId);
    if ((cseRes == nullptr) || !cseRes->isA(cse::DiscreteResource))
    {
        Array().serializeOut(client->socket());
        return;
    }
    const cse::DiscreteResource* cseDres = (cse::DiscreteResource*)cseRes;
    cls::DiscreteResource* clsDres = cseDres->clsResource();

    // build list of time-slots
    Array timeSlots;
    const DiscreteTimetable& tt = clsDres->timetable();
    const clp::IntSpan* ts;
    const clp::IntSpan* head = tt.head();
    const clp::IntSpan* tail = tt.tail()->prev();
    while (head->max() < 0) head = head->next();
    for (ts = head; ts != tail; ts = ts->next())
    {
        int beginTS = ts->min();
        int endTS = ts->max() + 1;
        uint_t reqCap = ts->v0();
        uint_t prvCap = ts->v1();
        TimeSlot* timeSlot =
            new TimeSlot(
                context->timeSlotToTime(beginTS),
                context->timeSlotToTime(endTS),
                reqCap,
                prvCap);
        timeSlots += timeSlot;
    }

    timeSlots.serializeOut(client->socket());
    finishCmd(client);
}

///////////////////////////////////////////////////////////////////////////////

void
Server::handle_NOP(SEclient* client, const utl::Array& cmd)
{
    if (cmd.size() != 1)
    {
        clientDisconnect(client);
        return;
    }

    Bool(true).serializeOut(client->socket());
    finishCmd(client);
}

///////////////////////////////////////////////////////////////////////////////

void
Server::handle_stop(SEclient* client, const utl::Array& cmd)
{
    if (cmd.size() != 1)
    {
        clientDisconnect(client);
        return;
    }
    if (client->runningThread() != nullptr)
    {
        client->run()->stop();
    }
    //clientEnableMsgs(client);
}

///////////////////////////////////////////////////////////////////////////////

void
Server::finishCmd(SEclient* client)
{
    utl::String("___END_RESPONSE___").serializeOut(client->socket());
}

///////////////////////////////////////////////////////////////////////////////

bool
Server::allAre(
    const Object* object,
    const RunTimeClass* rtc) const
{
    if (!object->isA(Collection))
    {
        return false;
    }
    const Collection* col = (const Collection*)object;
    Collection::iterator it;
    for (it = col->begin(); it != col->end(); ++it)
    {
        Object* obj = *it;
        if (!obj->_isA(rtc))
        {
            return false;
        }
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////

void
Server::initClevorDataSet(
    SchedulerConfiguration* schedulerConfig,
    Array& jobs,
    Array& jobGroups,
    Array& precedenceCts,
    Array& resources,
    Array& resourceGroups,
    Array& resourceSequenceLists,
    ClevorDataSet& dataSet)
{
    dataSet.set(schedulerConfig->clone());

    // so that we don't have to make copies
    jobs.setOwner(false);
    jobGroups.setOwner(false);
    precedenceCts.setOwner(false);
    resources.setOwner(false);
    resourceGroups.setOwner(false);
    resourceSequenceLists.setOwner(false);

    // jobs
    forEachIt(Array, jobs, Job, job)
        dataSet.add(job);
    endForEach

    // job groups
    forEachIt (Array, jobGroups, JobGroup, jobGroup)
        dataSet.add(jobGroup);
    endForEach

    // precedence-cts
    forEachIt(Array, precedenceCts, PrecedenceCt, pct)
        dataSet.add(pct);
    endForEach

    // resources
    forEachIt(Array, resources, Resource, resource)
        dataSet.add(resource);
    endForEach

    // resource-groups
    forEachIt(Array, resourceGroups, ResourceGroup, resourceGroup)
        dataSet.add(resourceGroup);
    endForEach

    // resource-sequence-lists
    forEachIt(Array, resourceSequenceLists, ResourceSequenceList, rsl)
        dataSet.add(rsl);
    endForEach
}

///////////////////////////////////////////////////////////////////////////////

void
Server::initObjectives(
    SchedulerConfiguration* schedulerConfig,
    const objective_vector_t& objectives,
    Array& evalConfigs)
{
    uint_t objIdx = 0;
    forEachIt(Array, evalConfigs, ScheduleEvaluatorConfiguration, evalConfig)
        evalConfig.setSchedulerConfig(schedulerConfig->clone());
        Objective* objective = objectives[objIdx++];
        objective->indEvaluator()->initialize(evalConfig);
    endForEach
}


///////////////////////////////////////////////////////////////////////////////

void
Server::init(bool recording)
{
    _exit = false;
    _recording = recording;
    addHandler("authorizeClient",   &Server::handle_authorizeClient);
    addHandler("exit",              &Server::handle_exit);
    addHandler("initSimpleRun",     &Server::handle_initSimpleRun);
    addHandler("initOptimizerRun",  &Server::handle_initOptimizerRun);
    addHandler("run",               &Server::handle_run);
    addHandler("getRunStatus",      &Server::handle_getRunStatus);
    addHandler("getBestScore",      &Server::handle_getBestScore);
    addHandler("getBestScoreAudit", &Server::handle_getBestScoreAudit);
    addHandler("getBestScoreAuditReport",
               &Server::handle_getBestScoreAuditReport);
    addHandler("getBestScoreComponent",
               &Server::handle_getBestScoreComponent);
    addHandler("getBestSchedule",   &Server::handle_getBestSchedule);
    addHandler("getMakespan",       &Server::handle_getMakespan);
    addHandler("getTimetable",      &Server::handle_getTimetable);
    addHandler("NOP",               &Server::handle_NOP);
    addHandler("stop",              &Server::handle_stop);
}

///////////////////////////////////////////////////////////////////////////////

CSE_NS_END;
