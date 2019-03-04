#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <libutl/Object.h>
#include <clp/ConstrainedBound.h>
#include <clp/RevSet.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Precedence relationship.
   \ingroup clp
*/
enum cg_precedence_rel_t
{
    pr_precedes, /**< lhs precedes rhs */
    pr_succeeds, /**< lhs succeeds rhs */
    pr_none      /**< no precedence relationship */
};

////////////////////////////////////////////////////////////////////////////////////////////////////

class CycleGroup;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Order CycleGroup objects by their id.
   \see CycleGroup::id
   \ingroup clp
*/
struct CycleGroupIdOrdering
{
    bool operator()(const CycleGroup* lhs, const CycleGroup* rhs) const;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

using cg_revset_t = RevSet<CycleGroup>;
using cg_set_id_t = std::set<CycleGroup*, CycleGroupIdOrdering>;
using cg_vector_t = std::vector<CycleGroup*>;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   A set of ConstrainedBound objects that form a cycle.

   All ConstrainedBound objects in a CycleGroup are reachable from all others in the group
   by following bound constraints (BoundCt).

   \ingroup clp
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class CycleGroup : public utl::Object
{
    UTL_CLASS_DECL(CycleGroup, utl::Object);
    UTL_CLASS_NO_COPY;

public:
    using cb_revset_t = RevSet<ConstrainedBound>;
    using iterator = cb_revset_t::iterator;

public:
    /** Constructor. */
    CycleGroup(Manager* mgr);

    virtual int compare(const utl::Object& rhs) const;

    virtual String toString() const;

    /** Add a bound. */
    void add(ConstrainedBound* cb);

    /** Eclipse the given CG. */
    void eclipse(CycleGroup* cg);

    /// \name Accessors (const)
    //@{
    /** Get the id. */
    uint_t
    id() const
    {
        return _id;
    }

    /** Get visited flag. */
    bool
    visited() const
    {
        return (_visitedIdx != uint_t_max);
    }

    /** Group is finalized iff all member bounds are finalized. */
    bool
    finalized() const
    {
        return (_numUnfinalizedCBs == 0);
    }

    /** Propagation suspended? */
    bool
    suspended() const
    {
        return (_numUnfinalizedPredCGs > 0);
    }

    /** Get visited index. */
    uint_t
    visitedIdx() const
    {
        return _visitedIdx;
    }

    /** Get size. */
    uint_t
    size() const
    {
        return _cbs.size();
    }

    /** Get immediate predecessor cycle-groups. */
    const cg_revset_t&
    predCGs() const
    {
        return _predCGs;
    }

    /** Get immediate successor cycle-groups. */
    const cg_revset_t&
    succCGs() const
    {
        return _succCGs;
    }

    /** Get all predecessor cycle-groups (transitive closure). */
    const cg_revset_t&
    allPredCGs() const
    {
        return _allPredCGs;
    }

    /** Get all successor cycle-groups (transitive closure). */
    const cg_revset_t&
    allSuccCGs() const
    {
        return _allSuccCGs;
    }

    /** Get the successor-count. */
    uint_t
    successorCount() const
    {
        return _successorCount;
    }

    /** Get the successor-depth. */
    uint_t
    successorDepth() const
    {
        return _successorDepth;
    }

    /** Get begin iterator for member bounds. */
    iterator
    begin() const
    {
        return _cbs.begin();
    }

    /** Get end iterator for member bounds. */
    iterator
    end() const
    {
        return _cbs.end();
    }
    //@}

    /// \name Accessors (non-const)
    //@{
    /** Set the id. */
    void
    setId(uint_t id)
    {
        _id = id;
    }

    /** Set visited-index. */
    void
    setVisitedIdx(uint_t visitedIdx)
    {
        _visitedIdx = visitedIdx;
    }

    /** Set successor-count. */
    uint_t
    setSuccessorCount(uint_t successorCount)
    {
        _successorCount = successorCount;
        return _successorCount;
    }

    /** Set successor-depth. */
    uint_t
    setSuccessorDepth(uint_t successorDepth)
    {
        _successorDepth = successorDepth;
        return _successorDepth;
    }
    //@}

    /// \name Precedence relationships (const)
    //@{
    /** Get the precedence relationship between self and cg. */
    cg_precedence_rel_t relationship(const CycleGroup* cg) const;

    /** Does self precede the given cycle-group? */
    bool precedes(const CycleGroup* cg) const;

    /** Does self succeed the given cycle-group? */
    bool succeeds(const CycleGroup* cg) const;
    //@}

    /// \name Precedence relationships (non-const)
    //@{
    /** Add a precedessor CG. */
    bool addPred(CycleGroup* cg, bool updateIndirect = true);

    /** Remove a predecessor CG. */
    void removePred(CycleGroup* cg);

    /** Clear indirect pred/succ lists. */
    void clearIndirectLists();

    /** Initialize indirect pred/succ lists. */
    void initIndirectLists();
    //@}

    /// \name Propagation
    //@{
    /** Enable propagation of member bounds. */
    virtual void unsuspend();

    /** Finalize self. */
    void finalize();

    /** Invoked after finalizing a member bound. */
    void finalizeMember();

    /** Finalize a predecessor bound. */
    void finalizePred();
    //@}

#ifdef DEBUG
    virtual String predCGsString() const;
    virtual String allPredCGsString() const;
    virtual String succCGsString() const;
    virtual String allSuccCGsString() const;
#endif

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

    void
    saveState()
    {
        uint_t d = _mgr->depth();
        if (d <= _stateDepth)
            return;
        _saveState();
    }

    void _saveState();

private:
    Manager* _mgr;
    BoundPropagator* _bp;
    uint_t _id;
    uint_t _visitedIdx;
    uint_t _successorCount;
    uint_t _successorDepth;

    // reversible /////////////////////////////////////////////
    cb_revset_t _cbs;
    cg_revset_t _predCGs;
    cg_revset_t _succCGs;
    cg_revset_t _allPredCGs;
    cg_revset_t _allSuccCGs;
    uint_t _stateDepth;
    uint_t _numUnfinalizedCBs;
    uint_t _numUnfinalizedPredCGs;
    ///////////////////////////////////////////////////////////
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_END;
