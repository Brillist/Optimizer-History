#include "main.h"
#include <libutl/Application.h>
#include <libutl/Bool.h>
#include <libutl/BufferedFileStream.h>
#include <libutl/BufferedTCPsocket.h>
#include <libutl/CmdLineArgs.h>
#include <libutl/HostOS.h>
#include <libutl/Pathname.h>
#include <libutl/Regex.h>
#include <libutl/Uint.h>
#include <cse/SEclient.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
UTL_APP(BT);
UTL_MAIN_RL(BT);

////////////////////////////////////////////////////////////////////////////////////////////////////

void
usage()
{
    utl::cerr << "usage: bt [-h <cse_host>] [-p <cse_port>] <cmd>\n"
                 "           <cmd> := exit\n"
                 "                 |  run <in_path> [out_path]\n"
                 "                 |  strip <in_path> <out_path> <regex>+\n";
    ;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool
connect(TCPsocket& server, InetHostname host, uint_t port)
{
    utl::cerr << "connect to CSE server: " << host << ":" << Uint(port) << " => " << flush;
    try
    {
        InetHostAddress hostAddr = host.address();
        server.open(hostAddr, port);
        String res;
        server >> res;
        res = res.subString(0, 3);
        if (res == "000")
        {
            utl::cerr << "OK" << endl;

            // get the challenge and response
            Uint challenge;
            challenge.serializeIn(server);
            utl::uint32_t response = cse::SEclient::response(challenge);

            // send authorizeClient command
            Array cmd;
            cmd += new String("authorizeClient");
            cmd += new Uint(response);
            cmd.serializeOut(server);

            // get response
            String s;
            server >> s;
        }
        else
        {
            utl::cerr << "server busy" << endl;
            return false;
        }
    }
    catch (NetworkEx&)
    {
        utl::cerr << "failed" << endl;
        return false;
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int
BT::run(int argc, char** argv)
{
    CmdLineArgs args(argc, argv);

    setSerializeMode(ser_readable);

    // set cse host?
    InetHostname cseHost;
    args.isSet("h", cseHost);
    if (cseHost.empty())
    {
        cseHost = "localhost";
    }

    // set cse port?
    uint_t csePort = 2000;
    String csePortStr;
    if (!args.isSet("p", csePortStr))
    {
        csePortStr = hostOS->getEnv("CSE_PORT");
    }
    if (!csePortStr.empty())
    {
        csePort = Uint(csePortStr);
    }

    // errors?
    size_t argIdx = args.idx();
    size_t numArgs = (args.items() - argIdx);
    if (args.printErrors(utl::cerr) || (numArgs < 1))
    {
        usage();
        return 1;
    }

    // get command
    String cmd = args(argIdx++);
    cmd.toLower();

    // for server connection
    TCPsocket server;

    //// EXIT ///////////////////////////////////////////////////////////////////////////////////////

    if ((cmd == "exit") && (numArgs == 1))
    {
        if (!connect(server, cseHost, csePort))
            return 1;
        Array cmd;
        cmd += new String("exit");
        cmd.serializeOut(server);
        utl::cerr << "sent EXIT command" << endl;
        return 0;
    }

    //// STRIP //////////////////////////////////////////////////////////////////////////////////////

    if ((cmd == "strip") && (numArgs >= 4))
    {
        Array stripCommands;
        Pathname inPath = args(argIdx++);
        Pathname outPath = args(argIdx++);
        do
        {
            Regex* regex = new Regex(args(argIdx++));
            stripCommands += regex;
        } while (argIdx < args.items());

        BufferedFileStream inFile(inPath, io_rd);
        BufferedFileStream outFile(outPath, io_wr | fs_create | fs_trunc);

        // read commands, write non-filtered ones to output file
        for (;;)
        {
            // read command from inFile
            Array cmd;
            try
            {
                cmd.serializeIn(inFile);
            }
            catch (StreamEOFex&)
            {
                break;
            }

            String command = (const String&)cmd(0);
            bool filtered = false;
            forEachIt(Array, stripCommands, Regex, regex) if (command == regex)
            {
                filtered = true;
                break;
            }
            endForEach if (filtered)
            {
                continue;
            }
            cmd.serializeOut(outFile);
        }

        return 0;
    }

    //// RUN ////////////////////////////////////////////////////////////////////////////////////////

    if (!connect(server, cseHost, csePort))
        return 1;

    if (!((cmd == "run") && (numArgs >= 2) && (numArgs <= 3)))
    {
        usage();
        return 1;
    }

    // open input file
    Pathname path = args(argIdx++);
    utl::cerr << "open input file: \"" << path << "\" => " << flush;
    BufferedFileStream inFile;
    try
    {
        inFile.open(path, io_rd);
        utl::cerr << "OK" << endlf;
    }
    catch (HostOSEx&)
    {
        utl::cerr << "failed" << endlf;
        return 1;
    }

    // open output file (if any)
    BufferedFileStream outFile;
    if (numArgs >= 3)
    {
        path = args(argIdx++);
        utl::cerr << "open output file: \"" << path << "\" => " << flush;
        try
        {
            outFile.open(path, io_wr | fs_create | fs_trunc);
            utl::cerr << "OK" << endlf;
        }
        catch (HostOSEx&)
        {
            utl::cerr << "failed" << endlf;
            return 1;
        }
    }

    utl::cerr << "running..." << endlf;
    bool initOptRun = false;
    bool run = false;
    for (;;)
    {
        // read command from inFile
        Array cmd;
        try
        {
            cmd.serializeIn(inFile);
        }
        catch (StreamEOFex&)
        {
            break;
        }

        ASSERTD(cmd.size() >= 1);
        ASSERTD(cmd(0).isA(String));
        const String& cmdName = (const String&)cmd(0);

        if (cmdName == "initOptimizerRun")
            initOptRun = true;
        else if (cmdName == "initSimpleRun")
            initOptRun = false;
        run = (initOptRun && (run || (cmdName == "run")));

        // print the command name
        utl::cerr << cmdName << "()" << endlf;

        // send command to server
        cmd.serializeOut(server);

        // get result
        for (;;)
        {
            // read string
            String str;
            server >> str;

            // write string to output file
            if (outFile.ok())
            {
                outFile << str << endl;
            }

            // end of response?
            if (str == "___END_RESPONSE___")
            {
                break;
            }
        }

        // wait for run to complete
        if (run)
        {
            utl::cerr << "waiting..." << endlf;
            Array grsCmd;
            grsCmd += String("getRunStatus");

            Bool complete = false;
            while (!complete)
            {
                // sleep for five seconds
                hostOS->sleep(5);

                // send getRunStatus
                grsCmd.serializeOut(server);

                // get response
                complete.serializeIn(server);
                String str;
                while (str != "___END_RESPONSE___")
                {
                    server >> str;
                }
            }
            run = false;
        }
    }

    return 0;
}
