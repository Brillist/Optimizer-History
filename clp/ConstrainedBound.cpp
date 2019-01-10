#include "libclp.h"
#include "BoundCt.h"
#include "BoundPropagator.h"
#include "CycleGroup.h"
#include "FailEx.h"
#include "ConstrainedBound.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(clp::ConstrainedBound, clp::Bound);

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

ConstrainedBound::ConstrainedBound(Manager* mgr, bound_t boundType, int bound)
    : Bound(mgr, boundType, bound)
{
    ASSERTD(_mgr != nullptr);
    _bp = _mgr->boundPropagator();
    _owner = nullptr;
    _twinBound = nullptr;
    _visitedIdx = uint_t_max;
    _queued = false;
    _cycleGroup = nullptr;
    _oldBound = bound;
    _lbCts.initialize(_mgr);
    _ubCts.initialize(_mgr);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

const int&
ConstrainedBound::get()
{
    /*if (!valid()) find();
    if (_bound != _oldBound)
    {
        enQ();
    }*/
    return _bound;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool
ConstrainedBound::finalized() const
{
    return (_cycleGroup != nullptr) && _cycleGroup->finalized();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ConstrainedBound::setCycleGroup(CycleGroup* cycleGroup)
{
    if (cycleGroup == _cycleGroup)
    {
        return;
    }
    saveState();
    _cycleGroup = cycleGroup;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool
ConstrainedBound::suspended() const
{
    return (_cycleGroup != nullptr) && _cycleGroup->suspended();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

utl::uint_t
ConstrainedBound::successorDepth() const
{
    ASSERTD(_cycleGroup != nullptr);
    return _cycleGroup->successorDepth();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ConstrainedBound::queueFind()
{
    invalidate();
    enQ();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ConstrainedBound::enQ()
{
    if (_queued || suspended())
        return;
    _bp->enQ(this);
    _queued = true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ConstrainedBound::propagateLB()
{
    // do nothing if propagation of the bound is suspended
    if (suspended())
    {
        return;
    }

#ifdef DEBUG
    if (_debugFlag)
    {
        BREAKPOINT;
    }
#endif

    // find() a new bound?
    if (_bound > _findPoint)
    {
        find();
    }

    // no change to bound => do nothing
    if (_bound == _oldBound)
    {
        return;
    }

    // failure?
    if ((_twinBound != nullptr) && (_bound > *_twinBound))
    {
        String& str = *new String();
        str = _name + " > upper bound";
        throw FailEx(str);
    }

    saveState();

    // inc = distance of lower bound movement
    uint_t inc = (_bound - _oldBound);
    ASSERTD(inc > 0);

    // update lb-cts
    if (_lbCts.size() > 0)
    {
        BoundCt** it = _lbCts.begin();
        BoundCt** lim = it + _lbCts.size();
        while (it < lim)
        {
            BoundCt* ct = *(it++);
            ct->increment(inc);
        }
    }

    // update ub-cts
    if (_ubCts.size() > 0)
    {
        BoundCt** it = _ubCts.begin();
        BoundCt** lim = it + _ubCts.size();
        while (it < lim)
        {
            BoundCt* ct = *(it++);
            ct->decrement(inc);
        }
    }

    // new lower bound
    _oldBound = _bound;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ConstrainedBound::propagateUB()
{
    // do nothing if propagation of the bound is suspended
    if (suspended())
    {
        return;
    }

#ifdef DEBUG
    if (_debugFlag)
    {
        BREAKPOINT;
    }
#endif

    // find() a new bound?
    if (_bound < _findPoint)
    {
        find();
    }

    // no change to bound => do nothing
    if (_bound == _oldBound)
    {
        return;
    }

    // failure?
    if ((_twinBound != nullptr) && (_bound < *_twinBound))
    {
        String& str = *new String();
        str = _name + " < lower bound";
        throw FailEx(str);
    }

    saveState();

    // dec = distance of upper bound movement
    uint_t dec = (_oldBound - _bound);
    ASSERTD(dec > 0);

    // update lb-cts
    if (_lbCts.size() > 0)
    {
        BoundCt** it = _lbCts.begin();
        BoundCt** lim = it + _lbCts.size();
        while (it < lim)
        {
            BoundCt* ct = *(it++);
            ct->increment(dec);
        }
    }

    // update ub-cts
    if (_ubCts.size() > 0)
    {
        BoundCt** it = _ubCts.begin();
        BoundCt** lim = it + _ubCts.size();
        while (it < lim)
        {
            BoundCt* ct = *(it++);
            ct->decrement(dec);
        }
    }

    // new upper bound
    _oldBound = _bound;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ConstrainedBound::_saveState()
{
    Bound::_saveState();
    _mgr->revSet(&_cycleGroup, 2);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ConstrainedBound::deInit()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_END;
