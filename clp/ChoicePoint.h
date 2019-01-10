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

   In a solution search, a choice point corresponds to a logical "Or" of
   goals.  At a choice point, the search state (including domains of all
   constrained variables) is preserved, then one of the goals is tried
   (pushed onto the goal stack).  If the execution of an Or-related goal
   fails, the search state is recovered from the choice point, and the
   next Or-related goal (if any) is tried.  If no remaining goals are left,
   then failure continues down the goal stack until another choice point
   (Or goal) is encountered.

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

#if UTL_HOST_WORDSIZE == 64
    /** Get rev-longs index. */
    utl::uint_t
    getRevLongsIdx() const
    {
        return _revLongsIdx;
    }

    /** Set rev-longs index. */
    void
    setRevLongsIdx(utl::uint_t revLongsIdx)
    {
        _revLongsIdx = revLongsIdx;
    }

    /** Get rev-long-arrays index. */
    utl::uint_t
    getRevLongArraysIdx() const
    {
        return _revLongArraysIdx;
    }

    /** Set rev-long-arrays index. */
    void
    setRevLongArraysIdx(utl::uint_t revLongArraysIdx)
    {
        _revLongArraysIdx = revLongArraysIdx;
    }

    /** Get rev-longs indirect index. */
    utl::uint_t
    getRevLongsIndIdx() const
    {
        return _revLongsIndIdx;
    }

    /** Set rev-longs indirect index. */
    void
    setRevLongsIndIdx(utl::uint_t revLongsIndIdx)
    {
        _revLongsIndIdx = revLongsIndIdx;
    }

    /** Get rev-long-arrays indirect index. */
    utl::uint_t
    getRevLongArraysIndIdx() const
    {
        return _revLongArraysIndIdx;
    }

    /** Set rev-long-arrays indirect index. */
    void
    setRevLongArraysIndIdx(utl::uint_t revLongArraysIndIdx)
    {
        _revLongArraysIndIdx = revLongArraysIndIdx;
    }
#endif

    /** Get rev-ints index. */
    utl::uint_t
    getRevIntsIdx() const
    {
        return _revIntsIdx;
    }

    /** Set rev-ints index. */
    void
    setRevIntsIdx(utl::uint_t revIntsIdx)
    {
        _revIntsIdx = revIntsIdx;
    }

    /** Get rev-arrays index. */
    utl::uint_t
    getRevIntArraysIdx() const
    {
        return _revIntArraysIdx;
    }

    /** Set rev-arrays index. */
    void
    setRevIntArraysIdx(utl::uint_t revIntArraysIdx)
    {
        _revIntArraysIdx = revIntArraysIdx;
    }

    /** Get rev-ints indirect index. */
    utl::uint_t
    getRevIntsIndIdx() const
    {
        return _revIntsIndIdx;
    }

    /** Set rev-ints indirect index. */
    void
    setRevIntsIndIdx(utl::uint_t revIntsIndIdx)
    {
        _revIntsIndIdx = revIntsIndIdx;
    }

    /** Get rev-arrays indirect index. */
    utl::uint_t
    getRevIntArraysIndIdx() const
    {
        return _revIntArraysIndIdx;
    }

    /** Set rev-arrays indirect index. */
    void
    setRevIntArraysIndIdx(utl::uint_t revIntArraysIndIdx)
    {
        _revIntArraysIndIdx = revIntArraysIndIdx;
    }

    /** Get rev-delta-vars index. */
    utl::uint_t
    getRevDeltaVarsIdx() const
    {
        return _revDeltaVarsIdx;
    }

    /** Set rev-delta-vars index. */
    void
    setRevDeltaVarsIdx(utl::uint_t revDeltaVarsIdx)
    {
        _revDeltaVarsIdx = revDeltaVarsIdx;
    }

    /** Get rev-cts index. */
    utl::uint_t
    getRevCtsIdx() const
    {
        return _revCtsIdx;
    }

    /** Set rev-cts index. */
    void
    setRevCtsIdx(utl::uint_t revCtsIdx)
    {
        _revCtsIdx = revCtsIdx;
    }

    /** Get rev-toggles index. */
    utl::uint_t
    getRevTogglesIdx() const
    {
        return _revTogglesIdx;
    }

    /** Set rev-toggles index. */
    void
    setRevTogglesIdx(utl::uint_t revTogglesIdx)
    {
        _revTogglesIdx = revTogglesIdx;
    }

    /** Get rev-allocations index. */
    utl::uint_t
    getRevAllocationsIdx() const
    {
        return _revAllocationsIdx;
    }

    /** Set rev-allocations index. */
    void
    setRevAllocationsIdx(utl::uint_t revAllocationsIdx)
    {
        _revAllocationsIdx = revAllocationsIdx;
    }

    /** Get rev-actions index. */
    utl::uint_t
    getRevActionsIdx() const
    {
        return _revActionsIdx;
    }

    /** Set rev-actions index. */
    void
    setRevActionsIdx(utl::uint_t revActionsIdx)
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
    utl::uint_t
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
    utl::uint_t label() const;

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
    utl::uint_t _orIdx;
#if UTL_HOST_WORDSIZE == 64
    utl::uint_t _revLongsIdx;
    utl::uint_t _revLongArraysIdx;
    utl::uint_t _revLongsIndIdx;
    utl::uint_t _revLongArraysIndIdx;
#endif
    utl::uint_t _revIntsIdx;
    utl::uint_t _revIntArraysIdx;
    utl::uint_t _revIntsIndIdx;
    utl::uint_t _revIntArraysIndIdx;
    utl::uint_t _revDeltaVarsIdx;
    utl::uint_t _revCtsIdx;
    utl::uint_t _revTogglesIdx;
    utl::uint_t _revAllocationsIdx;
    utl::uint_t _revActionsIdx;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_END;

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
