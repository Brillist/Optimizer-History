#include "libcls.h"
#include <libutl/BufferedFDstream.h>
#include <clp/FailEx.h>
#include "ESboundTimetable.h"
#include "ESbound.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;
CLP_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(cls::ESbound);

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

ESbound::ESbound(clp::Manager* mgr, int lb)
    : SchedulableBound(mgr, clp::bound_lb, lb)
{
    _efBound = nullptr;
    _bounds.initialize(mgr);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ESbound::add(Bound* bound)
{
    _bounds.add(bound);
    invalidate();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ESbound::setLB(int lb)
{
    if (lb <= _bound)
        return;
    if (_act->allocated())
    {
        setAllocatedLB(_bound, lb);
    }
    super::setLB(lb);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ESbound::registerEvents()
{
    // register for timetable events
    for (auto bound : _bounds)
    {
        if (!bound->isA(ESboundTimetable))
        {
            continue;
        }
        auto ttb = utl::cast<ESboundTimetable>(bound);
        ttb->registerEvents(this);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ESbound::allocateCapacity()
{
    // allocate resource capacity
    for (auto bound : _bounds)
    {
        if (!bound->isA(ESboundTimetable))
        {
            continue;
        }
        auto ttb = utl::cast<ESboundTimetable>(bound);
        ttb->deregisterEvents(this);
        ttb->allocateCapacity();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ESbound::deallocateCapacity()
{
    // deallocate resource capacity
    for (auto bound : _bounds)
    {
        if (!bound->isA(ESboundTimetable))
        {
            continue;
        }
        auto ttb = utl::cast<ESboundTimetable>(bound);
        ttb->deallocateCapacity();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ESbound::allocateCapacity(int t1, int t2)
{
    // allocate resource capacity
    for (auto bound : _bounds)
    {
        if (!bound->isA(ESboundTimetable))
        {
            continue;
        }
        auto ttb = utl::cast<ESboundTimetable>(bound);
        ttb->allocateCapacity(t1, t2);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ESbound::deallocateCapacity(int t1, int t2)
{
    // deallocate resource capacity
    for (auto bound : _bounds)
    {
        if (!bound->isA(ESboundTimetable))
        {
            continue;
        }
        auto ttb = utl::cast<ESboundTimetable>(bound);
        ttb->deallocateCapacity(t1, t2);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int
ESbound::find()
{
    saveState();

    // reference the activity
    auto act = utl::cast<BrkActivity>(_owner);

    // so we can determine whether _bound moved later
    int oldBound = _bound;

    // reconcile _bound with _bounds until _bound stabilizes
    do
    {
        _findPoint = int_t_max;
        bool first = true;
        for (auto bound : _bounds)
        {
            // don't exceed _findPoint
            if (_bound > _findPoint)
            {
                break;
            }

            // reconcile _bound with bound (_bound -> bound, bound -> _bound)
            bound->invalidate();
            bound->setLB(_bound);
            _bound = bound->get();

            // can't find a workable value?
            if (_bound == int_t_max)
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
    } while (_bound > _findPoint);

    // moving a scheduled activity?
    if (act->allocated() && (oldBound < _bound))
    {
        setAllocatedLB(oldBound, _bound);
    }

    return _bound;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ESbound::setAllocatedLB(int oldBound, int newBound)
{
    // deallocate capacity before moving a previously scheduled activity
    ASSERTD(_act->allocated());
    ASSERTD(oldBound < newBound);
    utl::cout << "WARNING: moving a scheduled activity:" << _act->id() << ", es:" << oldBound
              << "->" << newBound << utl::endlf;
    deallocateCapacity(oldBound, min(newBound - 1, _efBound->get()));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_END;
