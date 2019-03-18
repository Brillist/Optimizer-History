#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <clp/ConstrainedBound.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Schedulable bound.

   In forward scheduling, a SchedulableBound is a ConstrainedBound that finds the earliest valid
   time for an Activity to begin execution.  A SchedulableBound watches for events that can
   invalidate the last computation it did (\ref registerEvents), and when the Activity is scheduled
   it can allocate resource capacity (\ref allocateCapacity), which may cause other
   SchedulableBound%s to be notified and recalculate their own values.

   \see ESbound
   \see LFbound
   \see ESboundInt
   \see LFboundInt
   \ingroup cls
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class SchedulableBound : public clp::ConstrainedBound
{
    UTL_CLASS_DECL_ABC(SchedulableBound, clp::ConstrainedBound);
    UTL_CLASS_DEFID;

public:
    using bound_array_t = clp::RevArray<clp::Bound*>;

public:
    /**
       Constructor.
       \param mgr related Manager
       \param type bound type (lower or upper)
       \param bound initial value
    */
    SchedulableBound(clp::Manager* mgr, clp::bound_t type, int bound)
        : ConstrainedBound(mgr, type, bound)
    {
    }

    /** Register for events. */
    virtual void registerEvents() = 0;

    /** Allocate capacity (and deregister for events). */
    virtual void allocateCapacity() = 0;

    /** De-allocate capacity. */
    virtual void deallocateCapacity() = 0;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_END;
