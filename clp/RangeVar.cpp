#include "libclp.h"
#include "FailEx.h"
#include "RangeVar.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(clp::RangeVar);

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

RangeVar::RangeVar(ConstrainedBound* lb, ConstrainedBound* ub, bool failOnEmpty)
{
    ASSERTD(lb != nullptr);
    ASSERTD(ub != nullptr);

    init();

    // set bounds
    set(lb, ub);

    // configure fail-on-empty
    if (failOnEmpty)
    {
        _lb->setTwinBound(_ub);
        _ub->setTwinBound(_lb);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
RangeVar::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(RangeVar));
    const RangeVar& rv = (const RangeVar&)rhs;
    _name = rv._name;
    _object = rv._object;
    _lb = lut::clone(rv._lb);
    _ub = lut::clone(rv._ub);
    Manager* mgr = _lb->manager();
    ASSERTD(mgr != nullptr);
    mgr->revAllocate(_lb);
    mgr->revAllocate(_ub);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

String
RangeVar::toString() const
{
    std::ostringstream ss;
    if (lb() == ub())
    {
        ss << "[" << lb() << "]";
    }
    else
    {
        ss << "[" << lb() << ".." << ub() << "]";
    }
    return ss.str().c_str();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
RangeVar::setDebugFlag(bool debugFlag)
{
    if (_lb != nullptr)
        _lb->setDebugFlag(debugFlag);
    if (_ub != nullptr)
        _ub->setDebugFlag(debugFlag);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
RangeVar::setLB(ConstrainedBound* lb)
{
    _lb = lb;
    if (_lb != nullptr)
        _lb->setTwinBound(_ub);
    if (_ub != nullptr)
        _ub->setTwinBound(_lb);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
RangeVar::setUB(ConstrainedBound* ub)
{
    _ub = ub;
    if (_lb != nullptr)
        _lb->setTwinBound(_ub);
    if (_ub != nullptr)
        _ub->setTwinBound(_lb);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
RangeVar::init()
{
    _object = nullptr;
    _lb = nullptr;
    _ub = nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_END;
