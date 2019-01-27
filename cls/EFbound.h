#ifndef CLS_EFBOUND_H
#define CLS_EFBOUND_H

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <clp/ConstrainedBound.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

class ESbound;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Earliest valid time for an activity to end execution on a resource.

   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class EFbound : public clp::ConstrainedBound
{
    UTL_CLASS_DECL(EFbound, clp::ConstrainedBound);
    UTL_CLASS_DEFID;

public:
    /** Constructor. */
    EFbound(ESbound* esBound, int lb);

    /** Get the es-bound. */
    cls::ESbound*&
    esBound()
    {
        return _esBound;
    }

    /** Set the find-point. */
    void
    setFindPoint(int findPoint)
    {
        _findPoint = findPoint;
    }

    /** Set new bound.*/
    virtual void setLB(int lb);

protected:
    virtual int find();

private:
    ESbound* _esBound;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_END;

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
