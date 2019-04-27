#pragma once

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

/**
   Scheduling engine client.

   There's an instance of SEclient for each connected client.

   \see Server
   \ingroup cse
*/

/////////////////////////////////////////////////////////////////////////////////////////////////////

class SEclient : public utl::NetServerClient
{
    UTL_CLASS_DECL(SEclient, NetServerClient);

public:
    /**
       Constructor.
       \param server associated Server
       \param socket the stream used to communicate with the client
       \param addr the client's address
       \param recording record commands? (default = false)
    */
    SEclient(Server* server,
             utl::FDstream* socket,
             const utl::InetHostAddress& addr,
             bool recording = false);

    virtual void pause();

    /** Create a RunningThread to perform an optimization run for this client. */
    void createRunningThread();

    /** Join on this client's RunningThread (see utl::Thread::join). */
    void deleteRunningThread();

    /// \name Accessors (const)
    //@{
    /** Get authorized flag. */
    bool
    authorized() const
    {
        return _authorized;
    }

    /** Get the challenge issued to the client upon connection. */
    uint32_t
    challenge() const
    {
        return _challenge;
    }

    /** Get the response to the challenge that the client must provide for authorization. */
    uint32_t
    response() const
    {
        return _response;
    }

    /** Get the RunningThread for the active optimization run (if any). */
    RunningThread*
    runningThread() const
    {
        return _runningThread;
    }

    /** Get the SchedulingRun for the active run (if any). */
    SchedulingRun*
    run() const
    {
        return _run;
    }

    /** Client has a command log? */
    bool
    hasCommandLog() const
    {
        return (_commandLog != nullptr);
    }

    /** Get the client's command log. */
    utl::BufferedFileStream&
    commandLog() const
    {
        ASSERTD(_commandLog != nullptr);
        return *_commandLog;
    }
    //@}

    /// \name Accessors (non-const)
    //@{
    /** Set the authorized flag. */
    void
    setAuthorized(bool authorized)
    {
        _authorized = authorized;
    }
    //@}

    static uint32_t response(uint32_t challenge);

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
    uint32_t _challenge;
    uint32_t _response;
    RunningThread* _runningThread;
    SchedulingRun* _run;
    utl::BufferedFileStream* _commandLog;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;
