#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <clp/RevArray.h>
#include <cls/BrkActivity.h>
#include <cls/EFbound.h>
#include <cls/SchedulableBound.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Earliest valid time for a BrkActivity to begin execution.

   ESbound reconciles one or more provided Bound%s to determine the earliest valid starting time
   for a BrkActivity.  When an ESbound moves, it pushes its corresponding EFbound forward.
   
   Two types of bounds are normally tracked:

   - ESboundCalendar : to find the next time slot when a required resource is available for work
     (according to its calendar)
   - ESboundTimetable : to find the next time slot when a resource has the required available
     capacity (according to its timetable)

   ESbound overrides SchedulableBound::allocateCapacity to allocate capacity in the timetables
   of required DiscreteResource%s.

   \see ESboundCalendar
   \see ESboundTimetable
   \ingroup cls
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class ESbound : public SchedulableBound
{
    friend class EFbound;
    UTL_CLASS_DECL(ESbound, SchedulableBound);

public:
    using bound_array_t = clp::RevArray<clp::Bound*>;

public:
    /**
       Constructor.
       \param mgr related Manager
       \param lb initial lower bound
    */
    ESbound(clp::Manager* mgr, int lb);

    /** Add a lower-bound. */
    void add(Bound* bound);

    /** Set new bound. */
    virtual void setLB(int lb);

    /** Register for events  */
    virtual void registerEvents();

    /// \name Capacity Allocation
    //@{
    /** Allocate capacity. */
    virtual void allocateCapacity();

    /** Deallocate capacity. */
    virtual void deallocateCapacity();

    /** Allocate capacity from time t1 to t2. */
    virtual void allocateCapacity(int t1, int t2);

    /** Deallocate capacity from time t1 to t2. */
    virtual void deallocateCapacity(int t1, int t2);
    //@}

    /// \name Accessors (const)
    //@{
    /** Get the activity. */
    const BrkActivity*
    activity() const
    {
        return _act;
    }

    /** Get the earliest-finish bound. */
    const EFbound*
    efBound() const
    {
        return _efBound;
    }
    //@}

    /// \name Accessors (non-const)
    //@{
    /** Set the activity. */
    void
    setActivity(BrkActivity* act)
    {
        _act = act;
    }

    /** Set the earliest-finish bound. */
    void
    setEFbound(EFbound* efBound)
    {
        _efBound = efBound;
    }
    //@}

protected:
    virtual int find();
    virtual void setAllocatedLB(int oldBound, int newBound);

private:
    void
    init()
    {
        ABORT();
    }

    void
    deInit()
    {
    }

private:
    BrkActivity* _act;
    EFbound* _efBound;
    bound_array_t _bounds;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_END;
