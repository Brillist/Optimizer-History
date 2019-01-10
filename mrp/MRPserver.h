#ifndef MRP_MRPSERVER_H
#define MRP_MRPSERVER_H

/////////////////////////////////////////////////////////////////////////////////////////////////////

#include <cse/Server.h>

/////////////////////////////////////////////////////////////////////////////////////////////////////

MRP_NS_BEGIN;

/////////////////////////////////////////////////////////////////////////////////////////////////////

class MRPclient;

/////////////////////////////////////////////////////////////////////////////////////////////////////

/**
    Provide command interface to MRP client.

    \author Adam McKee
*/

/////////////////////////////////////////////////////////////////////////////////////////////////////

class MRPserver : public cse::Server
{
    UTL_CLASS_DECL(MRPserver);
    UTL_CLASS_NO_COPY;
    UTL_CLASS_NO_SERIALIZE;

public:
    /** Constructor. */
    MRPserver(utl::uint_t maxClients, bool recording = false)
        : cse::Server(maxClients, recording)
    {
        init();
    }

protected:
    typedef void (MRPserver::*MRPhfn)(MRPclient* client, const utl::Array& cmd);

protected:
    virtual utl::NetServerClient* clientMake(utl::FDstream* socket,
                                             const utl::InetHostAddress& addr);

private:
    void addHandler(const char* cmd, MRPhfn handler);
    void handle_initMRP(MRPclient* client, const utl::Array& cmd);
    void handle_generateJobs(MRPclient* client, const utl::Array& cmd);
    void handle_getJobs(MRPclient* client, const utl::Array& cmd);
    void handle_initScheduler(MRPclient* client, const utl::Array& cmd);
    void handle_initOptimizer(MRPclient* client, const utl::Array& cmd);

private:
    void init();
    void
    deInit()
    {
    }
};

/////////////////////////////////////////////////////////////////////////////////////////////////////

MRP_NS_END;

/////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
