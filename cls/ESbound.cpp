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
ESbound::registerEvents()
{
    // register for timetable events
    bound_array_t::iterator it, endIt = _bounds.end();
    for (it = _bounds.begin(); it != endIt; ++it)
    {
        Bound* bound = *it;
        if (!bound->isA(ESboundTimetable))
        {
            continue;
        }
        ESboundTimetable* ttb = (ESboundTimetable*)bound;
        ttb->registerEvents(this);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ESbound::allocateCapacity()
{
    // allocate resource capacity
    bound_array_t::iterator it, endIt = _bounds.end();
    for (it = _bounds.begin(); it != endIt; ++it)
    {
        Bound* bound = *it;
        if (!bound->isA(ESboundTimetable))
        {
            continue;
        }
        ESboundTimetable* ttb = (ESboundTimetable*)bound;
        ttb->deregisterEvents(this);
        ttb->allocateCapacity();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ESbound::allocateCapacity(int t1, int t2)
{
    // allocate resource capacity
    bound_array_t::iterator it, endIt = _bounds.end();
    for (it = _bounds.begin(); it != endIt; ++it)
    {
        Bound* bound = *it;
        if (!bound->isA(ESboundTimetable))
        {
            continue;
        }
        ESboundTimetable* ttb = (ESboundTimetable*)bound;
        ttb->allocateCapacity(t1, t2);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ESbound::deallocateCapacity()
{
    // deallocate resource capacity
    bound_array_t::iterator it, endIt = _bounds.end();
    for (it = _bounds.begin(); it != endIt; ++it)
    {
        Bound* bound = *it;
        if (!bound->isA(ESboundTimetable))
        {
            continue;
        }
        ESboundTimetable* ttb = (ESboundTimetable*)bound;
        ttb->deallocateCapacity();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ESbound::deallocateCapacity(int t1, int t2)
{
    // deallocate resource capacity
    bound_array_t::iterator it, endIt = _bounds.end();
    for (it = _bounds.begin(); it != endIt; ++it)
    {
        Bound* bound = *it;
        if (!bound->isA(ESboundTimetable))
        {
            continue;
        }
        ESboundTimetable* ttb = (ESboundTimetable*)bound;
        ttb->deallocateCapacity(t1, t2);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ESbound::add(Bound* bound)
{
    _bounds.add(bound);
    invalidate();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int
ESbound::find()
{
    saveState();

    // find the activity
    ASSERTD(_owner->isA(BrkActivity));
    BrkActivity* act = (BrkActivity*)_owner;

    int oldBound = _bound; //only for setAllocatedLB later

    do
    {
        _findPoint = int_t_max;
        bool first = true;
        bound_array_t::iterator it;
        bound_array_t::iterator endIt = _bounds.end();
        for (it = _bounds.begin(); (it != endIt) && (_bound <= _findPoint); ++it)
        {
            Bound* itBound = *it;
            itBound->invalidate();
            itBound->setLB(_bound);
            _bound = itBound->get();
            if (_bound == int_t_max)
            {
                String& str = *new String();
                str = _name + ": ";
                str += "no workable bound";
                throw FailEx(str);
            }

            // first iter?
            if (first)
            {
                first = false;
                _findPoint = _bound;
                // only track calendar if bound is already finalized
                if (act->allocated())
                    break; //why? joez
            }
        }
    } while (_bound > _findPoint);

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
    ASSERTD(_act->allocated());
    ASSERTD(oldBound < newBound);
    utl::cout << "WARNING: moving a scheduled activity:" << _act->id() << ", es:" << oldBound
              << "->" << newBound << utl::endlf;
    deallocateCapacity(oldBound, min(newBound - 1, _efBound->get()));
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
    ConstrainedBound::setLB(lb);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_END;
