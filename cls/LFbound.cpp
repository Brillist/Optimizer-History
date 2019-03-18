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
LFbound::add(Bound* bound)
{
    _bounds.add(bound);
    invalidate();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
LFbound::registerEvents()
{
    // register for timetable events
    for (auto bound : _bounds)
    {
        if (!bound->isA(LFboundTimetable))
        {
            continue;
        }
        auto ttb = utl::cast<LFboundTimetable>(bound);
        ttb->registerEvents(this);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
LFbound::allocateCapacity()
{
    // allocate resource capacity
    for (auto bound : _bounds)
    {
        if (!bound->isA(LFboundTimetable))
        {
            continue;
        }
        auto ttb = utl::cast<LFboundTimetable>(bound);
        ttb->deregisterEvents(this);
        ttb->allocateCapacity();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
LFbound::deallocateCapacity()
{
    // deallocate resource capacity
    for (auto bound : _bounds)
    {
        if (!bound->isA(LFboundTimetable))
        {
            continue;
        }
        auto ttb = utl::cast<LFboundTimetable>(bound);
        ttb->deallocateCapacity();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int
LFbound::find()
{
    // reference the activity
    auto act = utl::cast<BrkActivity>(_owner);

    // already allocated?
    if (act->allocated())
    {
        // deallocate
        deallocateCapacity();
    }

    // reconcile with _bounds until _bound stabilizes
    do
    {
        _findPoint = int_t_min;
        bool first = true;
        for (auto bound : _bounds)
        {
            // don't pass _findPoint
            if (_bound < _findPoint)
            {
                break;
            }

            // reconcile with bound (_bound -> bound, bound -> _bound)
            bound->invalidate();
            bound->setUB(_bound);
            _bound = bound->get();

            // can't find a workable value?
            if (_bound == int_t_min)
            {
                throw FailEx(_name + ": no workable bound");
            }

            // first bound we processed?
            if (first)
            {
                // update _findPoint
                first = false;
                _findPoint = _bound;
                // bound is already finalized -> don't run the loop for 2nd and later bounds
                if (act->allocated())
                {
                    break;
                }
            }
        }
    } while (_bound < _findPoint);

    // moving a scheduled activity?
    if (act->allocated())
    {
        allocateCapacity();
    }

    return _bound;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_END;
