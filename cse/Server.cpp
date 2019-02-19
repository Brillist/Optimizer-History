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
#include "RunningThread.h"
#include "Scheduler.h"
#include "ScheduleEvaluatorConfiguration.h"
#include "SchedulingRun.h"
#include "SEclient.h"
#include "Server.h"
#include "TimeSlot.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;
GOP_NS_USE;
CLP_NS_USE;
CLS_NS_USE;

/////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(cse::Server);

/////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

/////////////////////////////////////////////////////////////////////////////////////////////////////

void*
Server::run(void* arg)
{
    while (!_exit)
    {
        listen();
    }
    return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

NetServerClient*
Server::clientMake(FDstream* socket, const InetHostAddress& addr)
{
    return new SEclient(this, socket, addr, _recording);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

void
Server::clientWriteServerBusyMsg(Stream& os)
{
    os << "001 maximum number of clients would be exceeded." << endlf;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

void
Server::onClientConnect(NetServerClient* p_client)
{
    auto client = utl::cast<SEclient>(p_client);
    client->setSocket(new BufferedStream(client->socket()));
    auto& clientAddr = client->addr();
    auto& clientSocket = client->socket();
    clientSocket << "000 CSE-Server welcomes client at " << clientAddr << endlf;
    // also write the challenge ...
    Uint(client->challenge()).serializeOut(clientSocket);
    clientSocket.flush();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

void
Server::onClientDisconnect(NetServerClient* client)
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

void
Server::addHandler(const char* cmd, hfn handler)
{
    _handlers.insert(handler_map_t::value_type(std::string(cmd), handler));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

void
Server::handleCmd(NetServerClient* client, const Array& cmd)
{
    if (!cmd(0).isA(utl::String))
    {
        clientDisconnect(client);
        return;
    }

    auto seClient = utl::cast<SEclient>(client);
    std::string cmdStr = utl::cast<utl::String>(cmd(0)).get();

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

/////////////////////////////////////////////////////////////////////////////////////////////////////

void
Server::handle_authorizeClient(SEclient* client, const utl::Array& cmd)
{
    // must be only one argument
    if ((cmd.size() != 2) || !cmd(1).isA(Uint))
    {
        clientDisconnect(client);
        return;
    }

    auto& response = utl::cast<Uint>(cmd(1));
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

/////////////////////////////////////////////////////////////////////////////////////////////////////

void
Server::handle_exit(SEclient* client, const utl::Array& cmd)
{
    _exit = true;
    finishCmd(client);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

void
Server::handle_initSimpleRun(SEclient* client, const Array& cmd)
{
    // type checking
    if ((cmd.size() != 11) || !cmd(1).isA(SchedulerConfiguration) || !cmd(2).isA(Scheduler) ||
        !cmd(3).isA(Array) || !allAre(cmd(3), CLASS(Objective)) || !cmd(4).isA(Array) ||
        !allAre(cmd(4), CLASS(ScheduleEvaluatorConfiguration)) || !cmd(5).isA(Array) ||
        !allAre(cmd(5), CLASS(Job)) || !cmd(6).isA(Array) || !allAre(cmd(6), CLASS(JobGroup)) ||
        !cmd(7).isA(Array) || !allAre(cmd(7), CLASS(PrecedenceCt)) || !cmd(8).isA(Array) ||
        !allAre(cmd(8), CLASS(Resource)) || !cmd(9).isA(Array) ||
        !allAre(cmd(9), CLASS(ResourceGroup)) || !cmd(10).isA(Array) ||
        !allAre(cmd(10), CLASS(ResourceSequenceList)))
    {
        Bool(false).serializeOut(client->socket());
        utl::String str = "initSimpleRun error: wrong cmd.";
        str = str + Uint(cmd.size()).toString();
        str.serializeOut(client->socket());
        finishCmd(client);
        clientDisconnect(client);
        return;
    }

    auto schedulerConfig = utl::cast<SchedulerConfiguration>(cmd[1]);
    auto scheduler = utl::cast<Scheduler>(cmd[2]);
    auto& objectives = utl::cast<Array>(cmd(3));
    auto& evalConfigs = utl::cast<Array>(cmd(4));
    auto& jobs = utl::cast<Array>(cmd(5));
    auto& jobGroups = utl::cast<Array>(cmd(6));
    auto& precedenceCts = utl::cast<Array>(cmd(7));
    auto& resources = utl::cast<Array>(cmd(8));
    auto& resourceGroups = utl::cast<Array>(cmd(9));
    auto& resourceSequenceLists = utl::cast<Array>(cmd(10));

    // so we don't have to clone objectives
    objectives.setOwner(false);

    // create data-set
    ClevorDataSet* dataSet = new ClevorDataSet();
    initClevorDataSet(schedulerConfig, jobs, jobGroups, precedenceCts, resources, resourceGroups,
                      resourceSequenceLists, *dataSet);

    // configure scheduler
    scheduler = scheduler->clone();
    scheduler->setConfig(schedulerConfig->clone());

    // init objectives
    std::vector<Objective*> objectiveVector;
    for (auto obj_ : objectives)
    {
        auto obj = utl::cast<Objective>(obj_);
        objectiveVector.push_back(obj);
    }
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
        if (failEx.str() != nullptr)
            str = *failEx.str();
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

/////////////////////////////////////////////////////////////////////////////////////////////////////

void
Server::handle_initOptimizerRun(SEclient* client, const Array& cmd)
{
    // type checking
    if ((cmd.size() != 11) || !cmd(1).isA(SchedulerConfiguration) || !cmd(2).isA(Optimizer) ||
        !cmd(3).isA(OptimizerConfiguration) || !cmd(4).isA(Array) ||
        !allAre(cmd(4), CLASS(ScheduleEvaluatorConfiguration)) || !cmd(5).isA(Array) ||
        !allAre(cmd(5), CLASS(Job)) || !cmd(6).isA(Array) || !allAre(cmd(6), CLASS(JobGroup)) ||
        !cmd(7).isA(Array) || !allAre(cmd(7), CLASS(PrecedenceCt)) || !cmd(8).isA(Array) ||
        !allAre(cmd(8), CLASS(Resource)) || !cmd(9).isA(Array) ||
        !allAre(cmd(9), CLASS(ResourceGroup)) || !cmd(10).isA(Array) ||
        !allAre(cmd(10), CLASS(ResourceSequenceList)))
    {
        clientDisconnect(client);
        return;
    }

    auto schedulerConfig = utl::cast<SchedulerConfiguration>(cmd[1]);
    auto optimizer = utl::cast<Optimizer>(cmd[2]->clone());
    auto optimizerConfig = utl::cast<OptimizerConfiguration>(cmd[3]);
    optimizerConfig->setInd(new StringInd<uint_t>());
    auto& evalConfigs = utl::cast<Array>(cmd(4));
    auto& jobs = utl::cast<Array>(cmd(5));
    auto& jobGroups = utl::cast<Array>(cmd(6));
    auto& precedenceCts = utl::cast<Array>(cmd(7));
    auto& resources = utl::cast<Array>(cmd(8));
    auto& resourceGroups = utl::cast<Array>(cmd(9));
    auto& resourceSequenceLists = utl::cast<Array>(cmd(10));

    // create data-set
    auto dataSet = new ClevorDataSet();
    initClevorDataSet(schedulerConfig, jobs, jobGroups, precedenceCts, resources, resourceGroups,
                      resourceSequenceLists, *dataSet);

    // init scheduler
    auto scheduler = utl::cast<Scheduler>(optimizerConfig->indBuilder());
    scheduler->setConfig(schedulerConfig->clone());

    // init objectives
    initObjectives(schedulerConfig, optimizerConfig->objectives(), evalConfigs);

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
        if (failEx.str() != nullptr)
            str = *failEx.str();
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

/////////////////////////////////////////////////////////////////////////////////////////////////////

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
            client->run()->run();
        }
        catch (FailEx& failEx)
        {
            res = false;
            if (failEx.str() != nullptr)
                str = *failEx.str();
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

/////////////////////////////////////////////////////////////////////////////////////////////////////

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
        optimizer->runStatus()->get(complete, iteration, bestIter, bestScore);
        Bool(complete).serializeOut(client->socket());
        Uint(iteration).serializeOut(client->socket());
        Uint(bestIter).serializeOut(client->socket());
        Score(*bestScore).serializeOut(client->socket());
        if (complete)
            client->deleteRunningThread();
    }
    finishCmd(client);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

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
        auto& objectiveIdx = utl::cast<Uint>(cmd(1));
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
        auto& objectiveName = utl::cast<utl::String>(cmd(1));
        bestScore = client->run()->bestScore(objectiveName.get());
    }

    bestScore->serializeOut(client->socket());
    finishCmd(client);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

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

/////////////////////////////////////////////////////////////////////////////////////////////////////

void
Server::handle_getBestScoreAuditReport(SEclient* client, const utl::Array& cmd)
{
    if (cmd.size() != 1)
    {
        clientDisconnect(client);
        return;
    }

    auto report = client->run()->bestScoreAuditReport();
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

/////////////////////////////////////////////////////////////////////////////////////////////////////

void
Server::handle_getBestScoreComponent(SEclient* client, const utl::Array& cmd)
{
    if ((cmd.size() != 3) || !cmd(1).isA(utl::String) || !cmd(2).isA(utl::String))
    {
        clientDisconnect(client);
        return;
    }

    auto& objectiveName = utl::cast<utl::String>(cmd(1));
    auto& componentName = utl::cast<utl::String>(cmd(2));

    Float bestScore;
    std::string objName = objectiveName.get();
    std::string compName = componentName.get();

    bestScore = client->run()->bestScoreComponent(objName, compName);
    bestScore.serializeOut(client->socket());
    finishCmd(client);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

void
Server::handle_getBestSchedule(SEclient* client, const utl::Array& cmd)
{
    if (cmd.size() != 1)
    {
        clientDisconnect(client);
        return;
    }

    auto run = client->run();
    auto context = run->context();
    auto dataSet = context->clevorDataSet();
    auto& socket = client->socket();

    // write resource costs
    uint_t resId;
    double resCost;
    auto& resources = dataSet->resources();
    res_set_id_t::const_iterator resIt;
    for (resIt = resources.begin(); resIt != resources.end(); ++resIt)
    {
        auto res = *resIt;
        if (!res->isA(cse::DiscreteResource))
        {
            continue;
        }
        auto dres = utl::cast<cse::DiscreteResource>(res);
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

    auto& ops = dataSet->ops();
    jobop_set_id_t::const_iterator opIt;
    uint_t numOps = ops.size();
    utl::serialize(numOps, socket, io_wr);
    for (opIt = ops.begin(); opIt != ops.end(); ++opIt)
    {
        auto op = *opIt;

        // op id
        utl::serialize(op->id(), socket, io_wr);

        // scheduled?
        bool scheduled = !op->ignorable() && op->isScheduled() && (op->scheduledBy() == sa_clevor);
        utl::serialize(scheduled, socket, io_wr);
        if (!scheduled)
            continue;

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
            auto rgr = op->getResGroupReq(i);
            utl::serialize(rgr->scheduledResourceId(), socket, io_wr);
            utl::serialize(rgr->scheduledCapacity(), socket, io_wr);
        }
    }
    finishCmd(client);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

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

/////////////////////////////////////////////////////////////////////////////////////////////////////

void
Server::handle_getTimetable(SEclient* client, const utl::Array& cmd)
{
    if ((cmd.size() != 2) || !cmd(1).isA(Uint))
    {
        clientDisconnect(client);
        return;
    }

    auto& resId = utl::cast<Uint>(cmd(1));
    auto run = client->run();
    auto context = run->context();
    auto dataSet = context->clevorDataSet();

    // find resource
    auto cseRes = dataSet->findResource(resId);
    if ((cseRes == nullptr) || !cseRes->isA(cse::DiscreteResource))
    {
        Array().serializeOut(client->socket());
        return;
    }
    auto cseDres = utl::cast<cse::DiscreteResource>(cseRes);
    auto clsDres = utl::cast<cls::DiscreteResource>(cseDres->clsResource());

    // build list of time-slots
    Array timeSlots;
    auto& tt = clsDres->timetable();
    auto head = tt.head();
    auto tail = tt.tail()->prev();
    while (head->max() < 0)
        head = head->next();
    for (auto ts = head; ts != tail; ts = ts->next())
    {
        int beginTS = ts->min();
        int endTS = ts->max() + 1;
        uint_t reqCap = ts->v0();
        uint_t prvCap = ts->v1();
        TimeSlot* timeSlot = new TimeSlot(context->timeSlotToTime(beginTS),
                                          context->timeSlotToTime(endTS), reqCap, prvCap);
        timeSlots += timeSlot;
    }

    timeSlots.serializeOut(client->socket());
    finishCmd(client);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

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

/////////////////////////////////////////////////////////////////////////////////////////////////////

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
    finishCmd(client);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

void
Server::finishCmd(SEclient* client)
{
    utl::String("___END_RESPONSE___").serializeOut(client->socket());
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

bool
Server::allAre(const Object* object, const RunTimeClass* rtc) const
{
    if (!object->isA(Collection))
        return false;
    auto& col = utl::cast<Collection>(*object);
    for (auto obj : col)
    {
        if (!obj->_isA(rtc))
            return false;
    }
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

void
Server::initClevorDataSet(SchedulerConfiguration* schedulerConfig,
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
    for (auto job : jobs)
    {
        dataSet.add(utl::cast<Job>(job));
    }

    // job groups
    for (auto jobGroup : jobGroups)
    {
        dataSet.add(utl::cast<JobGroup>(jobGroup));
    }

    // precedence-cts
    for (auto pct : precedenceCts)
    {
        dataSet.add(utl::cast<PrecedenceCt>(pct));
    }

    // resources
    for (auto resource : resources)
    {
        dataSet.add(utl::cast<Resource>(resource));
    }

    // resource groups
    for (auto resourceGroup : resourceGroups)
    {
        dataSet.add(utl::cast<ResourceGroup>(resourceGroup));
    }

    // resource sequence lists
    for (auto rsl : resourceSequenceLists)
    {
        dataSet.add(utl::cast<ResourceSequenceList>(rsl));
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

void
Server::initObjectives(SchedulerConfiguration* schedulerConfig,
                       const objective_vector_t& objectives,
                       Array& evalConfigs)
{
    uint_t objIdx = 0;
    for (auto evalConfig_ : evalConfigs)
    {
        auto evalConfig = utl::cast<ScheduleEvaluatorConfiguration>(evalConfig_);
        evalConfig->setSchedulerConfig(schedulerConfig->clone());
        auto objective = objectives[objIdx++];
        objective->indEvaluator()->initialize(evalConfig);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

void
Server::init(bool recording)
{
    _exit = false;
    _recording = recording;
    addHandler("authorizeClient", &Server::handle_authorizeClient);
    addHandler("exit", &Server::handle_exit);
    addHandler("initSimpleRun", &Server::handle_initSimpleRun);
    addHandler("initOptimizerRun", &Server::handle_initOptimizerRun);
    addHandler("run", &Server::handle_run);
    addHandler("getRunStatus", &Server::handle_getRunStatus);
    addHandler("getBestScore", &Server::handle_getBestScore);
    addHandler("getBestScoreAudit", &Server::handle_getBestScoreAudit);
    addHandler("getBestScoreAuditReport", &Server::handle_getBestScoreAuditReport);
    addHandler("getBestScoreComponent", &Server::handle_getBestScoreComponent);
    addHandler("getBestSchedule", &Server::handle_getBestSchedule);
    addHandler("getMakespan", &Server::handle_getMakespan);
    addHandler("getTimetable", &Server::handle_getTimetable);
    addHandler("NOP", &Server::handle_NOP);
    addHandler("stop", &Server::handle_stop);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;
