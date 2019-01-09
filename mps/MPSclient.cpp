#include "libmps.h"
#include "MPSclient.h"

///////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
CSE_NS_USE;

///////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(mps::MPSclient, mrp::MRPclient);

///////////////////////////////////////////////////////////////////////////////

MPS_NS_BEGIN;

///////////////////////////////////////////////////////////////////////////////

MPSclient::MPSclient(
    MPSserver* server,
    utl::FDstream* socket,
    const utl::InetHostAddress& addr,
    bool recording)
    : MRPclient((mrp::MRPserver*)server, socket, addr, recording)
{
    init();
}

///////////////////////////////////////////////////////////////////////////////

void
MPSclient::init()
{
    _mpsRun = new MPSrun();
}

///////////////////////////////////////////////////////////////////////////////

void
MPSclient::deInit()
{
    delete _mpsRun;
}

///////////////////////////////////////////////////////////////////////////////

MPS_NS_END;
