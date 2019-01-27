#include "libmps.h"
#include <libutl/BufferedFDstream.h>
#include <libutl/MemStream.h>
#include <libutl/Time.h>
#include <libutl/Bool.h>
#include "MPSitem.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef DEBUG
// #define DEBUG_UNIT
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;
CSE_NS_USE;
MRP_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(mps::MPSitem);

////////////////////////////////////////////////////////////////////////////////////////////////////

MPS_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

void
MPSitem::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(MPSitem));
    const MPSitem& mi = (const MPSitem&)rhs;
    Item::copy(mi);
    _eoq = mi._eoq;
    _dtf = mi._dtf;
    _ptf = mi._ptf;
    //     _planningHorizon = mi._planningHorizon;
    _inventoryCost = mi._inventoryCost;
    _inventoryCostPeriod = mi._inventoryCostPeriod;
    _allowMPSquantitySplit = mi._allowMPSquantitySplit;
    copySet(_periods, mi._periods);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
MPSitem::serialize(Stream& stream, uint_t io, uint_t)
{
    Item::serialize(stream, io);
    utl::serialize(_eoq, stream, io);
    utl::serialize(_dtf, stream, io);
    utl::serialize(_ptf, stream, io);
    //     utl::serialize(_planningHorizon, stream, io);
    utl::serialize(_inventoryCost, stream, io);
    utl::serialize((uint_t&)_inventoryCostPeriod, stream, io);
    utl::serialize(_allowMPSquantitySplit, stream, io);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
MPSitem::linkPeriods()
{
    MPSitemPeriod* prev = nullptr;
    for (mpsitemperiod_set_st_t::iterator it = _periods.begin(); it != _periods.end(); it++)
    {
        MPSitemPeriod* period = *it;
        if (prev)
        {
            period->prev() = prev;
            prev->next() = period;
        }
        prev = period;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

String
MPSitem::toString() const
{
    MemStream str;
    str << Item::toString() << '\n'
        << "   "
        << "itemType:MPS"
        << ", EOQ:" << _eoq << ", DTF:" << Time(_dtf).toString() << ", PTF:"
        << Time(_ptf).toString()
        //         << ", plngHrzn:" << Time(_planningHorizon).toString()
        << ", ivntCost:" << Float(_inventoryCost).toString(2) << ", period:" << _inventoryCostPeriod
        << ", allowMPSqntySplit:" << Bool(_allowMPSquantitySplit).toString();
    mpsitemperiod_set_id_t::const_iterator it;
    for (it = _periods.begin(); it != _periods.end(); it++)
    {
        MPSitemPeriod* period = (*it);
        str << '\n' << "      " << period->toString();
    }
    str << '\0';
    return String((char*)str.get());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
MPSitem::init()
{
    _eoq = uint_t_max;
    _dtf = -1;
    _ptf = -1;
    //     _planningHorizon = -1;
    _inventoryCost = 0.0;
    _inventoryCostPeriod = period_undefined;
    _allowMPSquantitySplit = true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
MPSitem::deInit()
{
    deleteCont(_periods);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

MPS_NS_END;
