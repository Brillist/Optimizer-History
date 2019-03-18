#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <clp/ConstrainedBound.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

class ESboundInt;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Earliest valid time for an IntActivity to end execution on a resource.

   \ingroup cls
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class EFboundInt : public clp::ConstrainedBound
{
    UTL_CLASS_DECL(EFboundInt, clp::ConstrainedBound);
    UTL_CLASS_DEFID;

public:
    /** Constructor. */
    EFboundInt(ESboundInt* esBound, int lb);

    /// \name Accessors (const)
    //@{
    /** Get the related ESboundInt. */
    const cls::ESboundInt* esBound()
    {
        return _esBound;
    }
    //@}

    /// \name Accessors (non-const)
    //@{
    /** Set the related ESboundInt. */
    void setESbound(ESboundInt* esBound)
    {
        _esBound = esBound;
    }

    /** Set the find-point. */
    void
    setFindPoint(int findPoint)
    {
        _findPoint = findPoint;
    }
    //@}

protected:
    virtual int find();

private:
    ESboundInt* _esBound;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_END;
