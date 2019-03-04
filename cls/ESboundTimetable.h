#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <cls/TimetableBound.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   An activity's execution cannot overlap with a period of insufficient resource capacity.

   \ingroup cls
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class ESboundTimetable : public TimetableBound
{
    UTL_CLASS_DECL(ESboundTimetable, TimetableBound);

public:
    /** Constructor. */
    ESboundTimetable(BrkActivity* act, ResourceCapPts* rcp, const CapPt* capPt, int lb)
        : TimetableBound(act, rcp, capPt, clp::bound_lb, lb)
    {
    }

    virtual void allocateCapacity();

    virtual void allocateCapacity(int t1, int t2);

    virtual void deallocateCapacity();

    virtual void deallocateCapacity(int t1, int t2);

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
