#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <libutl/Object.h>
#include <clp/CycleGroup.h>
#include <clp/RevArray.h>
#include <clp/RevSet.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

class BoundCt;
class ConstrainedBound;
class Manager;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Bound propagator.

   BoundPropagator tracks precedence relationships between constrained bounds, and enforces
   constraints between bounds.  Propagation of a bound is a process where its attached constraints
   respond to the bound's movement by pushing other bounds in their natural direction of movement
   (lower bounds move up, upper bounds move down).  When propagation of a bound causes other
   bounds to move (to satisfy its constraints), *those* affected bounds are then subject to
   propagation.

   When a new precedence link is registered (by adding a constraint with \ref addBoundCt or
   by directly registering a precedence relationship with \ref addPrecedenceLink),
   BoundPropagator also updates the transitive closure of precedence relationships (so that
   `A->B` and `B->C` also implies `A->C`).  When updating the transitive closure, a cycle in the
   precedence relationships may be discovered.  When such a cycle is detected, all bounds in the
   cycle are placed into a new CycleGroup that contains all bounds in the cycle.  Since each
   ConstrainedBound begins in its own CycleGroup, and the formation of a cycle merges all involved
   bounds into a new CycleGroup, precedence links between CycleGroups never form a cycle.  We use
   CycleGroups to determine the order in which bounds should be calculated (we first calculate
   bounds that belong to CycleGroups that have no predecessors, then calculate bounds in the
   immediate successors of *those* CycleGroups, etc.).

   \see Bound
   \see BoundCt
   \see ConstrainedBound
   \see CycleGroup
   \ingroup clp
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class BoundPropagator : public utl::Object
{
    UTL_CLASS_DECL(BoundPropagator, utl::Object);
    UTL_CLASS_NO_COPY;

public:
    /** Constructor. */
    BoundPropagator(Manager* mgr)
    {
        init(mgr);
    }

    /// \name Precedence relationships
    //@{
    /**
       Add a BoundCt that allows a source bound to push a destination bound.
       \param src source bound
       \param dst destination bound
       \param delta extra distance between source and destination bounds
       \param cycleCheck check for cycles?
    */
    void addBoundCt(ConstrainedBound* src, ConstrainedBound* dst, int delta, bool cycleCheck = true);

    /** Add a new precedence relationship between two cycle-groups. */
    void addPrecedenceLink(CycleGroup* src, CycleGroup* dst, bool cycleCheck = true);

    /** Perform static analysis (transitive closure). */
    void staticAnalysis();

    /** Create a new CycleGroup and return it. */
    CycleGroup* newCycleGroup(CycleGroup* cg = nullptr);
    //@}

    /// \name Propagation
    //@{
    /** Propagate. */
    void propagate();

    /** Clear the propagation queue. */
    void clearPropQ();

    /** Put the given bound into the propagation queue. */
#ifdef DEBUG
    void enQ(ConstrainedBound* bound);
#else
    void
    enQ(ConstrainedBound* bound)
    {
        _propQ.push_back(bound);
    }
#endif

    /** Unsuspend CGs that have no predecessors. */
    void unsuspendInitial();

    /** Unsuspend a bound. */
    virtual void unsuspend(ConstrainedBound* cb);

    /** Finalize a bound. */
    virtual void finalize(ConstrainedBound* cb);
    //@}
protected:
    Manager* _mgr;

private:
    using bct_array_t = RevArray<BoundCt*>;

private:
    void
    init()
    {
        ABORT();
    }
    void init(Manager* mgr);
    void deInit();

    // depth-first search (DFS)
    void dfs(CycleGroup* src, CycleGroup* dst);
    void dfs(CycleGroup* cg);
    void dfs_push(CycleGroup* cg);
    void dfs_pop();

    // make a cycle-group
    void makeCycleGroup();

    // set successor depth for all CGs
    void setSuccessorDepth();
    void setSuccessorDepth(CycleGroup* cg);

private:
#ifdef DEBUG
    void sanityCheckCGs();
    void sanityCheckCG(CycleGroup* cg);
#endif

    // for propagation
    ConstrainedBound* _boundInProcess;
    std::deque<ConstrainedBound*> _propQ;
    CycleGroup* _restartCG;
    cg_revset_t _cgs;

    // cycle-groups array
    CycleGroup** _cgArray;
    CycleGroup** _cgArrayPtr;
    CycleGroup** _cgArrayLim;
    size_t _cgArraySize;

    uint_t _initCgId;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_END;
