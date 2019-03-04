#include "libcls.h"
#include <clp/FailEx.h>
#include "LFboundTimetable.h"
#include "LFbound.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;
CLP_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(cls::LFbound);

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

LFbound::LFbound(clp::Manager* mgr, int ub)
    : SchedulableBound(mgr, clp::bound_ub, ub)
{
    _lsBound = nullptr;
    _bounds.initialize(mgr);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
LFbound::registerEvents()
{
    // register for timetable events
    bound_array_t::iterator it, endIt = _bounds.end();
    for (it = _bounds.begin(); it != endIt; ++it)
    {
        Bound* bound = *it;
        if (!bound->isA(LFboundTimetable))
        {
            continue;
        }
        LFboundTimetable* ttb = (LFboundTimetable*)bound;
        ttb->registerEvents(this);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
LFbound::allocateCapacity()
{
    // allocate resource capacity
    bound_array_t::iterator it, endIt = _bounds.end();
    for (it = _bounds.begin(); it != endIt; ++it)
    {
        Bound* bound = *it;
        if (!bound->isA(LFboundTimetable))
        {
            continue;
        }
        LFboundTimetable* ttb = (LFboundTimetable*)bound;
        ttb->deregisterEvents(this);
        ttb->allocateCapacity();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
LFbound::deallocateCapacity()
{
    // allocate resource capacity
    bound_array_t::iterator it, endIt = _bounds.end();
    for (it = _bounds.begin(); it != endIt; ++it)
    {
        Bound* bound = *it;
        if (!bound->isA(LFboundTimetable))
        {
            continue;
        }
        LFboundTimetable* ttb = (LFboundTimetable*)bound;
        ttb->deallocateCapacity();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
LFbound::add(Bound* bound)
{
    _bounds.add(bound);
    invalidate();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int
LFbound::find()
{
    // find the activity
    ASSERTD(_owner->isA(BrkActivity));
    BrkActivity* act = (BrkActivity*)_owner;

    if (act->allocated())
    {
        deallocateCapacity();
    }

    do
    {
        _findPoint = int_t_min;
        bool first = true;
        bound_array_t::iterator it;
        bound_array_t::iterator endIt = _bounds.end();
        for (it = _bounds.begin(); (it != endIt) && (_bound >= _findPoint); ++it)
        {
            Bound* itBound = *it;
            itBound->invalidate();
            itBound->setUB(_bound);
            _bound = itBound->get();
            if (_bound == int_t_min)
            {
                throw FailEx(_name + ": no workable bound");
            }

            // first iter?
            if (first)
            {
                first = false;
                _findPoint = _bound;
                // only track calendar if bound is already finalized
                if (act->allocated())
                    break;
            }
        }
    } while (_bound < _findPoint);

    if (act->allocated())
    {
        allocateCapacity();
    }

    return _bound;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_END;
