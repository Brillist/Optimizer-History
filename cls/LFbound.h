#ifndef CLS_LFBOUND_H
#define CLS_LFBOUND_H

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <clp/RevArray.h>
#include <cls/BrkActivity.h>
#include <cls/LSbound.h>
#include <cls/SchedulableBound.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Latest valid time for an activity to end execution.

   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class LFbound : public SchedulableBound
{
    friend class LSbound;
    UTL_CLASS_DECL(LFbound);

public:
    typedef clp::RevArray<clp::Bound*> bound_array_t;

public:
    /** Constructor. */
    LFbound(clp::Manager* mgr, int ub);

    /** Register for events. */
    virtual void registerEvents();

    /** Allocate capacity. */
    virtual void allocateCapacity();

    /** Deallocate capacity. */
    virtual void deallocateCapacity();

    /** Add a lower-bound. */
    void add(Bound* bound);

    /// \name Accessors
    //@{
    /** Get the ls-bound. */
    const LSbound*
    lsBound() const
    {
        return _lsBound;
    }

    /** Get the ls-bound. */
    LSbound*&
    lsBound()
    {
        return _lsBound;
    }
    //@}
protected:
    virtual int find();

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
    LSbound* _lsBound;
    bound_array_t _bounds;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_END;

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
