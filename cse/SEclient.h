#ifndef CSE_SECLIENT_H
#define CSE_SECLIENT_H

/////////////////////////////////////////////////////////////////////////////////////////////////////

#include <libutl/BufferedFileStream.h>
#include <libutl/BufferedTCPsocket.h>
#include <libutl/NetCmdServer.h>
#include <libutl/TCPsocket.h>

/////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

/////////////////////////////////////////////////////////////////////////////////////////////////////

class Server;
class SchedulingRun;
class RunningThread;

/////////////////////////////////////////////////////////////////////////////////////////////////////

class SEclient : public utl::NetServerClient
{
    UTL_CLASS_DECL(SEclient);

public:
    SEclient(Server* server,
             utl::FDstream* socket,
             const utl::InetHostAddress& addr,
             bool recording = false);

    bool&
    authorized()
    {
        return _authorized;
    }

    utl::uint32_t
    challenge() const
    {
        return _challenge;
    }

    utl::uint32_t
    response() const
    {
        return _response;
    }

    void createRunningThread();

    void deleteRunningThread();

    RunningThread*
    runningThread()
    {
        return _runningThread;
    }

    SchedulingRun*
    run()
    {
        return _run;
    }

    bool
    hasCommandLog() const
    {
        return (_commandLog != nullptr);
    }

    utl::BufferedFileStream&
    commandLog()
    {
        ASSERTD(_commandLog != nullptr);
        return *_commandLog;
    }

    static utl::uint32_t response(utl::uint32_t challenge);

private:
    void
    init()
    {
        ABORT();
    }

    void deInit();

    void makeChallengeResponsePair();

private:
    bool _authorized;
    utl::uint32_t _challenge;
    utl::uint32_t _response;

    RunningThread* _runningThread;
    SchedulingRun* _run;
    utl::BufferedFileStream* _commandLog;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;

/////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
