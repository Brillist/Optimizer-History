#include "libcse.h"
#include <libutl/HostOS.h>
#include <libutl/NetServer.h>
#include <libutl/R250.h>
#include "Server.h"
#include "SchedulingRun.h"
#include "RunningThread.h"
#include "SEclient.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;

/////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(cse::SEclient, utl::NetServerClient);

/////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

/////////////////////////////////////////////////////////////////////////////////////////////////////

static utl::uint_t numKeys = 4;
static utl::uint32_t keys[] = {0xe850a09c, 0xfae9751b, 0xe9b7a73d, 0xdeadbeef};

/////////////////////////////////////////////////////////////////////////////////////////////////////

SEclient::SEclient(Server* server,
                   utl::FDstream* socket,
                   const utl::InetHostAddress& addr,
                   bool recording)
    : NetServerClient(server, socket, addr)
{
    makeChallengeResponsePair();
    _runningThread = nullptr;
    _run = new SchedulingRun();
    _commandLog = nullptr;
    if (recording)
    {
        String clientHost;
        try
        {
            clientHost = addr.name();
        }
        catch (HostNotFoundEx&)
        {
            clientHost = addr.toString();
        }
        Pathname path = String("/work/cse-commands-") + clientHost;
        try
        {
            _commandLog = new BufferedFileStream(path, io_wr | fs_create | fs_trunc, 0644);
        }
        catch (Exception& ex)
        {
            ex.dump(cerr);
            ASSERTD(_commandLog == nullptr);
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

void
SEclient::createRunningThread()
{
    ASSERTD(_runningThread == nullptr);
    //     _runningThread = new RunningThread(this);
    _runningThread = new RunningThread();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

void
SEclient::deleteRunningThread()
{
    ASSERTD(_runningThread != nullptr);
    _runningThread->join();
    _runningThread = nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

utl::uint32_t
SEclient::response(utl::uint32_t challenge)
{
    // count number of 1's in challenge
    uint_t numberOfOnes = 0;
    for (utl::uint32_t tmp = challenge; tmp != 0; tmp >>= 1)
    {
        if ((tmp & 1) != 0)
            ++numberOfOnes;
    }

    // compute response
    utl::uint32_t response = challenge;
    uint_t idx = numberOfOnes % numKeys;
    uint_t numXORs = (numberOfOnes % (numKeys - 1)) + 1;
    while (numXORs-- > 0)
    {
        response ^= keys[idx];
        if (++idx == numKeys)
            idx = 0;
    }

    return response;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

void
SEclient::deInit()
{
    if (_run)
        _run->stop();
    if (_runningThread != nullptr)
    {
        _runningThread->join();
    }
    delete _run;
    delete _commandLog;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

void
SEclient::makeChallengeResponsePair()
{
    // initially not authorized
    _authorized = false;

    // pseudo-randomly choose challenge bit-pattern
    _challenge = R250(uint32_t_max).eval();

    // calculate the required response
    _response = response(_challenge);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;
