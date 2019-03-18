#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <cls/TimetableBound.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

class LFbound;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Latest time when a DiscreteResource has the required capacity.

   LFboundTimetable is a mirror of the ESboundTimetable class for backward scheduling.

   \see DiscreteResourceTimetable
   \see LFbound
   \ingroup cls
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class LFboundTimetable : public TimetableBound
{
    UTL_CLASS_DECL(LFboundTimetable, TimetableBound);

public:
    /**
       Constructor.
       \param act activity this bound is computed for
       \param rcp all of the resource's `[capacity,processing-time]` pairings
       \param capPt `[capacity,processing-time]` pair this bound is computed for
       \param ub initial upper bound
    */
    LFboundTimetable(BrkActivity* act, ResourceCapPts* rcp, const CapPt* capPt, int ub)
        : TimetableBound(act, rcp, capPt, clp::bound_ub, ub)
    {
    }

    /// \name Capacity Allocation
    //@{
    virtual void allocateCapacity();

    virtual void deallocateCapacity();
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
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_END;
