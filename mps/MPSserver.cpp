#include "libmps.h"
#include <libutl/Bool.h>
#include <clp/FailEx.h>
#include <gop/ConfigEx.h>
#include <mrp/MRPserver.h>
#include "MPSdataSet.h"
#include "MPSserver.h"
#include "MPSclient.h"
#include "MPSitemPeriod.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef DEBUG
#define DEBUG_UNIT
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;
GOP_NS_USE;
CSE_NS_USE;
CLP_NS_USE;
MRP_NS_USE;

/////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(mps::MPSserver);

/////////////////////////////////////////////////////////////////////////////////////////////////////

MPS_NS_BEGIN;

/////////////////////////////////////////////////////////////////////////////////////////////////////

NetServerClient*
MPSserver::clientMake(FDstream* socket, const InetHostAddress& addr)
{
    MPSserver* const server = this;
    return new MPSclient(server, socket, addr, _recording);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

void
MPSserver::addHandler(const char* cmd, MPShfn handler)
{
    Server::addHandler(cmd, reinterpret_cast<hfn>(handler));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

void
MPSserver::handle_initMPS(MPSclient* client, const Array& cmd)
{
#ifdef DEBUG_UNIT
    utl::cout << utl::endl
              << "MPSserver::handle_initMPS."
              << "cmd.size():" << cmd.size() << utl::endlf;
#endif

    if ((cmd.size() != 3) || !cmd(1).isA(Array) || !allAre(cmd(1), CLASS(Item))

        || !cmd(2).isA(Array) || !allAre(cmd(2), CLASS(MPSitemPeriod)))
    {
        Bool(false).serializeOut(client->socket());
        String str = "initMPS error: wrong cmd.";
        str.serializeOut(client->socket());
        finishCmd(client);
        clientDisconnect(client); //?
        return;
    }
    Array& mpsItems = (Array&)cmd(1);
    Array& mpsItemPeriods = (Array&)cmd(2);

    bool result = true;
    String errStr;
    MPSdataSet* dataSet = new MPSdataSet();
    try
    {
        dataSet->initialize(mpsItems, mpsItemPeriods);
        client->mpsRun()->initialize(dataSet);
        //set MRPrun's dataSet point
        client->mrpRun()->dataSet() = dataSet;
        client->mrpRun()->dataSetOwner() = false;
    }
    catch (...)
    {
        result = false;
        errStr = "initMPS error: MPSdataSet initialization failed.";
    }

    Bool(result).serializeOut(client->socket());
    if (!result)
    {
        errStr.serializeOut(client->socket());
        finishCmd(client);
        clientDisconnect(client);
    }
    else
    {
        finishCmd(client);
    }
    return;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void
MPSserver::handle_mpsRun(MPSclient* client, const Array& cmd)
{
#ifdef DEBUG_UNIT
    utl::cout << utl::endl
              << "MPSserver::handle_mpsRun."
              << "cmd.size():" << cmd.size() << utl::endlf;
#endif

    if (cmd.size() != 1)
    {
        Bool(false).serializeOut(client->socket());
        String str = "initMPS error: wrong cmd.";
        str.serializeOut(client->socket());
        finishCmd(client);
        clientDisconnect(client); //?
        return;
    }

    bool result = false;
    String errStr = "initMPS erro: ";
    try
    {
        client->mpsRun()->run();
        result = true;
    }
    catch (clp::FailEx& ex)
    {
        errStr = *ex.str();
    }
    catch (...)
    {
        errStr = "MPSRun->run() failed.";
    }
    Bool(result).serializeOut(client->socket());
    if (!result)
    {
        errStr.serializeOut(client->socket());
        finishCmd(client);
        clientDisconnect(client);
    }
    else
    {
        finishCmd(client);
    }
    return;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

void
MPSserver::handle_getMPS(MPSclient* client, const Array& cmd)
{
#ifdef DEBUG_UNIT
    utl::cout << utl::endl
              << "MPSserver::handle_getMPS."
              << "cmd.size():" << cmd.size() << utl::endlf;
#endif

    if (cmd.size() != 1)
    {
        Bool(false).serializeOut(client->socket());
        String str = "getMPS error: wrong cmd.";
        str.serializeOut(client->socket());
        finishCmd(client);
        clientDisconnect(client); //?
        return;
    }

    String errStr;
    MPSdataSet* dataSet = client->mpsRun()->dataSet();
    ASSERTD(dataSet != nullptr);
    Bool(true).serializeOut(client->socket());

#ifdef DEBUG_UNIT
    mpsitemperiod_set_id_t::const_iterator it;
    for (it = dataSet->periods().begin(); it != dataSet->periods().end(); it++)
    {
        MPSitemPeriod* period = (*it);
        utl::cout << period->toString() << utl::endlf;
    }
#endif

    lut::serialize<MPSitemPeriod*>(dataSet->periods(), client->socket(), io_wr);
    finishCmd(client);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

void
MPSserver::handle_generateWorkOrders(MPSclient* client, const Array& cmd)
{
#ifdef DEBUG_UNIT
    utl::cout << utl::endl
              << "MPSserver::handle_generateWorkOrders."
              << "cmd.size():" << cmd.size() << utl::endlf;
#endif

    MPSdataSet* dataSet = client->mpsRun()->dataSet();
    if ((cmd.size() != 4) || !cmd(1).isA(Array) || !allAre(cmd(1), CLASS(MPSitemPeriod))

        || !cmd(2).isA(Array) || !allAre(cmd(2), CLASS(Item))

        || !cmd(3).isA(Array) || !allAre(cmd(3), CLASS(Uint)))
    {
        Bool(false).serializeOut(client->socket());
        String str = "generateWorkOrders error: wrong cmd.";
        str.serializeOut(client->socket());
        finishCmd(client);
        clientDisconnect(client);
        return;
    }

    Array& mpsItemPeriods = (Array&)cmd(1);
    Array& items = (Array&)cmd(2);
    Array& woIds = (Array&)cmd(3);
    bool result = false;
    String errStr = "generateWorkOrders error: ";
    try
    {
        dataSet->resetMPSitemPeriods(mpsItemPeriods);
        dataSet->resetItems(items);
        dataSet->resetWorkOrderIds(woIds);

#ifdef DEBUG_UNIT
        utl::cout << "   Input MPSItemPeriods: size:" << Uint(dataSet->periods().size())
                  << utl::endlf;
        mpsitemperiod_set_itemid_t::iterator periodIt;
        for (periodIt = dataSet->periods().begin(); periodIt != dataSet->periods().end();
             periodIt++)
        {
            MPSitemPeriod* period = (*periodIt);
            utl::cout << "   " << period->toString() << utl::endlf;
        }

        utl::cout << "   Input existing woIds: size" << Uint(dataSet->woIds().size()) << ", Ids:";
        uint_set_t::iterator idIt;
        for (idIt = dataSet->woIds().begin(); idIt != dataSet->woIds().end(); idIt++)
        {
            utl::cout << (*idIt) << ", ";
        }
        utl::cout << utl::endlf;
#endif

        client->mpsRun()->generateWorkOrders();
        result = true;
    }
    catch (clp::FailEx& ex)
    {
        errStr += *ex.str();
    }
    catch (...)
    {
        errStr += "unknown error.";
    }

    Bool(result).serializeOut(client->socket());
    if (!result)
    {
        errStr.serializeOut(client->socket());
        finishCmd(client);
        clientDisconnect(client);
    }
    else
    {
        finishCmd(client);
    }
    return;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

void
MPSserver::handle_getWorkOrders(MPSclient* client, const Array& cmd)
{
#ifdef DEBUG_UNIT
    utl::cout << utl::endl
              << "MPSserver::handle_getWorkOrders."
              << "cmd.size():" << cmd.size() << utl::endlf;
#endif

    if (cmd.size() != 1)
    {
        Bool(false).serializeOut(client->socket());
        String str = "getWorkOrders error: wrong cmd.";
        str.serializeOut(client->socket());
        finishCmd(client);
        clientDisconnect(client);
        return;
    }

    MRPdataSet* dataSet = client->mrpRun()->dataSet();
    ASSERTD(dataSet != nullptr);

#ifdef DEBUG_UNIT
    utl::cout << "   Output wos: size:" << Uint(dataSet->workOrders().size()) << utl::endlf;
    for (workorder_set_id_t::iterator it = dataSet->workOrders().begin();
         it != dataSet->workOrders().end(); it++)
        utl::cout << (*it)->toString() << utl::endlf;
#endif

    Bool(true).serializeOut(client->socket());
    lut::serialize<WorkOrder*>(dataSet->workOrders(), client->socket(), io_wr);

    finishCmd(client);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

void
MPSserver::init()
{
    addHandler("initMPS", &MPSserver::handle_initMPS);
    addHandler("mpsRun", &MPSserver::handle_mpsRun);
    addHandler("getMPS", &MPSserver::handle_getMPS);
    addHandler("generateWorkOrders", &MPSserver::handle_generateWorkOrders);
    addHandler("getWorkOrders", &MPSserver::handle_getWorkOrders);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

MPS_NS_END;
