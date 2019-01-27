#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <libutl/Object.h>
#include <clp/ConstrainedBound.h>
#include <clp/RevSet.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/** Precedence relationship. */
enum cg_precedence_rel_t
{
    pr_precedes, /**< lhs precedes rhs */
    pr_succeeds, /**< lhs succeeds rhs */
    pr_none,     /**< no precedence relationship */
    pr_undefined /**< undefined/nullptr */
};

////////////////////////////////////////////////////////////////////////////////////////////////////

class CycleGroup;

////////////////////////////////////////////////////////////////////////////////////////////////////

struct CycleGroupIdOrdering : public std::binary_function<CycleGroup*, CycleGroup*, bool>
{
    bool operator()(const CycleGroup* lhs, const CycleGroup* rhs) const;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

typedef std::set<CycleGroup*, CycleGroupIdOrdering> cg_set_id_t;
typedef RevSet<CycleGroup> cg_revset_t;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   A set of ConstrainedBound objects s.t. any CB object in the set is reachable from any other
   (by following BoundCts).

   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class CycleGroup : public utl::Object
{
    UTL_CLASS_DECL(CycleGroup, utl::Object);
    UTL_CLASS_NO_COPY;

public:
    typedef RevSet<ConstrainedBound> cb_revset_t;
    typedef cb_revset_t::iterator iterator;
    typedef cg_revset_t::iterator cg_iterator;

public:
    /** Constructor. */
    CycleGroup(Manager* mgr);

    virtual int compare(const utl::Object& rhs) const;

    /** Add a bound. */
    void add(ConstrainedBound* cb);

    /** Eclipse the given CG. */
    void eclipse(CycleGroup* cg);

    /** Add a precedessor CG. */
    bool addPred(CycleGroup* cg, bool updateIndirect = true);

    /** Remove a predecessor CG. */
    void removePred(CycleGroup* cg);

    /** Clear indirect pred/succ lists. */
    void clearIndirectLists();

    /** Initialize indirect pred/succ lists. */
    void initIndirectLists();

    /** Remove an indirect predecessor. */
    void removeIndirectPred(CycleGroup* cg);

    /** Group is finalized iff all member bounds are finalized */
    bool
    finalized() const
    {
        return (_numUnfinalizedCBs == 0);
    }

    /** Enable propagation of member bounds. */
    virtual void unsuspend();

    /** Finalize self. */
    void finalize();

    /** Finalize the given member bound. */
    void finalizeMember();

    /** Finalize a predecessor bound. */
    void finalizePred();

    /** Get the precedence relationship between self and cg. */
    cg_precedence_rel_t relationship(const CycleGroup* cg) const;

    /** Does self precede the given cycle-group? */
    bool precedes(const CycleGroup* cg) const;

    /** Does self succeed the given cycle-group? */
    bool succeeds(const CycleGroup* cg) const;

    /// \name General Accessors
    //@{
    /** Get the id. */
    uint_t
    id() const
    {
        return _id;
    }

    /** Get the id. */
    uint_t&
    id()
    {
        return _id;
    }

    /** Get visited flag. */
    bool
    visited() const
    {
        return (_visitedIdx != uint_t_max);
    }

    /** Get visited index. */
    uint_t
    visitedIdx() const
    {
        return _visitedIdx;
    }

    /** Get visited count. */
    uint_t&
    visitedIdx()
    {
        return _visitedIdx;
    }

    /** Propagation suspended? */
    bool
    suspended()
    {
        return (_numUnfinalizedPredCGs > 0);
    }

    /** Get size. */
    uint_t
    size() const
    {
        return _cbs.size();
    }

    /** Constrained-Bounds begin iterator. */
    iterator
    begin() const
    {
        return _cbs.begin();
    }

    /** Constrained-Bounds end iterator. */
    iterator
    end() const
    {
        return _cbs.end();
    }

    /** Predecessor cycle-groups. */
    const cg_revset_t&
    predCGs() const
    {
        return _predCGs;
    }

    /** Successor cycle-groups. */
    const cg_revset_t&
    succCGs() const
    {
        return _succCGs;
    }

    /** All predecessor cycle-groups (transitive closure). */
    const cg_revset_t&
    allPredCGs() const
    {
        return _allPredCGs;
    }

    /** All successor cycle-groups (transitive closure). */
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
    uint_t&
    successorCount()
    {
        return _successorCount;
    }

    /** Get the successor-depth. */
    uint_t
    successorDepth() const
    {
        return _successorDepth;
    }

    /** Get the successor-depth. */
    uint_t&
    successorDepth()
    {
        return _successorDepth;
    }
    //@}

    virtual String toString() const;
    virtual String predCGsString() const;
    virtual String allPredCGsString() const;
    virtual String succCGsString() const;
    virtual String allSuccCGsString() const;

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
