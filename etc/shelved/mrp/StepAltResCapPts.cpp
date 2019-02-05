#include "libmrp.h"
#include <libutl/Uint.h>
#include <libutl/MemStream.h>
#include "StepAltResCapPts.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(mrp::StepAltResCapPts);

////////////////////////////////////////////////////////////////////////////////////////////////////

MRP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

void
StepAltResCapPts::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(StepAltResCapPts));
    const StepAltResCapPts& rcps = (const StepAltResCapPts&)rhs;
    _resourceId = rcps._resourceId;
    _caps = rcps._caps;
    _pts = rcps._pts;
    _ptPers = rcps._ptPers;
    _ptBatchSizes = rcps._ptBatchSizes;
    _setupIds = rcps._setupIds;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int
StepAltResCapPts::compare(const utl::Object& rhs) const
{
    if (!rhs.isA(StepAltResCapPts))
    {
        return Object::compare(rhs);
    }
    const StepAltResCapPts& rcps = (const StepAltResCapPts&)rhs;
    int res = utl::compare(_resourceId, rcps._resourceId);
    return res;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
StepAltResCapPts::serialize(Stream& stream, uint_t io, uint_t)
{
    utl::serialize(_resourceId, stream, io);
    lut::serialize<uint_t>(_caps, stream, io);
    lut::serialize<uint_t>(_pts, stream, io);
    lut::serialize<uint_t>(_ptPers, stream, io);
    lut::serialize<uint_t>(_ptBatchSizes, stream, io);
    lut::serialize<uint_t>(_setupIds, stream, io);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool
StepAltResCapPts::hasCap(uint_t cap)
{
    uint_t numCapPts = _caps.size();
    for (uint_t i = 0; i < numCapPts; i++)
    {
        if (cap == _caps[i])
        {
            return true;
        }
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

uint_t
StepAltResCapPts::minCap()
{
    uint_t minCap = uint_t_max;
    uint_t numCapPts = _caps.size();
    for (uint_t i = 0; i < numCapPts; i++)
    {
        if (_caps[i] < minCap)
            minCap = _caps[i];
    }
    return minCap;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

uint_t
StepAltResCapPts::maxCap()
{
    uint_t maxCap = 0;
    uint_t numCapPts = _caps.size();
    for (uint_t i = 0; i < numCapPts; i++)
    {
        if (_caps[i] > maxCap)
            maxCap = _caps[i];
    }
    return maxCap;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
StepAltResCapPts::addCapPt(
    uint_t cap, uint_t pt, rescappt_pt_per_t ptPer, uint_t ptBatchSize, uint_t setupId)
{
    _caps.push_back(cap);
    _pts.push_back(pt);
    _ptPers.push_back(ptPer);
    _ptBatchSizes.push_back(ptBatchSize);
    _setupIds.push_back(setupId);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

cls::ResourceCapPts*
StepAltResCapPts::createResourceCapPts(uint_t quantity) const
{
    cls::ResourceCapPts* resCapPts = new cls::ResourceCapPts();
    resCapPts->resourceId() = _resourceId;
    resCapPts->clear();
    uint_t numCapPts = _caps.size();
    for (uint_t i = 0; i < numCapPts; i++)
    {
        uint_t pt = _pts[i];
        rescappt_pt_per_t per = (rescappt_pt_per_t)_ptPers[i];
        uint_t ptBatchSize = _ptBatchSizes[i];
        if (per == cappt_pt_per_batch)
        {
            if (ptBatchSize != uint_t_max)
            {
                pt = pt * (uint_t)ceil((double)quantity / (double)ptBatchSize);
            }
        }
        else // cappt_pt_per_piece or cappt_pt_per_undefined
        {
            pt = pt * quantity;
        }
        resCapPts->addCapPt(_caps[i], pt);
    }
    return resCapPts;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

String
StepAltResCapPts::toString() const
{
    MemStream str;
    str << "stepAltResCapPts:"
        << ", resource:" << _resourceId << ", capPts(cap,pt,setup):{";
    ASSERTD(_caps.size() == _pts.size());
    ASSERTD(_caps.size() == _setupIds.size());
    int size = _caps.size();
    for (int i = 0; i < size; i++)
    {
        if (i != 0)
            str << ", ";
        str << "(" << _caps[i] << "," << _pts[i];
        if (_ptPers[i] == cappt_pt_per_batch)
        {
            if (_ptBatchSizes[i] != uint_t_max)
            {
                str << "/batch(" << _ptBatchSizes[i] << ")";
            }
            else
            {
                str << "/wo";
            }
        }
        else
        {
            str << "/piece";
        }
        str << "," << _setupIds[i] << ")";
    }
    str << "]" << '\0';
    return String((char*)str.get());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

MRP_NS_END;
