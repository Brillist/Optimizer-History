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

   \author Adam McKee
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

    /** Add a new bound-ct. */
    void addBoundCt(ConstrainedBound* src, ConstrainedBound* dst, int d, bool cycleCheck = true);

    /** Add a new precedence relationship between two cycle-groups. */
    void addPrecedenceLink(CycleGroup* src, CycleGroup* dst, bool cycleCheck = true);

    /** Perform static analysis (transitive closure). */
    void staticAnalysis();

    /** Unsuspend CGs that have no predecessors. */
    void unsuspendInitial();

    /** Unsuspend a constrained-bound. */
    virtual void unsuspend(ConstrainedBound* cb);

    /** Finalize a constrained-bound. */
    virtual void finalize(ConstrainedBound* cb);

    /** Create a new CycleGroup and return it. */
    CycleGroup* newCycleGroup(CycleGroup* cg = nullptr);

    /// \name Propagation.
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
    //@}
protected:
    Manager* _mgr;

private:
    /*     typedef std::set<CycleGroup*> cg_set_t; */
    /*     typedef clp::RevSet<CycleGroup> cg_revset_t; */
    typedef RevArray<BoundCt*> bct_array_t;

private:
    void
    init()
    {
        ABORT();
    }
    void init(Manager* mgr);
    void deInit();

    // for depth-first search (DFS)
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
