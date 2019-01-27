#ifndef CLS_ESBOUNDTIMETABLE_H
#define CLS_ESBOUNDTIMETABLE_H

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <cls/TimetableBound.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   An activity's execution time cannot overlap with any period of
   insufficient available resource capacity.

   \author Adam McKee
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

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
