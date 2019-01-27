#ifndef CLP_BOUNDCT_H
#define CLP_BOUNDCT_H

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <clp/ConstrainedBound.h>
#include <clp/Manager.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Bound constraint.

   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class BoundCt : public utl::Object
{
    UTL_CLASS_DECL(BoundCt, utl::Object);
    UTL_CLASS_NO_COPY;

public:
    /** Constructor. */
    BoundCt(int v, ConstrainedBound* srcBound, ConstrainedBound* dstBound);

    /// \name Accessors
    //@{
    int
    v() const
    {
        return _v;
    }

    int&
    v()
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

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
