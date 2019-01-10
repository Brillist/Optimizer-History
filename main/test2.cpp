#include "main.h"
#include <libutl/Application.h>
#include <libutl/BufferedFDstream.h>
#include <libutl/FileStream.h>
#include <libutl/TCPsocket.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
UTL_APP(Test);
UTL_MAIN_RL(Test);

////////////////////////////////////////////////////////////////////////////////////////////////////

void writeText(TCPsocket& socket, const char* filename);

////////////////////////////////////////////////////////////////////////////////////////////////////

int
Test::run(int, char**)
{
    setSerializeMode(ser_readable);

    // connect to server
    InetHostname localhost("stan.adam.vpn");
    InetHostAddress serverAddr(localhost);
    TCPsocket server(serverAddr, 2000);
    String str;
    server >> str;
    utl::cout << str << endlf;

    writeText(server, "initSimpleRun.txt");

    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
writeText(TCPsocket& socket, const char* filename)
{
    FileStream fs(filename, io_rd);
    socket.copyData(fs);
    socket.flush();
}
