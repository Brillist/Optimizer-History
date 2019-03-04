#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <clp/ConstrainedBound.h>
#include <clp/Manager.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Bound constraint.

   BoundCt forces a destination bound to maintain a required relationship with a source bound.

   \see Manager
   \ingroup clp
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class BoundCt : public utl::Object
{
    UTL_CLASS_DECL(BoundCt, utl::Object);
    UTL_CLASS_NO_COPY;

public:
    /**
       Constructor.
       \param val initially propagated value for destination bound
       \param srcBound source bound
       \param dstBound destination bound
    */
    BoundCt(int val, ConstrainedBound* srcBound, ConstrainedBound* dstBound);

    /// \name Accessors (const)
    //@{
    int
    v() const
    {
        return _v;
    }

    /** Get the source bound. */
    ConstrainedBound*
    srcBound() const
    {
        return _srcBound;
    }

    /** Get the destination bound. */
    ConstrainedBound*
    dstBound() const
    {
        return _dstBound;
    }
    //@}

    /// \name Accessors (non-const)
    //@{
    void
    setV(int val)
    {
        _v = val;
    }
    //@}

    /// \name Modification
    /** Apply pressure to lower bound. */
    void
    increment(uint_t inc)
    {
        saveState();
        _v += inc;
        _dstBound->setLB(_v);
    }

    /** Apply pressure to upper bound. */
    void
    decrement(uint_t dec)
    {
        saveState();
        _v -= dec;
        _dstBound->setUB(_v);
    }
    //@}

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

    /** Save state so it can be restored when backtracking. */
    void
    saveState()
    {
        uint_t d = _mgr->depth();
        if (d == _stateDepth)
            return;
        ASSERTD(d > _stateDepth);
        _mgr->revSet(&_stateDepth, 2U);
        _stateDepth = d;
    }

private:
    Manager* _mgr;
    uint_t _stateDepth;
    int _v;
    ConstrainedBound* _srcBound;
    ConstrainedBound* _dstBound;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_END;
