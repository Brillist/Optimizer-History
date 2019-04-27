#pragma once

/////////////////////////////////////////////////////////////////////////////////////////////////////

#include <libutl/Application.h>

/////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Server application.

   ServerApp specializes utl::Application to serve as the entry point to the CSE application.

   The following command-line arguments are accepted:

   - **-h** : print usage and exit
   - **-d** : execute as a daemon
   - **-p &lt;port>** : listen for client connections on `port` (default is 2000)
   - **-r** : record client commands to `/work/cse-commands-<client-ip>`

   After reading command-line arguments, ServerApp runs an instance of Server to receive
   client connections and respond to commands issued by clients.

   \see Server
   \ingroup cse
*/

/////////////////////////////////////////////////////////////////////////////////////////////////////

class ServerApp : public utl::Application
{
    UTL_CLASS_DECL(ServerApp, utl::Application);
    UTL_CLASS_DEFID;

public:
    virtual int run(int argc = 0, char** argv = nullptr);

private:
    void usage();
};

/////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;
