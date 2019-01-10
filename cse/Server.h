#ifndef CSE_SERVER_H
#define CSE_SERVER_H

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
    Provide command interface to client.

    \author Adam McKee
*/

/////////////////////////////////////////////////////////////////////////////////////////////////////

class Server : public utl::NetCmdServer
{
    UTL_CLASS_DECL(Server);
    UTL_CLASS_NO_COPY;
    UTL_CLASS_NO_SERIALIZE;

public:
    Server(utl::uint_t maxClients, bool recording = false)
        : utl::NetCmdServer(maxClients, 0)
    {
        init(recording);
    }

    virtual void* run(void* arg = nullptr);

protected:
    typedef void (Server::*hfn)(SEclient* client, const utl::Array& cmd);
    typedef std::map<std::string, hfn> handler_map_t;

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
    virtual void handleCmd(utl::NetServerClient* client, const utl::Array& cmd);

    //@{
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
    void init(bool recording = false);
    void
    deInit()
    {
    }

private:
    bool _exit;
    handler_map_t _handlers;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;

/////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
