#include "libmrp.h"
#include <libutl/MemStream.h>
#include <mrp/ProcessPlan.h>
#include <mrp/ProcessStep.h>
#include "PlanStepRelation.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(mrp::PlanStepRelation);

////////////////////////////////////////////////////////////////////////////////////////////////////

MRP_NS_BEGIN;

///////////////////////////////////////////////////////////////////////////////////////////////////

void
PlanStepRelation::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(PlanStepRelation));
    const PlanStepRelation& psr = (const PlanStepRelation&)rhs;
    _planOwner = psr._planOwner;
    if (_planOwner)
        _plan = utl::clone(psr._plan);
    else
        _plan = psr._plan;
    _stepOwner = psr._stepOwner;
    if (_stepOwner)
        _step = utl::clone(psr._step);
    else
        _step = psr._step;
    _succStepOwner = psr._succStepOwner;
    if (_succStepOwner)
        _succStep = utl::clone(psr._succStep);
    else
        _succStep = psr._succStep;
    _type = psr._type;
    _lag = psr._lag;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
PlanStepRelation::serialize(Stream& stream, uint_t io, uint_t)
{
    utl::serialize(_plan->id(), stream, io);
    utl::serialize(_step->id(), stream, io);
    if (_succStep)
    {
        utl::serialize(_succStep->id(), stream, io);
    }
    else
    {
        utl::serialize((uint_t&)uint_t_max, stream, io);
    }
    utl::serialize((uint_t&)_type, stream, io);
    utl::serialize(_lag, stream, io);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
PlanStepRelation::setPlan(ProcessPlan* plan, bool owner)
{
    if (_planOwner)
        delete _plan;
    _plan = plan;
    _planOwner = owner;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
PlanStepRelation::setStep(ProcessStep* step, bool owner)
{
    if (_stepOwner)
        delete _step;
    _step = step;
    _stepOwner = owner;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
PlanStepRelation::setSuccStep(ProcessStep* step, bool owner)
{
    if (_succStepOwner)
        delete _succStep;
    _succStep = step;
    _succStepOwner = owner;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

cse::PrecedenceCt*
PlanStepRelation::createPrecedenceCt()
{
    cse::PrecedenceCt* pct = new cse::PrecedenceCt();
    pct->lhsOpId() = _step->id();
    pct->rhsOpId() = _succStep->id();
    pct->type() = _type;
    pct->delay() = _lag;
    return pct;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

String
PlanStepRelation::toString() const
{
    MemStream str;
    str << "PlanStepRelation: plan:" << _plan->id() << ", step:" << _step->id() << ", succStep:";
    if (_succStep)
        str << _succStep->id();
    else
        str << "null";
    str << ", type:" << _type << ", lag:" << _lag << '\0';
    return String((char*)str.get());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
PlanStepRelation::init()
{
    _plan = new ProcessPlan();
    _step = new ProcessStep();
    _succStep = new ProcessStep();
    _type = cse::pct_undefined;
    _lag = 0;
    _planOwner = true;
    _stepOwner = true;
    _succStepOwner = true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
PlanStepRelation::deInit()
{
    setPlan(nullptr);
    setStep(nullptr);
    setSuccStep(nullptr);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

MRP_NS_END;
