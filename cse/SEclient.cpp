#include "libcse.h"
#include <libutl/HostOS.h>
#include <libutl/NetServer.h>
#include "Server.h"
#include "SchedulingRun.h"
#include "RunningThread.h"
#include "SEclient.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;

/////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(cse::SEclient);

/////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

/////////////////////////////////////////////////////////////////////////////////////////////////////

static uint_t numKeys = 4;
static uint32_t keys[] = {0xe850a09c, 0xfae9751b, 0xe9b7a73d, 0xdeadbeef};

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
SEclient::pause()
{
    super::pause();
    deInit();
    makeChallengeResponsePair();
    _run = new SchedulingRun();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

void
SEclient::createRunningThread()
{
    ASSERTD(_runningThread == nullptr);
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

uint32_t
SEclient::response(uint32_t challenge)
{
    // count number of 1's in challenge
    uint_t numberOfOnes = 0;
    for (uint32_t tmp = challenge; tmp != 0; tmp >>= 1)
    {
        if ((tmp & 1) != 0)
            ++numberOfOnes;
    }

    // compute response
    uint32_t response = challenge;
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
    if (_run != nullptr)
    {
        _run->stop();
    }
    if (_runningThread != nullptr)
    {
        _runningThread->join();
    }
    delete _run;
    _run = nullptr;
    _runningThread = nullptr;
    delete _commandLog;
    _commandLog = nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

void
SEclient::makeChallengeResponsePair()
{
    // initially not authorized
    _authorized = false;

    // pseudo-randomly choose challenge bit-pattern
    _challenge = lut::rng_t().uniform((uint32_t)0, uint32_t_max);

    // calculate the required response
    _response = response(_challenge);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;
