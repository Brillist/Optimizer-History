#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

class Goal;
class Manager;
class Or;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Choice point.

   A choice point records the state of all managed data, allowing that state to be restored
   so that different choices can be made to find a different solution.

   \see Manager
   \see Manager::pushState
   \see Manager::popState
   \ingroup clp
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class ChoicePoint
{
public:
    using goal_stack_t = std::stack<Goal*>;

public:
    /** Constructor. */
    ChoicePoint()
    {
        init();
    }

    /**
       Constructor.
       \param orGoal Or goal associated with this ChoicePoint
       \param goalStack goal stack at time of ChoicePoint's creation (restored when backtracking)
    */
    ChoicePoint(Or* orGoal, const goal_stack_t& goalStack)
    {
        init();
        set(orGoal, goalStack);
    }

    /** Destructor. */
    virtual ~ChoicePoint()
    {
        deInit();
    }

    /** Reset to default state. */
    void clear();

    /**
       (Re-)initialize.
       \param orGoal Or goal associated with this ChoicePoint
       \param goalStack goal stack at time of ChoicePoint's creation (restored when backtracking)
    */
    void set(Or* orGoal, const goal_stack_t& goalStack);

    /** Has a remaining choice? */
    bool hasRemainingChoice() const;

    /** Get the next choice and increment the index. */
    Goal* nextChoice();

    /** Backtrack. */
    void backtrack(goal_stack_t& goalStack);

    /** This is the root choice point? */
    bool
    isRoot() const
    {
        return (_or == nullptr);
    }

    /// \name Accessors (const)
    //@{
    /** Get the manager. */
    Manager*
    manager() const
    {
        return _mgr;
    }

    /** Get the or goal. */
    Or*
    orGoal() const
    {
        return _or;
    }

    /** Get the or-index. */
    uint_t
    orIdx() const
    {
        return _orIdx;
    }

    /** Has a label? */
    bool hasLabel() const;

    /** Get the label. */
    uint_t label() const;

    /** Get rev-longs index. */
    uint_t
    revLongsIdx() const
    {
        return _revLongsIdx;
    }

    /** Get rev-long-arrays index. */
    uint_t
    revLongArraysIdx() const
    {
        return _revLongArraysIdx;
    }

    /** Get rev-longs indirect index. */
    uint_t
    revLongsIndIdx() const
    {
        return _revLongsIndIdx;
    }

    /** Get rev-long-arrays indirect index. */
    uint_t
    revLongArraysIndIdx() const
    {
        return _revLongArraysIndIdx;
    }

    /** Get rev-ints index. */
    uint_t
    revIntsIdx() const
    {
        return _revIntsIdx;
    }

    /** Get rev-arrays index. */
    uint_t
    revIntArraysIdx() const
    {
        return _revIntArraysIdx;
    }
    /** Get rev-ints indirect index. */
    uint_t
    revIntsIndIdx() const
    {
        return _revIntsIndIdx;
    }

    /** Get rev-arrays indirect index. */
    uint_t
    revIntArraysIndIdx() const
    {
        return _revIntArraysIndIdx;
    }

    /** Get rev-delta-vars index. */
    uint_t
    revDeltaVarsIdx() const
    {
        return _revDeltaVarsIdx;
    }

    /** Get rev-cts index. */
    uint_t
    revCtsIdx() const
    {
        return _revCtsIdx;
    }

    /** Get rev-toggles index. */
    uint_t
    revTogglesIdx() const
    {
        return _revTogglesIdx;
    }

    /** Get rev-allocations index. */
    uint_t
    revAllocationsIdx() const
    {
        return _revAllocationsIdx;
    }

    /** Get rev-actions index. */
    uint_t
    revActionsIdx() const
    {
        return _revActionsIdx;
    }
    //@}

    /// \name Accessors (non-const)
    //@{
    /** Set the manager. */
    void
        setManager(Manager* mgr)
    {
        _mgr = mgr;
    }

    /** Set rev-longs index. */
    void
    setRevLongsIdx(uint_t revLongsIdx)
    {
        _revLongsIdx = revLongsIdx;
    }

    /** Set rev-long-arrays index. */
    void
    setRevLongArraysIdx(uint_t revLongArraysIdx)
    {
        _revLongArraysIdx = revLongArraysIdx;
    }

    /** Set rev-longs indirect index. */
    void
    setRevLongsIndIdx(uint_t revLongsIndIdx)
    {
        _revLongsIndIdx = revLongsIndIdx;
    }

    /** Set rev-long-arrays indirect index. */
    void
    setRevLongArraysIndIdx(uint_t revLongArraysIndIdx)
    {
        _revLongArraysIndIdx = revLongArraysIndIdx;
    }

    /** Set rev-ints index. */
    void
    setRevIntsIdx(uint_t revIntsIdx)
    {
        _revIntsIdx = revIntsIdx;
    }

    /** Set rev-arrays index. */
    void
    setRevIntArraysIdx(uint_t revIntArraysIdx)
    {
        _revIntArraysIdx = revIntArraysIdx;
    }

    /** Set rev-ints indirect index. */
    void
    setRevIntsIndIdx(uint_t revIntsIndIdx)
    {
        _revIntsIndIdx = revIntsIndIdx;
    }

    /** Set rev-arrays indirect index. */
    void
    setRevIntArraysIndIdx(uint_t revIntArraysIndIdx)
    {
        _revIntArraysIndIdx = revIntArraysIndIdx;
    }

    /** Set rev-delta-vars index. */
    void
    setRevDeltaVarsIdx(uint_t revDeltaVarsIdx)
    {
        _revDeltaVarsIdx = revDeltaVarsIdx;
    }

    /** Set rev-cts index. */
    void
    setRevCtsIdx(uint_t revCtsIdx)
    {
        _revCtsIdx = revCtsIdx;
    }

    /** Set rev-toggles index. */
    void
    setRevTogglesIdx(uint_t revTogglesIdx)
    {
        _revTogglesIdx = revTogglesIdx;
    }

    /** Set rev-allocations index. */
    void
    setRevAllocationsIdx(uint_t revAllocationsIdx)
    {
        _revAllocationsIdx = revAllocationsIdx;
    }

    /** Set rev-actions index. */
    void
    setRevActionsIdx(uint_t revActionsIdx)
    {
        _revActionsIdx = revActionsIdx;
    }
    //@}

private:
    void init();
    void deInit();
    void saveGoalStack(const goal_stack_t& goalStack);
    void restoreGoalStack(goal_stack_t& goalStack);

private:
    Manager* _mgr;
    Or* _or;
    goal_vector_t _goals;
    uint_t _orIdx;
    uint_t _revLongsIdx;
    uint_t _revLongArraysIdx;
    uint_t _revLongsIndIdx;
    uint_t _revLongArraysIndIdx;
    uint_t _revIntsIdx;
    uint_t _revIntArraysIdx;
    uint_t _revIntsIndIdx;
    uint_t _revIntArraysIndIdx;
    uint_t _revDeltaVarsIdx;
    uint_t _revCtsIdx;
    uint_t _revTogglesIdx;
    uint_t _revAllocationsIdx;
    uint_t _revActionsIdx;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_END;
