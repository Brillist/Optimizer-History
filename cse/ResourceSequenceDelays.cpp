#include "libcse.h"
#include "ResourceSequenceDelays.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(cse::ResourceSequenceDelay);
UTL_CLASS_IMPL(cse::ResourceSequenceDelays);

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////
// ResourceSequenceDelay ///////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void
ResourceSequenceDelay::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(ResourceSequenceDelay));
    const ResourceSequenceDelay& rsd = (const ResourceSequenceDelay&)rhs;
    _lhsOpSequenceId = rsd._lhsOpSequenceId;
    _rhsOpSequenceId = rsd._rhsOpSequenceId;
    _delay = rsd._delay;
    _cost = rsd._cost;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int
ResourceSequenceDelay::compare(const Object& rhs) const
{
    ASSERTD(rhs.isA(ResourceSequenceDelay));
    const ResourceSequenceDelay& rsd = (const ResourceSequenceDelay&)rhs;
    int res;
    res = lut::compare(_lhsOpSequenceId, rsd._lhsOpSequenceId);
    if (res != 0)
        return res;
    res = lut::compare(_rhsOpSequenceId, rsd._rhsOpSequenceId);
    if (res != 0)
        return res;
    res = lut::compare(_delay, rsd._delay);
    if (res != 0)
        return res;
    res = lut::compare(_cost, rsd._cost);
    return res;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ResourceSequenceDelay::init()
{
    _lhsOpSequenceId = uint_t_max;
    _rhsOpSequenceId = uint_t_max;
    _delay = 0;
    _cost = 0.0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// ResourceSequenceDelays //////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void
ResourceSequenceDelays::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(ResourceSequenceDelays));
    const ResourceSequenceDelays& rsd = (const ResourceSequenceDelays&)rhs;
    _resourceSequenceId = rsd._resourceSequenceId;
    copySet(_resourceSequenceDelays, rsd._resourceSequenceDelays);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int
ResourceSequenceDelays::compare(const Object& rhs) const
{
    ASSERTD(rhs.isA(ResourceSequenceDelays));
    const ResourceSequenceDelays& rsd = (const ResourceSequenceDelays&)rhs;
    int res;
    res = lut::compare(_resourceSequenceId, rsd._resourceSequenceId);
    if (res != 0)
        return res;
    res = compareContObjects(_resourceSequenceDelays, rsd._resourceSequenceDelays);
    return res;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ResourceSequenceDelays::add(uint_t lhsOpSequenceId, uint_t rhsOpSequenceId, int delay, double cost)
{
    ResourceSequenceDelay* rsd =
        new ResourceSequenceDelay(lhsOpSequenceId, rhsOpSequenceId, delay, cost);
    _resourceSequenceDelays.insert(rsd);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ResourceSequenceDelays::init()
{
    _resourceSequenceId = uint_t_max;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ResourceSequenceDelays::deInit()
{
    deleteCont(_resourceSequenceDelays);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;
