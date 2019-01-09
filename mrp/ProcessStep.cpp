#include "libmrp.h"
#include <libutl/MemStream.h>
#include <cse/JobOp.h>
#include <mrp/StepItemRequirement.h>
#include <mrp/StepResourceRequirement.h>
#include <mrp/StepAltResCapPts.h>
#include "ProcessStep.h"

//////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;
CSE_NS_USE;

//////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(mrp::ProcessStep, utl::Object);

//////////////////////////////////////////////////////////////////////////////

MRP_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////

bool
StepOrderingIncId::operator()(
    const ProcessStep* lhs,
    const ProcessStep* rhs) const
{
    return (lhs->id() < rhs->id());
}

//////////////////////////////////////////////////////////////////////////////

void
ProcessStep::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(ProcessStep));
    const ProcessStep& ps = (const ProcessStep&)rhs;
    _id = ps._id;
    _name = ps._name;
    _processingTime = ps._processingTime;
    _ptPer = ps._ptPer;
    _ptBatchSize = ps._ptBatchSize;
    _fixedCost = ps._fixedCost;
    _fcPer = ps._fcPer;
    _fcBatchSize = ps._fcBatchSize;
    _itemReqs = ps._itemReqs;
    _resReqs = ps._resReqs;
    _resCapPts = ps._resCapPts;
}

//////////////////////////////////////////////////////////////////////////////

void
ProcessStep::serialize(Stream& stream, uint_t io, uint_t)
{
    utl::serialize(_id, stream, io);
    lut::serialize(_name, stream, io);
    utl::serialize(_processingTime, stream, io);
    utl::serialize((uint_t&)_ptPer, stream, io);
    utl::serialize(_ptBatchSize, stream, io);
    utl::serialize(_fixedCost, stream, io);
    utl::serialize((uint_t&)_fcPer, stream, io);
    utl::serialize(_fcBatchSize, stream, io);
    _itemReqs.serialize(stream, io);
    _resReqs.serialize(stream, io);
    _resCapPts.serialize(stream, io);
}

//////////////////////////////////////////////////////////////////////////////

JobOp*
ProcessStep::createJobOp(uint_t id, uint_t quantity)
{
    JobOp* op = new JobOp();
    op->id() = id;
    op->name() = _name;
    if (_processingTime != uint_t_max)
    {
        if (_ptPer == pt_per_batch)
        {
            if (_ptBatchSize == uint_t_max)
            {
                op->processingTime() = _processingTime;
            }
            else
            {
                ASSERT(_ptBatchSize != 0);
                op->processingTime() = _processingTime *
                    (uint_t)ceil((double)quantity / (double)_ptBatchSize);
            }
        }
        else // pt_per_piece or pt_per_undefined
        {
            op->processingTime() = _processingTime * quantity;
        }
    }
    if (_fixedCost != 0.0)
    {
        if (_fcPer == fc_per_batch)
        {
            if (_fcBatchSize == uint_t_max)
            {
                op->cost() = _fixedCost;
            }
            else
            {
                ASSERT(_fcBatchSize != 0);
                op->cost() = _fixedCost *
                    ceil((double)quantity / (double)_fcBatchSize);
            }
        }
        else // fc_per_piece or fc_per_undefined
        {
            op->cost() = _fixedCost * (double)quantity;
        }
    }
    op->type() = op_normal;
    op->status() = opstatus_unstarted;
    op->scheduledBy() = sa_clevor;
    return op;
}

//////////////////////////////////////////////////////////////////////////////

String
ProcessStep::toString() const
{
    MemStream str;
    str << "processstep:" << _id
        << ", name:" << _name.c_str()
        << ", pt:" << _processingTime;
    if(_ptPer == pt_per_batch)
    {
        if (_ptBatchSize != uint_t_max)
        {
            str << "/batch("
                << _ptBatchSize << ")";
        }
        else
        {
            str << "/workorder";
        }
    }
    else
    {
        str << "/piece";
    }
    if (_fixedCost != uint_t_max && _fixedCost != 0.0)
    {
        str << ", fixedCost:$" << _fixedCost;
        if (_fcPer == fc_per_batch)
        {
            if (_fcBatchSize != uint_t_max)
            {
                str << "/batch("
                    << _fcBatchSize << ")";
            }
            else
            {
                str << "/workorder";
            }
        }
        else
        {
            str << "/piece";
        }
    }
    forEachIt(Array, _itemReqs, StepItemRequirement, itemReq)
        str << '\n' << "   "
            << itemReq.toString();
    endForEach
    forEachIt(Array, _resReqs, StepResourceRequirement, resReq)
        str << '\n' << "   "
            << resReq.toString();
    endForEach
    forEachIt(Array, _resCapPts, StepAltResCapPts, capPts)
        str << '\n' << "   "
            << capPts.toString();
    endForEach

    str << '\0';
    return String((char*)str.get());
}

//////////////////////////////////////////////////////////////////////////////

MRP_NS_END;
