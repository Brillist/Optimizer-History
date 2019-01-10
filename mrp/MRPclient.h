#ifndef MRP_MRPCLIENT_H
#define MRP_MRPCLIENT_H

/////////////////////////////////////////////////////////////////////////////////////////////////////

#include <cse/SEclient.h>
#include <mrp/MRPrun.h>

/////////////////////////////////////////////////////////////////////////////////////////////////////

MRP_NS_BEGIN;

/////////////////////////////////////////////////////////////////////////////////////////////////////

class MRPserver;

/////////////////////////////////////////////////////////////////////////////////////////////////////

class MRPclient : public cse::SEclient
{
    UTL_CLASS_DECL(MRPclient);

public:
    MRPclient(MRPserver* server,
              utl::FDstream* socket,
              const utl::InetHostAddress& addr,
              bool recording = false);

    MRPrun*
    mrpRun()
    {
        return _mrpRun;
    }

private:
    void init();

    void deInit();

private:
    MRPrun* _mrpRun;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////

MRP_NS_END;

/////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
