#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <clp/RevArray.h>
#include <cls/BrkActivity.h>
#include <cls/LSbound.h>
#include <cls/SchedulableBound.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Latest valid time for a BrkActivity to begin execution.

   LFbound is a mirror of the ESbound class for backward scheduling.

   \see LFboundTimetable
   \see LFboundCalendar
   \ingroup cls
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class LFbound : public SchedulableBound
{
    friend class LSbound;
    UTL_CLASS_DECL(LFbound, SchedulableBound);

public:
    using bound_array_t = clp::RevArray<clp::Bound*>;

public:
    /**
       Constructor.
       \param mgr related Manager
       \param ub initial upper bound
    */
    LFbound(clp::Manager* mgr, int ub);

    /** Add a lower-bound. */
    void add(Bound* bound);

    /** Register for events. */
    virtual void registerEvents();

    /// \name Capacity Allocation
    //@{
    /** Allocate capacity. */
    virtual void allocateCapacity();

    /** Deallocate capacity. */
    virtual void deallocateCapacity();
    //@}

    /// \name Accessors
    //@{
    /** Get the latest-start bound. */
    const LSbound*
    lsBound() const
    {
        return _lsBound;
    }

    /** Set the latest-start bound. */
    void setLSbound(LSbound* lsBound)
    {
        _lsBound = lsBound;
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
