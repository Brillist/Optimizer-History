#include "libcse.h"
#include "ResourceSequenceList.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(cse::ResourceSequenceRule, utl::Object);
UTL_CLASS_IMPL(cse::ResourceSequenceList, utl::Object);

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////
// ResourceSequenceRule ////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void
ResourceSequenceRule::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(ResourceSequenceRule));
    const ResourceSequenceRule& rsr = (const ResourceSequenceRule&)rhs;
    _lhsOpSequenceId = rsr._lhsOpSequenceId;
    _rhsOpSequenceId = rsr._rhsOpSequenceId;
    _delay = rsr._delay;
    _cost = rsr._cost;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int
ResourceSequenceRule::compare(const Object& rhs) const
{
    if (!rhs.isA(ResourceSequenceRule))
        return Object::compare(rhs);

    const ResourceSequenceRule& rsr = (const ResourceSequenceRule&)rhs;
    int res;
    res = lut::compare(_lhsOpSequenceId, rsr._lhsOpSequenceId);
    if (res != 0)
        return res;
    res = lut::compare(_rhsOpSequenceId, rsr._rhsOpSequenceId);
    if (res != 0)
        return res;
    res = lut::compare(_delay, rsr._delay);
    if (res != 0)
        return res;
    res = lut::compare(_cost, rsr._cost);
    return res;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ResourceSequenceRule::serialize(Stream& stream, uint_t io, uint_t)
{
    utl::serialize(_lhsOpSequenceId, stream, io);
    utl::serialize(_rhsOpSequenceId, stream, io);
    utl::serialize(_delay, stream, io);
    utl::serialize(_cost, stream, io);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ResourceSequenceRule::init()
{
    _lhsOpSequenceId = uint_t_max;
    _rhsOpSequenceId = uint_t_max;
    _delay = 0;
    _cost = 0.0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// ResourceSequenceList ////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void
ResourceSequenceList::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(ResourceSequenceList));
    const ResourceSequenceList& rsl = (const ResourceSequenceList&)rhs;
    _id = rsl._id;
    copySet(_rsl, rsl._rsl);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int
ResourceSequenceList::compare(const Object& rhs) const
{
    if (!rhs.isA(ResourceSequenceList))
        return Object::compare(rhs);

    const ResourceSequenceList& rsl = (const ResourceSequenceList&)rhs;
    int res;
    res = lut::compare(_id, rsl._id);
    if (res != 0)
        return res;
    res = compareContObjects(_rsl, rsl._rsl);
    return res;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool
ResourceSequenceList::equals(const ResourceSequenceList& rhs) const
{
    return (compare(rhs) == 0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ResourceSequenceList::serialize(Stream& stream, uint_t io, uint_t)
{
    utl::serialize(_id, stream, io);
    lut::serialize<Object*>(_rsl, stream, io);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

const ResourceSequenceRule*
ResourceSequenceList::findRule(uint_t lhsosid, uint_t rhsosid) const
{
    rsr_set_t::const_iterator it;
    for (it = _rsl.begin(); it != _rsl.end(); ++it)
    {
        const ResourceSequenceRule* rsr = *it;
        uint_t rsrLhsosid = rsr->lhsOpSequenceId();
        uint_t rsrRhsosid = rsr->rhsOpSequenceId();

        // match lhs-op-sequence-id?
        if ((lhsosid != uint_t_max) && (rsrLhsosid != uint_t_max) && (lhsosid != rsrLhsosid))
        {
            continue;
        }

        // match rhs-op-sequence-id?
        if ((rhsosid != uint_t_max) && (rsrRhsosid != uint_t_max) && (rhsosid != rsrRhsosid))
        {
            continue;
        }

        // found matching record
        return rsr;
    }
    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ResourceSequenceList::add(uint_t lhsOpSequenceId, uint_t rhsOpSequenceId, uint_t delay, double cost)
{
    ResourceSequenceRule* rsr =
        new ResourceSequenceRule(lhsOpSequenceId, rhsOpSequenceId, delay, cost);
    _rsl.insert(rsr);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ResourceSequenceList::init()
{
    _id = uint_t_max;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ResourceSequenceList::deInit()
{
    deleteCont(_rsl);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;
