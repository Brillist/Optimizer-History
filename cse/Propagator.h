#ifndef CSE_PROPAGATOR_H
#define CSE_PROPAGATOR_H

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <clp/BoundPropagator.h>
#include <cse/SchedulingContext.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Unary constraint.

   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class Propagator : public clp::BoundPropagator
{
    UTL_CLASS_DECL(Propagator);

public:
    Propagator(SchedulingContext* context)
        : BoundPropagator(context->manager())
    {
        _context = context;
    }

    // unsuspend() and finalize() are for overriding
    // the corresponding methods in clp::BoundPropagator
    virtual void unsuspend(clp::ConstrainedBound* cb);

    virtual void finalize(clp::ConstrainedBound* cb);

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
    SchedulingContext* _context;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
