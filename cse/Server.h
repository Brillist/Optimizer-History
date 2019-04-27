#pragma once

/////////////////////////////////////////////////////////////////////////////////////////////////////

#include <libutl/BufferedTCPsocket.h>
#include <libutl/NetCmdServer.h>
#include <gop/Objective.h>

/////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

/////////////////////////////////////////////////////////////////////////////////////////////////////

class ClevorDataSet;
class SchedulerConfiguration;
class SEclient;

/////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Provide network server interface.
   
   Server listens for client connections on the configured server sockets (see NetServer::addServer)
   and responds to commands issued by clients.  A command is represented as an Array where the first
   element is the name of the command (a utl::String), and remaining elements are the arguments to
   the command.

   The following commands are supported:

   ### authorizeClient

   ---

   A client must run this command to be authorized before it can run any other command.
   
   Argument: utl::Uint (the client's response to the challenge it received when it connected)

   Response: none (the client will be disconnected if it provides an incorrect response)

   ### exit

   ---

   Direct the CSE server application to exit.

   Arguments: none

   Response: none

   ### initSimpleRun

   ---

   Initialize a "simple" scheduling run to construct a forward schedule.

   Arguments:

   - cse::SchedulerConfiguration
   - cse::Scheduler
   - Array of gop::Objective%s
   - Array of cse::ScheduleEvaluatorConfiguration%s
   - Array of cse::Job%s
   - Array of cse::JobGroup%s
   - Array of cse::PrecedenceCt%s
   - Array of cse::Resource%s
   - Array of cse::ResourceGroup%s
   - Array of cse::ResourceSequenceList%s

   Response:

   - utl::Bool (true indicates success, false indicates failure)
   - utl::String containing error message (only in case of failure)

   ### initOptimizerRun

   ---

   Initialize an optimization run.

   Arguments:

   - cse::SchedulerConfiguration
   - gop::Optimizer
   - gop::OptimizerConfiguration
   - utl::Array of cse::ScheduleEvaluatorConfiguration%s
   - utl::Array of cse::Job%s
   - utl::Array of cse::JobGroup%s
   - utl::Array of cse::PrecedenceCt%s
   - utl::Array of cse::Resource%s
   - utl::Array of cse::ResourceGroup%s
   - utl::Array of cse::ResourceSequenceList%s

   Response:

   - utl::Bool (true indicates success, false indicates failure)
   - utl::String containing error message (only in case of failure)

   ### run

   ---

   If the client had previously issued an **initSimpleRun** command, generate a forward schedule.
   If the client had previously issued an **initOptimizerRun** command, spawn a thread to execute
   the optimization run.

   Arguments: none

   Response:

   - utl::Bool (true indicates success, false indicates failure)
   - utl::String containing error message (only in case of failure)

   ### getRunStatus

   ---

   Request status information for the current run.

   Arguments: none

   Response:

   - utl::Bool : completion status
   - utl::Uint : current iteration (= 1 for a simple run)
   - utl::Uint : best iteration (= 1 for a simple run)
   - cse::Score : best score

   ### getBestScore

   ---

   Provide the best score for the current run.

   Argument: utl::Uint (objective index) **OR** utl::String (objective name)

   Response: cse::Score

   ### getBestScoreAudit

   ---

   Provide the audit text for the schedule with the best score.

   Arguments: none

   Response: utl::String (the audit text)

   ### getBestScoreAuditReport

   ---

   Provide the audit report for the schedule with the best score.

   Arguments: none

   Response: cse::AuditReport

   ### getBestScoreComponent

   ---

   Provide a component score for the schedule with the best score.

   Arguments:

   - utl::String (objective name)
   - utl::String (component name)

   Response: utl::Float (the requested component score)

   ### getBestSchedule

   ---

   Provide cse::DiscreteResource costs (see cse::ResourceCost) and cse::JobOp scheduling
   information.

   Arguments: none

   Response:

   First, DiscreteResource costs are provided as a series of pairs:

   - utl::Uint (resource id)
   - utl::Float (resource cost)

   The resource cost information is terminated by sending a special pair:

   - utl::Uint (utl::uint_t_max as resource id)
   - utl::Float (-1.0 as resource cost)

   cse::JobOp scheduling information is provided next, beginning with:

   - utl::Uint (count of JobOp%s)

   Then, for each JobOp:

   - utl::Uint : scheduling agent (see \ref scheduling_agent_t)
   - utl::Bool : frozen flag
   - utl::Uint : processing time
   - utl::Uint : remaining processing time
   - utl::Int : start time (time_t)
   - utl::Int : end time (time_t)
   - utl::Int : resume time (time_t)
   - utl::Array of cse::ResourceRequirement%s
   - utl::Array of cse::ResourceGroupRequirement%s

   ### getMakespan

   ---

   Provide the makespan of the schedule with the best score.

   Arguments: none

   Response: utl::Int (makespan as time_t)

   ### getTimetable

   ---

   Provide the timetable of a cse::DiscreteResource.

   Arguments: utl::Uint (cse::DiscreteResource's id)

   Response: Array of cse::TimeSlot%s specifying required and provided capacity over time

   \ingroup cse
*/

/////////////////////////////////////////////////////////////////////////////////////////////////////

class Server : public utl::NetCmdServer
{
    UTL_CLASS_DECL(Server, utl::NetCmdServer);
    UTL_CLASS_NO_COPY;
    UTL_CLASS_NO_SERIALIZE;

public:
    /**
       Constructor.
       \param maxClients maximum number of connected clients
       \param recording record client commands to a file? (default = false)
    */
    Server(uint_t maxClients, bool recording = false)
        : utl::NetCmdServer(maxClients, 0)
    {
        init(recording);
    }

    virtual void* run(void* arg = nullptr);

protected:
    typedef void (Server::*hfn)(SEclient* client, const utl::Array& cmd);
    using handler_map_t = std::map<std::string, hfn>;

protected:
    virtual utl::NetServerClient* clientMake(utl::FDstream* socket,
                                             const utl::InetHostAddress& addr);
    virtual void clientWriteServerBusyMsg(utl::Stream& os);
    virtual void onClientConnect(utl::NetServerClient* client);
    virtual void onClientDisconnect(utl::NetServerClient* client);
    void addHandler(const char* cmd, hfn handler);
    bool allAre(const utl::Object* object, const utl::RunTimeClass* rtc) const;
    void finishCmd(SEclient* client);

    void initObjectives(SchedulerConfiguration* schedulerConfig,
                        const gop::objective_vector_t& objectives,
                        utl::Array& evalConfigs);

    void initClevorDataSet(SchedulerConfiguration* schedulerConfig,
                           utl::Array& jobs,
                           utl::Array& jobGroups,
                           utl::Array& precedenceCts,
                           utl::Array& resources,
                           utl::Array& resourceGroups,
                           utl::Array& resourceSequenceLists,
                           ClevorDataSet& dataSet);

protected:
    bool _recording;

private:
    void init(bool recording = false);
    void
    deInit()
    {
    }

    virtual void handleCmd(utl::NetServerClient* client, const utl::Array& cmd);
    void handle_authorizeClient(SEclient* client, const utl::Array& cmd);
    void handle_exit(SEclient* client, const utl::Array& cmd);
    void handle_initSimpleRun(SEclient* client, const utl::Array& cmd);
    void handle_initOptimizerRun(SEclient* client, const utl::Array& cmd);
    void handle_run(SEclient* client, const utl::Array& cmd);
    void handle_getRunStatus(SEclient* client, const utl::Array& cmd);
    void handle_getBestScore(SEclient* client, const utl::Array& cmd);
    void handle_getBestScoreAudit(SEclient* client, const utl::Array& cmd);
    void handle_getBestScoreAuditReport(SEclient* client, const utl::Array& cmd);
    void handle_getBestScoreComponent(SEclient* client, const utl::Array& cmd);
    void handle_getBestSchedule(SEclient* client, const utl::Array& cmd);
    void handle_getMakespan(SEclient* client, const utl::Array& cmd);
    void handle_getTimetable(SEclient* client, const utl::Array& cmd);
    void handle_NOP(SEclient* client, const utl::Array& cmd);
    void handle_stop(SEclient* client, const utl::Array& cmd);

private:
    bool _exit;
    handler_map_t _handlers;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;
