#include "libmps.h"
#include <libutl/Time.h>
#include <libutl/BufferedFDstream.h>
#include <clp/FailEx.h>
#include "MPSrun.h"
#include <stdlib.h>

//////////////////////////////////////////////////////////////////////////////

#ifdef DEBUG
#define DEBUG_UNIT
#endif

//////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;
CSE_NS_USE;
MRP_NS_USE;

//////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(mps::MPSrun, utl::Object);

//////////////////////////////////////////////////////////////////////////////

MPS_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////

void
MPSrun::initialize(
    MPSdataSet* dataSet)
{
    delete _dataSet;
    _dataSet = dataSet;
}

//////////////////////////////////////////////////////////////////////////////

bool
MPSrun::run()
{
    mpsitem_set_id_t items = _dataSet->mpsItems();
    for (mpsitem_set_id_t::iterator it = items.begin();
         it != items.end(); it++)
    {
        mpsitemperiod_set_st_t periods = (*it)->periods();
        mpsitemperiod_set_st_t::iterator pIt;
#ifdef DEBUG_UNIT
        utl::cout << "item: " << (*it)->id() << utl::endlf;
        for (pIt = periods.begin(); pIt != periods.end(); pIt++)
            utl::cout << (*pIt)->toString() << utl::endlf;
        utl::cout << utl::endl
                  << "Calculate _pab and _mpsQuantity (calculate1)..."
                  << utl::endlf;
#endif
        // first run to calcualte _pab and _mpsQuantity
        for (pIt = periods.begin(); pIt != periods.end(); pIt++)
        {
            MPSitemPeriod* period = *pIt;
            period->calculate1();
        }
#ifdef DEBUG_UNIT
        for (pIt = periods.begin(); pIt != periods.end(); pIt++)
            utl::cout << (*pIt)->toString() << utl::endlf;
        utl::cout << utl::endl
                  << "Calculate _atp (calculate2)..."
                  << utl::endlf;
#endif
        // the second run to calculate _atp
        for (pIt = periods.begin(); pIt != periods.end(); pIt++)
        {
            MPSitemPeriod* period = *pIt;
            period->calculate2();
        }
#ifdef DEBUG_UNIT
        for (pIt = periods.begin(); pIt != periods.end(); pIt++)
            utl::cout << (*pIt)->toString() << utl::endlf;
#endif
    }

    return true;
}

//////////////////////////////////////////////////////////////////////////////

bool
MPSrun::generateWorkOrders()
{
//     static uint_t id = 0;
    mpsitem_set_id_t items = _dataSet->mpsItems();
    for (mpsitem_set_id_t::const_iterator it = items.begin();
         it != items.end(); it++)
    {
        time_t releaseTime = -1;
        mpsitemperiod_set_st_t periods = (*it)->periods();
        for (mpsitemperiod_set_st_t::const_iterator pIt = periods.begin();
             pIt != periods.end(); pIt++)
        {
            MPSitemPeriod* period = *pIt;
            // only generate workorders for unplanned wos.
            if (period->status() != periodstatus_unplanned) continue;

            // use the first period's st as releaseTimes of all WOs
            uint_t onHand = period->onHand();
            if (onHand != uint_t_max)
            {
                releaseTime = period->startTime();
            }

            uint_t quantity = period->mpsQuantity();
            if (quantity == 0) continue;
            uint_t eoq = period->item()->eoq();
            if (quantity < eoq)
            {
                utl::cout << "WARNING: \"" << period->toString()
                          << "\" has a MPSquantity that is less than eoq("
                          << Uint(eoq).toString()
                          << ")" << utl::endlf;
            }

            // create workorder(s)
            Item* item = _dataSet->findMRPitem(period->item()->id());
            if (item == nullptr)
            {
                String* str = new String();
                *str = "MPSitem-" + Uint(period->item()->id()).toString()
                    + ": doesn't exist as an item in MRP.";
                throw clp::FailEx(str);
            }
            if (dynamic_cast<ManufactureItem*>(item) == nullptr)
            {
                String* str = new String();
                *str = "MPSitem-" + Uint(period->item()->id()).toString()
                    + ": has to be a ManufactureItem in MRP.";
                throw clp::FailEx(str);
            }
            if (eoq <= 0)
            {
                String* str = new String();
                *str = "EOQ of item " + Uint(period->item()->id()).toString()
                    + "cannot be zero.";
                    throw clp::FailEx(str);
            }
            while (quantity > 0)
            {
                uint_t quan;
                if ((quantity >=  2 *  eoq) && (period->item()->allowMPSquantitySplit()))
                {
                    quan = eoq;
                }
                else
                {
                    quan = quantity;
                }
                _initId++;
                while (_dataSet->woIds().find(_initId) != _dataSet->woIds().end())
                {
                    _initId++;
                }
                WorkOrder* wo = generateWorkOrder(
                    _initId,
                    period,
                    (ManufactureItem*)item,
                    quan,
                    releaseTime);
                _dataSet->MRPdataSet::add(wo);
                quantity = quantity - quan;

#ifdef DEBUG_UNIT
                utl::cout << "Generated " << wo->toString() << utl::endlf;
#endif
            }
        }
    }
    return true;
}

//////////////////////////////////////////////////////////////////////////////

WorkOrder*
MPSrun::generateWorkOrder(
    uint_t id,
    MPSitemPeriod* period,
    ManufactureItem* item,
    uint_t quantity,
    time_t releaseTime)
{
    WorkOrder* wo = new WorkOrder();
    wo->id() = id;
    wo->setItem(item);
    wo->mpsItemPeriodIds().insert(period->id());
    wo->quantity() = quantity;
    wo->releaseTime() = releaseTime;
    wo->dueTime() = period->endTime();
    wo->status() = wostatus_unplanned;
    std::string name = item->name() + "_"
        + (char*)(Uint(quantity).toString()) + "_"
        + (char*)(Time(wo->dueTime()).toString("$yyyy/$m/$d"));
    wo->name() = name;
    // temporary change. Joe, July 10, 2007
    wo->opportunityCost() = period->opportunityCost() * quantity;
//     wo->opportunityCost() = period->opportunityCost();
    wo->opportunityCostPeriod() = period->opportunityCostPeriod();
    // temporary change. Joe, July 10, 2007
    wo->latenessCost() = period->latenessCost() * quantity;
//     wo->latenessCost() = period->latenessCost();
    wo->latenessCostPeriod() = period->latenessCostPeriod();
    wo->inventoryCost() = period->item()->inventoryCost() * quantity;
    wo->inventoryCostPeriod() = period->item()->inventoryCostPeriod();
    return wo;
}

//////////////////////////////////////////////////////////////////////////////

void
MPSrun::init()
{
    _dataSet = nullptr;
    _initId = 0;
}

//////////////////////////////////////////////////////////////////////////////

void
MPSrun::deInit()
{
    delete _dataSet;
}

//////////////////////////////////////////////////////////////////////////////

MPS_NS_END;
