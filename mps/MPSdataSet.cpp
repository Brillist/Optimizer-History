#include "libmps.h"
#include <libutl/BufferedFDstream.h>
#include <clp/FailEx.h>
#include "MPSdataSet.h"

//////////////////////////////////////////////////////////////////////////////

#ifdef DEBUG
#define DEBUG_UNIT
#endif

//////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;
MRP_NS_USE;

//////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(mps::MPSdataSet, mrp::MRPdataSet);

//////////////////////////////////////////////////////////////////////////////

MPS_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////

void
MPSdataSet::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(MPSdataSet));
    const MPSdataSet& ds = (const MPSdataSet&)rhs;
    copySet(_mpsItems, ds._mpsItems);
    copySet(_mpsItemPeriods, ds._mpsItemPeriods);
}

//////////////////////////////////////////////////////////////////////////////

void
MPSdataSet::add(MPSitem* item)
{
    _mpsItems.insert(item);
}

//////////////////////////////////////////////////////////////////////////////

void
MPSdataSet::add(MPSitemPeriod* period)
{
    MPSitem* item = lut::setFind(mpsItems(), (MPSitem*)period->item());
    if (item == nullptr)
    {
        String* str = new String();
        *str = "MPSitemPeriod contains an unknown item";
        *str += Uint(period->item()->id()).toString();
        throw clp::FailEx(str);
    }

    _mpsItemPeriods.insert(period);
    item->periods().insert(period);
    period->setItem(item);
}

//////////////////////////////////////////////////////////////////////////////

void
MPSdataSet::initialize(
    Array& mpsItems,
    Array& mpsItemPeriods)
{
    // clear out existing data if there is any.
    deInit();

    // so that we don't have to make copies.
    mpsItems.setOwner(false);

    forEachIt(Array, mpsItems, MPSitem, item)
        add(item);
    endForEach

    resetMPSitemPeriods(mpsItemPeriods);

#ifdef DEBUG_UNIT
    checkData();
#endif
}

//////////////////////////////////////////////////////////////////////////////

void
MPSdataSet::resetMPSitemPeriods(Array& mpsItemPeriods)
{
    mpsitem_set_id_t::const_iterator it;
    for (it = _mpsItems.begin(); it != _mpsItems.end(); it++)
    {
        MPSitem* item = (*it);
        deleteCont(item->periods());
    }
    _mpsItemPeriods.clear();

    // so that we don't have to make copies.
    mpsItemPeriods.setOwner(false);
    forEachIt(Array, mpsItemPeriods, MPSitemPeriod, period)
        add(period);
    endForEach

        for (mpsitem_set_id_t::const_iterator it = _mpsItems.begin();
             it != _mpsItems.end(); it++)
        {
            MPSitem* item = *it;
            item->linkPeriods();
        }
}

//////////////////////////////////////////////////////////////////////////////

void
MPSdataSet::resetWorkOrderIds(Array& workOrderIds)
{
    _woIds.clear();

    // so that we don't have to make copies.
    workOrderIds.setOwner(false);
    forEachIt(Array, workOrderIds, Uint, woId)
        _woIds.insert((uint_t)woId);
    endForEach
}

//////////////////////////////////////////////////////////////////////////////

void
MPSdataSet::checkData() const
{
    utl::cout << "MPSdataSet::checkData MPSitems:" << utl::endlf;
    for (mpsitem_set_id_t::const_iterator it = mpsItems().begin();
         it != mpsItems().end(); it++)
        utl::cout << (*it)->toString() << utl::endlf;
}

//////////////////////////////////////////////////////////////////////////////

void
MPSdataSet::init()
{
}

//////////////////////////////////////////////////////////////////////////////

void
MPSdataSet::deInit()
{
    deleteCont(_mpsItems);
    // all mpsItemPeriods are owned by mpsItems,
    // so there is no need for deleteCont(_mpsItemPeriods)
    _mpsItemPeriods.clear();
//     deleteCont(_mpsItemPeriods);
    _woIds.clear();
}

//////////////////////////////////////////////////////////////////////////////

MPS_NS_END;
