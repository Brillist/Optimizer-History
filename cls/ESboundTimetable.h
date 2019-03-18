#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <cls/TimetableBound.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Earliest time when a DiscreteResource has the required capacity.

   \see DiscreteResourceTimetable
   \see ESbound
   \ingroup cls
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class ESboundTimetable : public TimetableBound
{
    UTL_CLASS_DECL(ESboundTimetable, TimetableBound);

public:
    /**
       Constructor.
       \param act activity this bound is computed for
       \param rcp all of the resource's `[capacity,processing-time]` pairings
       \param capPt `[capacity,processing-time]` pair this bound is computed for
       \param lb initial lower bound
    */
    ESboundTimetable(BrkActivity* act, ResourceCapPts* rcp, const CapPt* capPt, int lb)
        : TimetableBound(act, rcp, capPt, clp::bound_lb, lb)
    {
    }

    /// \name Capacity Allocation
    //@{
    virtual void allocateCapacity();

    virtual void deallocateCapacity();

    virtual void allocateCapacity(int t1, int t2);

    virtual void deallocateCapacity(int t1, int t2);
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
