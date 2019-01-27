#ifndef CLP_CHOICEPOINT_H
#define CLP_CHOICEPOINT_H

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

class Goal;
class Manager;
class Or;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Choice point.

   In a solution search, a choice point corresponds to a logical "Or" of goals.  At a choice point,
   the search state (including domains of all constrained variables) is preserved, then one of the
   goals is tried (pushed onto the goal stack).  If the execution of an Or-related goal fails, the
   search state is recovered from the choice point, and the next Or-related goal (if any) is tried.
   If no remaining goals are left, then failure continues down the goal stack until another choice
   point (Or goal) is encountered.

   \see Manager
   \see Or
   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class ChoicePoint
{
public:
    typedef std::stack<Goal*> goal_stack_t;

public:
    /** Constructor. */
    ChoicePoint()
    {
        init();
    }

    /** Constructor. */
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

    /** Re-initialize. */
    void clear();

    /** Set attributes. */
    void set(Or* orGoal, const goal_stack_t& goalStack);

    /** Get rev-longs index. */
    uint_t
    getRevLongsIdx() const
    {
        return _revLongsIdx;
    }

    /** Set rev-longs index. */
    void
    setRevLongsIdx(uint_t revLongsIdx)
    {
        _revLongsIdx = revLongsIdx;
    }

    /** Get rev-long-arrays index. */
    uint_t
    getRevLongArraysIdx() const
    {
        return _revLongArraysIdx;
    }

    /** Set rev-long-arrays index. */
    void
    setRevLongArraysIdx(uint_t revLongArraysIdx)
    {
        _revLongArraysIdx = revLongArraysIdx;
    }

    /** Get rev-longs indirect index. */
    uint_t
    getRevLongsIndIdx() const
    {
        return _revLongsIndIdx;
    }

    /** Set rev-longs indirect index. */
    void
    setRevLongsIndIdx(uint_t revLongsIndIdx)
    {
        _revLongsIndIdx = revLongsIndIdx;
    }

    /** Get rev-long-arrays indirect index. */
    uint_t
    getRevLongArraysIndIdx() const
    {
        return _revLongArraysIndIdx;
    }

    /** Set rev-long-arrays indirect index. */
    void
    setRevLongArraysIndIdx(uint_t revLongArraysIndIdx)
    {
        _revLongArraysIndIdx = revLongArraysIndIdx;
    }

    /** Get rev-ints index. */
    uint_t
    getRevIntsIdx() const
    {
        return _revIntsIdx;
    }

    /** Set rev-ints index. */
    void
    setRevIntsIdx(uint_t revIntsIdx)
    {
        _revIntsIdx = revIntsIdx;
    }

    /** Get rev-arrays index. */
    uint_t
    getRevIntArraysIdx() const
    {
        return _revIntArraysIdx;
    }

    /** Set rev-arrays index. */
    void
    setRevIntArraysIdx(uint_t revIntArraysIdx)
    {
        _revIntArraysIdx = revIntArraysIdx;
    }

    /** Get rev-ints indirect index. */
    uint_t
    getRevIntsIndIdx() const
    {
        return _revIntsIndIdx;
    }

    /** Set rev-ints indirect index. */
    void
    setRevIntsIndIdx(uint_t revIntsIndIdx)
    {
        _revIntsIndIdx = revIntsIndIdx;
    }

    /** Get rev-arrays indirect index. */
    uint_t
    getRevIntArraysIndIdx() const
    {
        return _revIntArraysIndIdx;
    }

    /** Set rev-arrays indirect index. */
    void
    setRevIntArraysIndIdx(uint_t revIntArraysIndIdx)
    {
        _revIntArraysIndIdx = revIntArraysIndIdx;
    }

    /** Get rev-delta-vars index. */
    uint_t
    getRevDeltaVarsIdx() const
    {
        return _revDeltaVarsIdx;
    }

    /** Set rev-delta-vars index. */
    void
    setRevDeltaVarsIdx(uint_t revDeltaVarsIdx)
    {
        _revDeltaVarsIdx = revDeltaVarsIdx;
    }

    /** Get rev-cts index. */
    uint_t
    getRevCtsIdx() const
    {
        return _revCtsIdx;
    }

    /** Set rev-cts index. */
    void
    setRevCtsIdx(uint_t revCtsIdx)
    {
        _revCtsIdx = revCtsIdx;
    }

    /** Get rev-toggles index. */
    uint_t
    getRevTogglesIdx() const
    {
        return _revTogglesIdx;
    }

    /** Set rev-toggles index. */
    void
    setRevTogglesIdx(uint_t revTogglesIdx)
    {
        _revTogglesIdx = revTogglesIdx;
    }

    /** Get rev-allocations index. */
    uint_t
    getRevAllocationsIdx() const
    {
        return _revAllocationsIdx;
    }

    /** Set rev-allocations index. */
    void
    setRevAllocationsIdx(uint_t revAllocationsIdx)
    {
        _revAllocationsIdx = revAllocationsIdx;
    }

    /** Get rev-actions index. */
    uint_t
    getRevActionsIdx() const
    {
        return _revActionsIdx;
    }

    /** Set rev-actions index. */
    void
    setRevActionsIdx(uint_t revActionsIdx)
    {
        _revActionsIdx = revActionsIdx;
    }

    /** Get the manager. */
    Manager*
    manager() const
    {
        return _mgr;
    }

    /** Set the manager. */
    void
    setManager(Manager* mgr)
    {
        _mgr = mgr;
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

    /** Is the root choice point? */
    bool
    isRoot() const
    {
        return (_or == nullptr);
    }

    /** Has a remaining choice? */
    bool hasRemainingChoice() const;

    /** Get the next choice and increment the index. */
    Goal* getNextChoice();

    /** Has a label? */
    bool isLabeled() const;

    /** Get the label. */
    uint_t label() const;

    /** Backtrack. */
    void backtrack(goal_stack_t& goalStack);

private:
    typedef std::vector<Goal*> goal_vector_t;

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

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
