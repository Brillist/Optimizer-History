#ifndef CLS_SCHEDULABLEBOUND_H
#define CLS_SCHEDULABLEBOUND_H

//////////////////////////////////////////////////////////////////////////////

#include <clp/ConstrainedBound.h>

//////////////////////////////////////////////////////////////////////////////

CLS_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////

/**
   Schedulable bound.

   \author Adam McKee
*/

//////////////////////////////////////////////////////////////////////////////

class SchedulableBound : public clp::ConstrainedBound
{
    UTL_CLASS_DECL_ABC(SchedulableBound);
    UTL_CLASS_DEFID;
public:
    typedef clp::RevArray<clp::Bound*> bound_array_t;
public:
    /** Constructor. */
    SchedulableBound(clp::Manager* mgr, clp::bound_t type, int bound)
        : ConstrainedBound(mgr, type, bound) {}

    /** Register for events. */
    virtual void registerEvents()=0;

    /** Allocate capacity (and deregister for events). */
    virtual void allocateCapacity()=0;

    /** De-allocate capacity. */
    virtual void deallocateCapacity()=0;
};

//////////////////////////////////////////////////////////////////////////////

CLS_NS_END;

//////////////////////////////////////////////////////////////////////////////

#endif
