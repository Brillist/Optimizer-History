#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <clp/ConstrainedBound.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

class ESbound;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Earliest valid time for a BrkActivity to complete execution on a resource.

   \ingroup cls
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class EFbound : public clp::ConstrainedBound
{
    UTL_CLASS_DECL(EFbound, clp::ConstrainedBound);
    UTL_CLASS_DEFID;

public:
    /**
       Constructor.
       \param esBound related ESbound
       \param lb initial value
    */
    EFbound(ESbound* esBound, int lb);

    /** Set new bound.*/
    virtual void setLB(int lb);

    /// \name Accessors (const)
    //@{
    /** Get the ESbound. */
    const ESbound*
    esBound() const
    {
        return _esBound;
    }
    //@}

    /// \name Accessors (non-const)
    //@{
    /** Set the ESbound. */
    void
    setESbound(ESbound* esBound)
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
    ESbound* _esBound;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_END;
