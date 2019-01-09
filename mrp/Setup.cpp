#include "libmrp.h"
#include <libutl/MemStream.h>
#include <libutl/Int.h>
#include <libutl/Float.h>
#include <mrp/ProcessPlan.h>
#include <mrp/ProcessStep.h>
#include "Setup.h"

//////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;

//////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(mrp::Setup, utl::Object);

//////////////////////////////////////////////////////////////////////////////

MRP_NS_BEGIN;

/////////////////////////////////////////////////////////////////////////////

bool
SetupOrderingIncId::operator()(Setup* lhs, Setup* rhs) const
{
    return (lhs->id() < rhs->id());
}

/////////////////////////////////////////////////////////////////////////////

void
Setup::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(Setup));
    const Setup& s = (const Setup&)rhs;
    _id = s._id;
    _prevSetupIdsMap = s._prevSetupIdsMap;
    _setupTimes = s._setupTimes;
    _setupCosts = s._setupCosts;
}

//////////////////////////////////////////////////////////////////////////////

void
Setup::serialize(Stream& stream, uint_t io, uint_t mode)
{
    utl::serialize(_id, stream, io);
    lut::serialize<uint_t, uint_t>(_prevSetupIdsMap, stream, io);
    lut::serialize<uint_t>(_setupTimes, stream, io);
    lut::serialize<double>(_setupCosts, stream, io);
}

//////////////////////////////////////////////////////////////////////////////

uint_t
Setup::getPrevSetupIdx(uint_t prevSetupId) const
{
    uint_uint_map_t::const_iterator it;
    it  = _prevSetupIdsMap.find(prevSetupId);
    if (it == _prevSetupIdsMap.end())
    {
        return uint_t_max;
    }
    else
    {
        return (*it).second;
    }
}

//////////////////////////////////////////////////////////////////////////////

String
Setup::toString() const
{
    MemStream str;
    str << "setup:" << _id
        << ", prevSetups(id,time,cost):[";
    uint_uint_map_t::const_iterator it;
    for (it = _prevSetupIdsMap.begin(); it != _prevSetupIdsMap.end();
         it++)
    {
        if (it != _prevSetupIdsMap.begin())
            str << ", ";
        str << "(" << (*it).first
            << "," << _setupTimes[(*it).second]
            << "," << Float(_setupCosts[(*it).second]).toString("precision:2");
    }
    str << "]" << '\0';
    return String((char*)str.get());
}

//////////////////////////////////////////////////////////////////////////////

MRP_NS_END;
