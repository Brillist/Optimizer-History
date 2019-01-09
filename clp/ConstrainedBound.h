#ifndef CLP_CONSTRAINEDBOUND_H
#define CLP_CONSTRAINEDBOUND_H

//////////////////////////////////////////////////////////////////////////////

#include <clp/Bound.h>
#include <clp/ConstrainedVar.h>
#include <clp/RevArray.h>

//////////////////////////////////////////////////////////////////////////////

CLP_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////

class BoundCt;
class BoundPropagator;
class CycleGroup;

//////////////////////////////////////////////////////////////////////////////

/**
   Constrained/propagated bound.

   \author Adam McKee
*/

//////////////////////////////////////////////////////////////////////////////

class ConstrainedBound : public Bound
{
    friend class BoundPropagator;
    UTL_CLASS_DECL(ConstrainedBound);
    UTL_CLASS_NO_COPY;
public:
    typedef RevArray<BoundCt*> bct_array_t;
public:
    /** Constructor. */
    ConstrainedBound(
        Manager* mgr,
        bound_t type,
        int bound = utl::int_t_min);

    /// \name Accessors
    //@{
    virtual const int& get();

    /** Finalized? */
    bool finalized() const;

    /** Get the propagator. */
    BoundPropagator* boundPropagator() const
    { return _bp; }

    /** Get the owner. */
    utl::Object* owner() const
    { return _owner; }

    /** Get the owner. */
    utl::Object*& owner()
    { return _owner; }

    /** Set the twin bound. */
    void setTwinBound(Bound* twinBound)
    {
        if (twinBound == nullptr) _twinBound = nullptr;
        else _twinBound = &twinBound->getRef();
    }

    /** Get visited flag. */
    bool visited() const
    { return (_visitedIdx != utl::uint_t_max); }

    /** Get visited index. */
    utl::uint_t visitedIdx() const
    { return _visitedIdx; }

    /** Get visited index. */
    utl::uint_t& visitedIdx()
    { return _visitedIdx; }

    /** Already in propagation queue? */
    bool queued() const
    { return _queued; }

    /** Already in propagation queue? */
    bool& queued()
    { return _queued; }

    /** Get the cycle-group. */
    const CycleGroup* cycleGroup() const
    { return _cycleGroup; }

    /** Get the cycle-group. */
    CycleGroup* cycleGroup()
    { return _cycleGroup; }

    /** Set the cycle-group. */
    void setCycleGroup(CycleGroup* cycleGroup);

    /** Suspended? */
    bool suspended() const;

    /** Get the successor-depth. */
    utl::uint_t successorDepth() const;

    /** Get the last propagated bound. */
    int last() const
    { return _oldBound; }
    //@}

    /// \name Bound-Cts
    //@{
    /** Get the list of lb-cts. */
    const bct_array_t& lbCts() const
    { return _lbCts; }

    /** Get the list of ub-cts. */
    const bct_array_t& ubCts() const
    { return _ubCts; }
    //@}

    /// \name Update
    //@{
    /** Put the bound in the queue (and force find). */
    void queueFind();

    /** Put self into the propagation queue. */
    void enQ();

    /** Set lower bound. */
    virtual void setLB(int lb)
    {
        ASSERTD(_type == bound_lb);
        if (lb <= _bound) return;
#ifdef DEBUG
        if (_debugFlag)
        {
            BREAKPOINT;
        }
#endif
        saveState();
        _bound = lb;
        enQ();
    }

    /** Set upper bound. */
    virtual void setUB(int ub)
    {
        ASSERTD(_type == bound_ub);
        if (ub >= _bound) return;
#ifdef DEBUG
        if (_debugFlag)
        {
            BREAKPOINT;
        }
#endif
        saveState();
        _bound = ub;
        enQ();
    }

    /** Propagate. */
    void propagate()
    {
        if (_type == bound_lb) propagateLB();
        else propagateUB();
    }
    //@}
protected:
    void propagateLB();

    void propagateUB();

    bool addPred(ConstrainedBound* cb);

    virtual void _saveState();
protected:
    BoundPropagator* _bp;
    utl::Object* _owner;
    const int* _twinBound;
    utl::uint_t _visitedIdx;
    bool _queued;

    // reversible /////////////////
    CycleGroup* _cycleGroup;
    int _oldBound;
#if UTL_HOST_WORDSIZE == 64
    int _pad0;
#endif
    ///////////////////////////////

    // bound-cts
    bct_array_t _lbCts;
    bct_array_t _ubCts;
private:
    void init()
    { ABORT(); }
    void deInit();
};

//////////////////////////////////////////////////////////////////////////////

CLP_NS_END;

//////////////////////////////////////////////////////////////////////////////

#endif
