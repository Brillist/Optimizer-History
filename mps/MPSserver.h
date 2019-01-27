#ifndef MPS_MPSSERVER_H
#define MPS_MPSSERVER_H

/////////////////////////////////////////////////////////////////////////////////////////////////////

#include <mrp/MRPserver.h>

/////////////////////////////////////////////////////////////////////////////////////////////////////

MPS_NS_BEGIN;

/////////////////////////////////////////////////////////////////////////////////////////////////////

class MPSclient;

/////////////////////////////////////////////////////////////////////////////////////////////////////

/**
    Provide command interface to MPS client.

    \author Joe Zhou
*/

/////////////////////////////////////////////////////////////////////////////////////////////////////

class MPSserver : public mrp::MRPserver
{
    UTL_CLASS_DECL(MPSserver, mrp::MRPserver);
    UTL_CLASS_NO_COPY;
    UTL_CLASS_NO_SERIALIZE;

public:
    /** Constructor. */
    MPSserver(uint_t maxClients, bool recording = false)
        : mrp::MRPserver(maxClients, recording)
    {
        init();
    }

protected:
    typedef void (MPSserver::*MPShfn)(MPSclient* client, const utl::Array& cmd);

protected:
    virtual utl::NetServerClient* clientMake(utl::FDstream* socket,
                                             const utl::InetHostAddress& addr);

private:
    void addHandler(const char* cmd, MPShfn handler);
    void handle_initMPS(MPSclient* client, const utl::Array& cmd);
    void handle_mpsRun(MPSclient* client, const utl::Array& cmd);
    void handle_getMPS(MPSclient* client, const utl::Array& cmd);
    void handle_generateWorkOrders(MPSclient* client, const utl::Array& cmd);
    void handle_getWorkOrders(MPSclient* client, const utl::Array& cmd);

private:
    void init();
    void
    deInit()
    {
    }
};

/////////////////////////////////////////////////////////////////////////////////////////////////////

MPS_NS_END;

/////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
