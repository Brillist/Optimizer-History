#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <lut/Functor.h>
#include <lut/SkipListDepthArray.h>
#include <clp/Constraint.h>
#include <clp/FailEx.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

class Bound;
class ChoicePoint;
class ConstrainedBound;
class ConstrainedVar;
class BoundPropagator;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Management of constraints, constrained variables, and the solution search.

   ## Constructing solutions (and backtracking to continue searching)

   Constructing a solution is a process of making choices until each constrained variable has been
   bound (without violating any constraint).  The solution search is a process of experimenting
   with different possibilities to find the best one(s).  To experiment with different choices we
   require the ability to *backtrack* the search state.  With backtracking support, we can place a
   marker for the current state (a ChoicePoint), then make choices (changing the state accordingly)
   leading to either the successful construction of a solution, or failure/inconsistency.  Whether
   our choices end in success or failure, we can backtrack to the previous state and try something
   different.

   ## The goal stack

   The execution of a goal makes a decision (or multiple decisions) that lead toward a solution.
   A goal can have sub-goals, so Manager uses a stack to control Goal execution.

   Looking at the implementation of \ref nextSolution, we find code that looks like this:

   \code
   while (!_goalStack.empty())
   {
       // pop a goal from the stack
       auto goal = _goalStack.top();
       _goalStack.pop();

       // execute the goal and propagate
       goal->execute();
       propagate();

       // remove our reference
       goal->removeRef();
   }
   \endcode

   That's a bit simplified because it ignores the special handling of Or goals, but it shows
   how "normal" goal execution is controlled.  A goal can push other goals onto the stack (by
   calling \ref add), and Manager will execute those (in LIFO/stack order), continuing that loop
   until the goal stack is empty.
   
   \see BoundPropagator
   \see ChoicePoint
   \see Goal
   \ingroup clp
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class Manager : public utl::Object
{
    UTL_CLASS_DECL(Manager, utl::Object);
    UTL_CLASS_NO_COPY;

public:
    using ct_set_t = std::unordered_set<Constraint*, lut::HashUint<Constraint*>>;
    using ct_const_iterator = ct_set_t::const_iterator;

public:
    /// \name Solution search
    //@{
    /** Push a goal onto the goal stack. */
    void add(const Goal& goal);

    /** Push a goal onto the goal stack. */
    void add(Goal* goal);

    /** Reset completely. */
    void reset();

    /** Restart the search. */
    void restart();

    /** Find the next solution. */
    bool nextSolution();

    /**
       Propagate.
       \see BoundPropagator::propagate
    */
    void propagate();

    /** Push a ChoicePoint onto the stack to enable backtracking to the current search state. */
    void pushState();

    /** Backtrack to the ChoicePoint created by the most recent call to \ref pushState. */
    void popState();
    //@}

    /// \name Constraints
    //@{
    /** Add a constraint. */
    void add(const Constraint& ct);

    /** Add a constraint. */
    bool add(Constraint* ct);

    /** Remove a constraint. */
    void remove(Constraint* ct);
    //@}

    /// \name Constrained variables
    //@{
    /** Add a constrained variable. */
    void add(ConstrainedVar* var);

    /** Remove the given constrained variable. */
    void remove(ConstrainedVar* var);
    //@}

    /// \name Accessors (const)
    //@{
    /** Get the current depth. */
    uint_t
    depth() const
    {
        return _cpStackSize;
    }

    /** Get the skip-list delta array. */
    const lut::SkipListDepthArray*
    skipListDepthArray() const
    {
        return _skipListDepthArray;
    }

    /** Get the bound propagator. */
    BoundPropagator*
    boundPropagator() const
    {
        return _boundPropagator;
    }

    /** Get constraints begin iterator. */
    ct_set_t::const_iterator
    ctsBegin() const
    {
        return _cts.begin();
    }

    /** Get constraints end iterator. */
    ct_set_t::const_iterator
    ctsEnd() const
    {
        return _cts.end();
    }
    //@}

    /// \name Accessors (non-const)
    //@{
    /** Set the bound propagator. */
    void setBoundPropagator(BoundPropagator* bp);
    //@}

    /// \name Reversible Actions
    //@{
    /** Reversibly increment the given counter. */
    void
    revIncrement(int& counter, int num = 1)
    {
        revIncrement((uint_t&)counter, (uint_t)num);
    }

    /** Reversibly decrement the given counter. */
    void
    revDecrement(int& counter, int num = 1)
    {
        revDecrement((uint_t&)counter, (uint_t)num);
    }

    /** Reversibly increment the given counter. */
    void
    revIncrement(uint_t& counter, uint_t num = 1)
    {
        ASSERTD(!_cpStack.empty() && (_topCP == _cpStack.top()));
        revSet(counter);
        counter += num;
    }

    /** Reversibly decrement the given counter. */
    void
    revDecrement(uint_t& counter, uint_t num = 1)
    {
        ASSERTD(!_cpStack.empty() && (_topCP == _cpStack.top()));
        revSet(counter);
        counter -= num;
    }

    /** Reversibly set the given integer. */
    template <class T>
    void
    revSet(T& i, T v)
    {
        revSet(i);
        i = v;
    }

    /** Reversibly set the given integer. */
    template <class T>
    void
    revSet(T& i)
    {
        if (sizeof(T) == 8)
        {
            revSetLong((size_t&)i);
            return;
        }
        ASSERTD(sizeof(T) == 4);
        revSetInt((uint_t&)i);
    }

    /** Reversibly set the given array. */
    template <class T>
    void
    revSet(T* array, uint_t size)
    {
        if (sizeof(T) == 8)
        {
            revSetLongArray((size_t*)array, size);
            return;
        }
        ASSERTD(sizeof(T) == 4);
        revSetIntArray((uint_t*)array, size);
    }

    /** Reversibly set the given array element (indirectly). */
    template <class T>
    void
    revSetIndirect(T*& array, uint_t idx)
    {
        if (sizeof(T) == 8)
        {
            revSetLongInd((size_t*&)array, idx);
            return;
        }
        ASSERTD(sizeof(T) == 4);
        revSetIntInd((uint_t*&)array, idx);
    }

    /** Reversibly set the given array (indirectly). */
    template <class T>
    void
    revSetIndirect(T*& array, uint_t idx, uint_t size)
    {
        if (sizeof(T) == 8)
        {
            revSetLongArrayInd((size_t*&)array, idx, size);
            return;
        }
        ASSERTD(sizeof(T) == 4);
        revSetIntArrayInd((uint_t*&)array, idx, size);
    }

    /** Indicate that the given variable was changed. */
    void
    revSetVar(ConstrainedVar* var)
    {
        if (_revDeltaVarsPtr == _revDeltaVarsLim)
        {
            utl::arrayGrow(_revDeltaVars, _revDeltaVarsPtr, _revDeltaVarsLim,
                           utl::max(utl::KB(4), (_revDeltaVarsSize + 1)));
            _revDeltaVarsSize = _revDeltaVarsLim - _revDeltaVars;
        }
        *_revDeltaVarsPtr++ = var;
    }

    /** Indicate that the given variable was changed. */
    void
    revAdd(Constraint* ct)
    {
        if (_revCtsPtr == _revCtsLim)
        {
            utl::arrayGrow(_revCts, _revCtsPtr, _revCtsLim,
                           utl::max(utl::KB(4), (_revCtsSize + 1)));
            _revCtsSize = _revCtsLim - _revCts;
        }
        *_revCtsPtr++ = ct;
        ct->addRef();
    }

    /** Reversibly toggle the given flag. */
    void
    revToggle(bool& flag)
    {
        if (_revTogglesPtr == _revTogglesLim)
        {
            utl::arrayGrow(_revToggles, _revTogglesPtr, _revTogglesLim,
                           utl::max(utl::KB(4), (_revTogglesSize + 1)));
            _revTogglesSize = _revTogglesLim - _revToggles;
        }
        bool* ptr = &flag;
        *_revTogglesPtr++ = ptr;
        flag = !flag;
    }

    /** Delete the given object when backtracking. */
    void
    revAllocate(utl::Object* object)
    {
        if (_revAllocationsPtr == _revAllocationsLim)
        {
            utl::arrayGrow(_revAllocations, _revAllocationsPtr, _revAllocationsLim,
                           utl::max(utl::KB(4), (_revAllocationsSize + 1)));
            _revAllocationsSize = _revAllocationsLim - _revAllocations;
        }
        *_revAllocationsPtr++ = object;
    }

    /** Execute the given function when backtracking. */
    void
    revAction(lut::Functor* action)
    {
        if (_revActionsPtr == _revActionsLim)
        {
            utl::arrayGrow(_revActions, _revActionsPtr, _revActionsLim,
                           utl::max(utl::KB(4), (_revActionsSize + 1)));
            _revActionsSize = _revActionsLim - _revActions;
        }
        *_revActionsPtr++ = action;
    }
    //@}
private:
    using goal_stack_t = std::stack<Goal*>;
    using cp_vector_t = std::vector<ChoicePoint*>;
    using cp_stack_t = std::stack<ChoicePoint*>;
    using cv_set_t = std::set<ConstrainedVar*>;

private:
    void init();
    void deInit();

    ChoicePoint* pushChoicePoint();
    void popChoicePoint();
    void goalStackClear();

    void revSetLong(size_t& i);
    void revSetLongArray(size_t* array, uint_t size);
    void revSetLongInd(size_t*& array, uint_t idx);
    void revSetLongArrayInd(size_t*& array, uint_t idx, uint_t size);

    void revSetInt(uint_t& i);
    void revSetIntArray(uint_t* array, uint_t size);
    void revSetIntInd(uint_t*& array, uint_t idx);
    void revSetIntArrayInd(uint_t*& array, uint_t idx, uint_t size);

    bool backtrack(uint_t depth = uint_t_max);
    void backtrackCP(ChoicePoint* cp);

    // goal stack
    goal_stack_t _goalStack;

    // misc
    ct_set_t _cts;
    cv_set_t _vars;
    lut::SkipListDepthArray* _skipListDepthArray;
    BoundPropagator* _boundPropagator;

    // Backtracking ////////////////////////////////////////////////////////////////////////////////

    // choice points
    cp_vector_t _storedCPs;
    cp_stack_t _cpStack;
    uint_t _cpStackSize;
    ChoicePoint* _topCP;

    // rev-longs
    size_t* _revLongs;
    size_t* _revLongsPtr;
    size_t* _revLongsLim;
    size_t _revLongsSize;

    // rev-long-arrays
    size_t* _revLongArrays;
    size_t* _revLongArraysPtr;
    size_t* _revLongArraysLim;
    size_t _revLongArraysSize;

    // indirect-rev-longs
    size_t* _revLongsInd;
    size_t* _revLongsIndPtr;
    size_t* _revLongsIndLim;
    size_t _revLongsIndSize;

    // indirect rev-long-arrays
    size_t* _revLongArraysInd;
    size_t* _revLongArraysIndPtr;
    size_t* _revLongArraysIndLim;
    size_t _revLongArraysIndSize;

    // rev-ints
    size_t* _revInts;
    size_t* _revIntsPtr;
    size_t* _revIntsLim;
    size_t _revIntsSize;

    // rev-int-arrays
    size_t* _revIntArrays;
    size_t* _revIntArraysPtr;
    size_t* _revIntArraysLim;
    size_t _revIntArraysSize;

    // indirect-rev-ints
    size_t* _revIntsInd;
    size_t* _revIntsIndPtr;
    size_t* _revIntsIndLim;
    size_t _revIntsIndSize;

    // indirect rev-int-arrays
    size_t* _revIntArraysInd;
    size_t* _revIntArraysIndPtr;
    size_t* _revIntArraysIndLim;
    size_t _revIntArraysIndSize;

    // delta-vars
    ConstrainedVar** _revDeltaVars;
    ConstrainedVar** _revDeltaVarsPtr;
    ConstrainedVar** _revDeltaVarsLim;
    size_t _revDeltaVarsSize;

    // toggles
    bool** _revToggles;
    bool** _revTogglesPtr;
    bool** _revTogglesLim;
    size_t _revTogglesSize;

    // added-cts
    Constraint** _revCts;
    Constraint** _revCtsPtr;
    Constraint** _revCtsLim;
    size_t _revCtsSize;

    // backtracking actions
    lut::Functor** _revActions;
    lut::Functor** _revActionsPtr;
    lut::Functor** _revActionsLim;
    size_t _revActionsSize;

    // allocations
    utl::Object** _revAllocations;
    utl::Object** _revAllocationsPtr;
    utl::Object** _revAllocationsLim;
    size_t _revAllocationsSize;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_END;
