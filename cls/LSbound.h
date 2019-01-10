#ifndef CLS_LSBOUND_H
#define CLS_LSBOUND_H

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <clp/ConstrainedBound.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

class LFbound;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Earliest valid time for an activity to end execution on a resource.

   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class LSbound : public clp::ConstrainedBound
{
    UTL_CLASS_DECL(LSbound);
    UTL_CLASS_DEFID;

public:
    /** Constructor. */
    LSbound(LFbound* lfBound, int ub);

    /** Get the es-bound. */
    cls::LFbound*&
    lfBound()
    {
        return _lfBound;
    }

    /** Set the find-point. */
    void
    setFindPoint(int findPoint)
    {
        _findPoint = findPoint;
    }

protected:
    virtual int find();

private:
    LFbound* _lfBound;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_END;

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
