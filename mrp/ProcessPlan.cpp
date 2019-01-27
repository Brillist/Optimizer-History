#include "libmrp.h"
#include <libutl/MemStream.h>
#include <libutl/BufferedFDstream.h>
#include "ProcessPlan.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(mrp::ProcessPlan);

////////////////////////////////////////////////////////////////////////////////////////////////////

MRP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

bool
PlanOrderingIncId::operator()(const ProcessPlan* lhs, const ProcessPlan* rhs) const
{
    return (lhs->id() < rhs->id());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ProcessPlan::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(ProcessPlan));
    const ProcessPlan& pp = (const ProcessPlan&)rhs;
    _id = pp._id;
    _name = pp._name;
    _version = pp._version;
    copyVector(_plansteps, pp._plansteps);
    //     _plansteps = pp._plansteps;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int
ProcessPlan::compare(const Object& rhs) const
{
    if (!rhs.isA(ProcessPlan))
    {
        return Object::compare(rhs);
    }
    const ProcessPlan& pp = (const ProcessPlan&)rhs;
    int res = utl::compare(_id, pp._id);
    return res;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ProcessPlan::serialize(Stream& stream, uint_t io, uint_t)
{
    utl::serialize(_id, stream, io);
    lut::serialize(_name, stream, io);
    lut::serialize(_version, stream, io);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

String
ProcessPlan::toString() const
{
    MemStream str;
    str << "processplan:" << _id << ", name:" << _name.c_str() << ", version:" << _version.c_str();
    for (planstep_vector_t::const_iterator it = _plansteps.begin(); it != _plansteps.end(); it++)
    {
        PlanStepRelation* planstep = *it;
        str << '\n' << "   " << planstep->toString();
    }
    str << '\0';
    return String((char*)str.get());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

MRP_NS_END;
