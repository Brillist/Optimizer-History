#ifndef CLP_MANAGER_H
#define CLP_MANAGER_H

//////////////////////////////////////////////////////////////////////////////

#include <lut/Functor.h>
#include <lut/SkipListDepthArray.h>
#include <clp/Constraint.h>
#include <clp/FailEx.h>

//////////////////////////////////////////////////////////////////////////////

CLP_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////

class Bound;
class ChoicePoint;
class ConstrainedBound;
class ConstrainedVar;
class BoundPropagator;

//////////////////////////////////////////////////////////////////////////////

/**
   Management of constraints, variables, and solution search.

   \author Adam McKee
*/

//////////////////////////////////////////////////////////////////////////////

class Manager : public utl::Object
{
    UTL_CLASS_DECL(Manager);
    UTL_CLASS_NO_COPY;
public:
    typedef std_hash_set<Constraint*, lut::HashUint<Constraint*> >
        ct_set_t;
    typedef ct_set_t::const_iterator ct_const_iterator;
public:
    /** Push the given goal onto the goal stack. */
    void add(const Goal& goal);

    /** Push the given goal onto the goal stack. */
    void add(Goal* goal);

    /** Add the given constraint to the list of constraints. */
    void add(const Constraint& ct);

    /** Add the given constraint to the list of constraints. */
    bool add(Constraint* ct);

    /** Add a constrained variable. */
    void add(ConstrainedVar* var);

    /** Remove a constraint. */
    void remove(Constraint* ct);

    /** Remove the given constrained variable. */
    void remove(ConstrainedVar* var);

    /** Get begin constraint iterator. */
    ct_set_t::const_iterator ctsBegin() const
    { return _cts.begin(); }

    /** Get end constraint iterator. */
    ct_set_t::const_iterator ctsEnd() const
    { return _cts.end(); }

    /** Reset completely. */
    void reset();

    /** Restart the search. */
    void restart();

    /** Find the next solution. */
    bool nextSolution();

    /** Propagate. */
    void propagate();

    /** Push state. */
    void pushState();

    /** Pop state. */
    void popState();

    /** Get the current depth. */
    utl::uint_t depth() const
    { return _cpStackSize; }

    /** Get the skip-list delta array. */
    const lut::SkipListDepthArray* skipListDepthArray() const
    { return _skipListDepthArray; }

    /** Get the bound propagator. */
    BoundPropagator* boundPropagator() const
    { return _boundPropagator; }

    /** Set the bound propagator. */
    void setBoundPropagator(BoundPropagator* bp);

    /// \name Reversible Actions
    //@{
    /** Reversibly increment the given counter. */
    void revIncrement(int& counter, int num = 1)
    { revIncrement((utl::uint_t&)counter, (utl::uint_t)num); }

    /** Reversibly decrement the given counter. */
    void revDecrement(int& counter, int num = 1)
    { revDecrement((utl::uint_t&)counter, (utl::uint_t)num); }

    /** Reversibly increment the given counter. */
    void revIncrement(utl::uint_t& counter, utl::uint_t num = 1)
    {
        ASSERTD(!_cpStack.empty() && (_topCP == _cpStack.top()));
        revSet(counter);
        counter += num;
    }

    /** Reversibly decrement the given counter. */
    void revDecrement(utl::uint_t& counter, utl::uint_t num = 1)
    {
        ASSERTD(!_cpStack.empty() && (_topCP == _cpStack.top()));
        revSet(counter);
        counter -= num;
    }

    /** Reversibly set the given integer. */
    template <class T>
    void revSet(T& i, T v)
    {
        revSet(i);
        i = v;
    }

    /** Reversibly set the given integer. */
    template <class T>
    void revSet(T& i)
    {
#if UTL_HOST_WORDSIZE == 64
        if (sizeof(T) == 8)
        {
            revSetLong((size_t&)i);
            return;
        }
#endif
        ASSERTD(sizeof(T) == 4);
        revSetInt((utl::uint_t&)i);
    }

    /** Reversibly set the given array. */
    template <class T>
    void revSet(T* array, utl::uint_t size)
    {
#if UTL_HOST_WORDSIZE == 64
        if (sizeof(T) == 8)
        {
            revSetLongArray((size_t*)array, size);
            return;
        }
#endif
        ASSERTD(sizeof(T) == 4);
        revSetIntArray((utl::uint_t*)array, size);
    }

    /** Reversible set the given array element (indirectly). */
    template <class T>
    void revSetIndirect(T*& array, utl::uint_t idx)
    {
#if UTL_HOST_WORDSIZE == 64
        if (sizeof(T) == 8)
        {
            revSetLongInd((size_t*&)array, idx);
            return;
        }
#endif
        ASSERTD(sizeof(T) == 4);
        revSetIntInd((utl::uint_t*&)array, idx);
    }

