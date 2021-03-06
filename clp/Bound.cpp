#include "libclp.h"
#include "Bound.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(clp::Bound);

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

Bound::Bound(Manager* mgr, bound_t type, int bound)
{
    ASSERTD(mgr != nullptr);
    _mgr = mgr;
    _debugFlag = false;
    _type = type;
    _stateDepth = _mgr->depth();
    if (bound == int_t_min)
    {
        _bound = loosest();
    }
    else
    {
        _bound = bound;
    }
    _findPoint = loosest();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

const int&
Bound::get()
{
    if (!valid())
        find();
    return _bound;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int
Bound::find()
{
    _findPoint = tightest();
    return _bound;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Bound::_saveState()
{
    ASSERTD(_stateDepth < _mgr->depth());
    _mgr->revSet(&_stateDepth, 3);
    _stateDepth = _mgr->depth();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_END;
