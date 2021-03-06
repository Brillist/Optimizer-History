#include "libmps.h"
#include <libutl/CmdLineArgs.h>
#include <libutl/Float.h>
#include <libutl/HostOS.h>
#include <libutl/LogMgr.h>
#include <libutl/TCPserverSocket.h>
#include <clp/IntSpan.h>
#include <mps/MPSserver.h>
#include "ServerApp.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;

/////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(mps::ServerApp);

/////////////////////////////////////////////////////////////////////////////////////////////////////

MPS_NS_BEGIN;

/////////////////////////////////////////////////////////////////////////////////////////////////////

int
ServerApp::run(int argc, char** argv)
{
    CmdLineArgs args(argc, argv);

    if (args.isSet("h"))
    {
        usage();
        return 0;
    }

    // daemon mode?
    if (args.isSet("d"))
    {
        hostOS->daemonInit();
    }

    // set listening port?
    uint_t port = 2000;
    String portStr;
    if (!args.isSet("p", portStr))
    {
        portStr = hostOS->getEnv("CSE_PORT");
    }
    if (!portStr.empty())
    {
        port = Uint(portStr);
    }

    // record commands?
    bool recording = args.isSet("r");

    // errors?
    if (args.printErrors(utl::cerr))
    {
        usage();
        return 1;
    }

    // set serialization mode
    setSerializeMode(ser_readable);

    // set up logging
#ifdef DEBUG
    logMgr.addStream(cerr, 0, 0);
    logMgr.setLevel(0, uint_t_max);
#endif

    MPSserver* server = new MPSserver(2, recording);
    TCPserverSocket* serverSocket = nullptr;

    // add server socket for network interface
    // (silently ignore any error)
    try
    {
        InetHostAddress hostAddr = InetHostname::localInetHostname().address();
        if (hostAddr != InetHostname("localhost").address())
        {
            serverSocket = new TCPserverSocket(hostAddr, port);
            server->addServer(serverSocket);
        }
    }
    catch (NetworkEx&)
    {
    }

    // add server socket for loopback interface
    // (this must succeed)
    try
    {
        serverSocket = new TCPserverSocket(InetHostname("localhost").address(), port);
        server->addServer(serverSocket);
    }
    catch (NetworkEx&)
    {
        utl::cerr << "failed to bind server socket -- another instance running?" << endl;
        delete server;
        return 1;
    }

    // run the server
    server->run();
    server->clientDisconnectAll();
    delete server;

    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

void
ServerApp::usage()
{
    utl::cout << "usage: clevor_se [-d] [-m] [-r] [-p <port>]" << endl;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

MPS_NS_END;