    /** Reversible set the given array (indirectly). */
    template <class T>
    void revSetIndirect(T*& array, utl::uint_t idx, utl::uint_t size)
    {
#if UTL_HOST_WORDSIZE == 64
        if (sizeof(T) == 8)
        {
            revSetLongArrayInd((size_t*&)array, idx, size);
            return;
        }
#endif
        ASSERTD(sizeof(T) == 4);
        revSetIntArrayInd((utl::uint_t*&)array, idx, size);
    }

    /** Indicate that the given variable was changed. */
    void revSetVar(ConstrainedVar* var)
    {
        if (_revDeltaVarsPtr == _revDeltaVarsLim)
        {
            utl::arrayGrow(
                _revDeltaVars,
                _revDeltaVarsPtr,
                _revDeltaVarsLim,
                utl::max(K(4), (_revDeltaVarsSize + 1)));
            _revDeltaVarsSize = _revDeltaVarsLim - _revDeltaVars;
        }
        *(_revDeltaVarsPtr++) = var;
    }

    /** Indicate that the given variable was changed. */
    void revAdd(Constraint* ct)
    {
        if (_revCtsPtr == _revCtsLim)
        {
            utl::arrayGrow(
                _revCts,
                _revCtsPtr,
                _revCtsLim,
                utl::max(K(4), (_revCtsSize + 1)));
            _revCtsSize = _revCtsLim - _revCts;
        }
        *(_revCtsPtr++) = ct;
        ct->addRef();
    }

    /** Reversibly toggle the given flag. */
    void revToggle(bool& flag)
    {
        if (_revTogglesPtr == _revTogglesLim)
        {
            utl::arrayGrow(
                _revToggles,
                _revTogglesPtr,
                _revTogglesLim,
                utl::max(K(4), (_revTogglesSize + 1)));
            _revTogglesSize = _revTogglesLim - _revToggles;
        }
        bool* ptr = &flag;
        *(_revTogglesPtr++) = ptr;
        flag = !flag;
    }

    /** Delete the given object when backtracking. */
    void revAllocate(utl::Object* object)
    {
        if (_revAllocationsPtr == _revAllocationsLim)
        {
            utl::arrayGrow(
                _revAllocations,
                _revAllocationsPtr,
                _revAllocationsLim,
                utl::max(K(4), (_revAllocationsSize + 1)));
            _revAllocationsSize = _revAllocationsLim - _revAllocations;
        }
        *(_revAllocationsPtr++) = object;
    }

    /** Execute the given function when backtracking. */
    void revAction(lut::Functor* action)
    {
        if (_revActionsPtr == _revActionsLim)
        {
            utl::arrayGrow(
                _revActions,
                _revActionsPtr,
                _revActionsLim,
                utl::max(K(4), (_revActionsSize + 1)));
            _revActionsSize = _revActionsLim - _revActions;
        }
        *(_revActionsPtr++) = action;
    }
    //@}
private:
    typedef std::stack<Goal*> goal_stack_t;
    typedef std::vector<ChoicePoint*> cp_vector_t;
    typedef std::stack<ChoicePoint*> cp_stack_t;
    typedef std::set<ConstrainedVar*> cv_set_t;
private:
    void init();
    void deInit();
    ChoicePoint* pushChoicePoint();
    void popChoicePoint();
    void goalStackClear();

#if UTL_HOST_WORDSIZE == 64
    void revSetLong(size_t& i);
    void revSetLongArray(size_t* array, utl::uint_t size);
    void revSetLongInd(size_t*& array, utl::uint_t idx);
    void revSetLongArrayInd(
        size_t*& array,
        utl::uint_t idx,
        utl::uint_t size);
#endif

    void revSetInt(utl::uint_t& i);
    void revSetIntArray(utl::uint_t* array, utl::uint_t size);
    void revSetIntInd(utl::uint_t*& array, utl::uint_t idx);
    void revSetIntArrayInd(
        utl::uint_t*& array,
        utl::uint_t idx,
        utl::uint_t size);

    bool backtrack(utl::uint_t depth = utl::uint_t_max);
    void backtrackCP(ChoicePoint* cp);

    // goal stack
    goal_stack_t   _goalStack;

    // misc
    ct_set_t       _cts;
    cv_set_t       _vars;
    lut::SkipListDepthArray*  _skipListDepthArray;
    BoundPropagator* _boundPropagator;

    // Backtracking ///////////////////////////////////////////////////////////

    // choice points
    cp_vector_t    _storedCPs;
    cp_stack_t     _cpStack;
    utl::uint_t    _cpStackSize;
    ChoicePoint*   _topCP;

#if UTL_HOST_WORDSIZE == 64
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

    // indirect-rev-ints
    size_t* _revLongsInd;
    size_t* _revLongsIndPtr;
    size_t* _revLongsIndLim;
    size_t _revLongsIndSize;

    // indirect rev-arrays
    size_t* _revLongArraysInd;
    size_t* _revLongArraysIndPtr;
    size_t* _revLongArraysIndLim;
    size_t _revLongArraysIndSize;
#endif

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

    // indirect rev-arrays
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

//////////////////////////////////////////////////////////////////////////////

CLP_NS_END;

//////////////////////////////////////////////////////////////////////////////

#endif
