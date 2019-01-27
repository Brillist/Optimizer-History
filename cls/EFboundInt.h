#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <clp/ConstrainedBound.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

class ESboundInt;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   EF bound for interruptible activity.

   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class EFboundInt : public clp::ConstrainedBound
{
    UTL_CLASS_DECL(EFboundInt, clp::ConstrainedBound);
    UTL_CLASS_DEFID;

public:
    /** Constructor. */
    EFboundInt(ESboundInt* esBound, int lb);

    /** Get the es-bound. */
    cls::ESboundInt*&
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

protected:
    virtual int find();

private:
    ESboundInt* _esBound;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_END;
