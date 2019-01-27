#include "libclp.h"
#include "BoundPropagator.h"
#include "ChoicePoint.h"
#include "ConstrainedVar.h"
#include "FailEx.h"
#include "IntSpan.h"
#include "Or.h"
#include "Manager.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(clp::Manager);

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Manager::add(const Goal& goal)
{
    add(goal.clone());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Manager::add(Goal* goal)
{
    _goalStack.push(goal);
    goal->addRef();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Manager::add(const Constraint& ct)
{
    ((Constraint&)ct).mclone();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool
Manager::add(Constraint* ct)
{
    if (_cts.find(ct) != _cts.end())
    {
        return false;
    }

    ASSERTD(!_cpStack.empty() && (_topCP == _cpStack.top()));
    revAdd(ct);
    ct->setManaged(true);
    _cts.insert(ct);
    ct->addRef();
    try
    {
        ct->postExpConstraints();
        ct->post();
        propagate();
    }
    catch (FailEx&)
    {
        throw;
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Manager::add(ConstrainedVar* var)
{
    ASSERTD(!var->isManaged());
    _vars.insert(var);
    var->setManaged(true);
    var->postConstraints();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Manager::remove(Constraint* ct)
{
    ct->unpost();
    if (_cts.erase(ct) != 0)
    {
        ct->removeRef();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Manager::remove(ConstrainedVar* var)
{
    _vars.erase(var);
    delete var;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Manager::reset()
{
    backtrack(uint_t_max - 2);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Manager::restart()
{
    ASSERTD(!_cpStack.empty());
    backtrack(uint_t_max - 1);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool
Manager::nextSolution()
{
    bool firstSearch = (_cpStackSize == 1);

    // first search?
    if (firstSearch)
    {
        // create root choice point
        pushChoicePoint();

        // reverse the goal stack
        goal_stack_t goalStack;
        while (!_goalStack.empty())
        {
            Goal* goal = _goalStack.top();
            _goalStack.pop();
            goalStack.push(goal);
        }
        _goalStack = goalStack;
    }
    // otherwise backtrack to continue the search
    else
    {
        if (!backtrack())
        {
            return false;
        }
    }

    while (!_goalStack.empty())
    {
        Goal* goal = _goalStack.top();
        _goalStack.pop();
        if (dynamic_cast<Or*>(goal) == nullptr)
        {
            try
            {
                goal->execute();
                propagate();
                goal->removeRef();
            }
            catch (FailEx& failEx)
            {
                goal->removeRef();
                if (!backtrack(failEx.label()))
                {
                    return false;
                }
            }
        }
        else
        {
            Or* orGoal = (Or*)goal;
            ChoicePoint* cp = _topCP;
            if (cp->orGoal() != orGoal)
            {
                cp = pushChoicePoint();
                cp->set(orGoal, _goalStack);
            }
            Goal* goal = cp->getNextChoice();
            _goalStack.push(goal);
            goal->addRef();
            orGoal->removeRef();
        }
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Manager::propagate()
{
    _boundPropagator->propagate();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Manager::pushState()
{
    pushChoicePoint();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Manager::popState()
{
    // must be choice point below root
    ASSERTD(_cpStackSize >= 2);
    auto cp = _topCP;

    // clear propagation queue
    _boundPropagator->clearPropQ();

    // backtrack on the choice point
    backtrackCP(cp);

    // remove the choice point
    popChoicePoint();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Manager::setBoundPropagator(BoundPropagator* bp)
{
    delete _boundPropagator;
    _boundPropagator = bp;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Manager::init()
{
    _cpStackSize = 0;
    _topCP = nullptr;

    _revLongsSize = 0;
    _revLongArraysSize = 0;
    _revLongsIndSize = 0;
    _revLongArraysIndSize = 0;
    _revLongs = _revLongsPtr = _revLongsLim = nullptr;
    _revLongArrays = _revLongArraysPtr = _revLongArraysLim = nullptr;
    _revLongsInd = _revLongsIndPtr = _revLongsIndLim = nullptr;
    _revLongArraysInd = _revLongArraysIndPtr = _revLongArraysIndLim = nullptr;

    _revIntsSize = 0;
    _revIntArraysSize = 0;
    _revIntsIndSize = 0;
    _revIntArraysIndSize = 0;
    _revDeltaVarsSize = 0;
    _revTogglesSize = 0;
    _revCtsSize = 0;
    _revActionsSize = 0;
    _revAllocationsSize = 0;
    _revInts = _revIntsPtr = _revIntsLim = nullptr;
    _revIntArrays = _revIntArraysPtr = _revIntArraysLim = nullptr;
    _revIntsInd = _revIntsIndPtr = _revIntsIndLim = nullptr;
    _revIntArraysInd = _revIntArraysIndPtr = _revIntArraysIndLim = nullptr;
    _revDeltaVars = _revDeltaVarsPtr = _revDeltaVarsLim = nullptr;
    _revToggles = _revTogglesPtr = _revTogglesLim = nullptr;
    _revCts = _revCtsPtr = _revCtsLim = nullptr;
    _revActions = _revActionsPtr = _revActionsLim = nullptr;
    _revAllocations = _revAllocationsPtr = _revAllocationsLim = nullptr;

    // init skip-list delta array
    _skipListDepthArray = new SkipListDepthArray(CLP_INTSPAN_MAXDEPTH);

    // create bound propagator
    _boundPropagator = new BoundPropagator(this);

    // create root choice point
    pushChoicePoint();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Manager::deInit()
{
    goalStackClear();

    for (ct_set_t::iterator it = _cts.begin(); it != _cts.end(); ++it)
    {
        (*it)->setPosted(false);
    }
    removeRefCont(_cts);
    deleteCont(_vars);
    delete _skipListDepthArray;
    delete _boundPropagator;

    deleteCont(_storedCPs);

    removeRefArray(_revCts, _revCtsPtr);
    deleteArray(_revAllocations, _revAllocationsPtr);

    delete[] _revLongs;
    delete[] _revLongArrays;
    delete[] _revLongsInd;
    delete[] _revLongArraysInd;
    delete[] _revInts;
    delete[] _revIntArrays;
    delete[] _revIntsInd;
    delete[] _revIntArraysInd;
    delete[] _revDeltaVars;
    delete[] _revToggles;
    delete[] _revCts;
    delete[] _revActions;
    delete[] _revAllocations;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

ChoicePoint*
Manager::pushChoicePoint()
{
    propagate();
    uint_t d = depth();
    while (_storedCPs.size() <= d)
    {
        _storedCPs.push_back(nullptr);
    }
    ChoicePoint* cp = _storedCPs[d];
    if (cp == nullptr)
    {
        cp = new ChoicePoint();
        _storedCPs[d] = cp;
    }

    cp->setManager(this);
    cp->setRevLongsIdx(_revLongsPtr - _revLongs);
    cp->setRevLongArraysIdx(_revLongArraysPtr - _revLongArrays);
    cp->setRevLongsIndIdx(_revLongsIndPtr - _revLongsInd);
    cp->setRevLongArraysIndIdx(_revLongArraysIndPtr - _revLongArraysInd);
    cp->setRevIntsIdx(_revIntsPtr - _revInts);
    cp->setRevIntArraysIdx(_revIntArraysPtr - _revIntArrays);
    cp->setRevIntsIndIdx(_revIntsIndPtr - _revIntsInd);
    cp->setRevIntArraysIndIdx(_revIntArraysIndPtr - _revIntArraysInd);
    cp->setRevDeltaVarsIdx(_revDeltaVarsPtr - _revDeltaVars);
    cp->setRevTogglesIdx(_revTogglesPtr - _revToggles);
    cp->setRevCtsIdx(_revCtsPtr - _revCts);
    cp->setRevActionsIdx(_revActionsPtr - _revActions);
    cp->setRevAllocationsIdx(_revAllocationsPtr - _revAllocations);
    _cpStack.push(cp);
    ++_cpStackSize;
    _topCP = cp;

    return cp;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Manager::popChoicePoint()
{
    auto cp = _topCP;
    cp->clear();
    _cpStack.pop();
    --_cpStackSize;
    _topCP = _cpStack.top();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Manager::goalStackClear()
{
    while (!_goalStack.empty())
    {
        auto goal = _goalStack.top();
        _goalStack.pop();
        goal->removeRef();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Manager::revSetLong(size_t& i)
{
    // grow if necessary
    if (_revLongsPtr == _revLongsLim)
    {
        utl::arrayGrow(_revLongs, _revLongsPtr, _revLongsLim, utl::max(utl::KB(4), (_revLongsSize + 1)));
        _revLongsSize = _revLongsLim - _revLongs;
    }
    *_revLongsPtr++ = (size_t)&i;
    *_revLongsPtr++ = (size_t)i;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Manager::revSetLongArray(size_t* array, uint_t size)
{
    // grow if necessary
    uint_t growth = size + 2;
    uint_t room = (_revLongArraysLim - _revLongArraysPtr);
    if (room < growth)
    {
        size_t curSize = (_revLongArraysPtr - _revLongArrays);
        utl::arrayGrow(_revLongArrays, _revLongArraysPtr, _revLongArraysLim,
                       utl::max(utl::KB(4), curSize + growth));
        _revLongArraysSize = _revLongArraysLim - _revLongArrays;
    }

    // copy array into _revLongArrays
    size_t* lhs = _revLongArraysPtr;
    size_t* rhs = array;
    _revLongArraysPtr += size;
    while (lhs < _revLongArraysPtr)
    {
        *lhs++ = *rhs++;
    }

    // size and start address
    *_revLongArraysPtr++ = size;
    *_revLongArraysPtr++ = (size_t)array;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Manager::revSetLongInd(size_t*& array, uint_t idx)
{
    // grow if necessary
    uint_t room = (_revLongsIndLim - _revLongsIndPtr);
    if (room < 3)
    {
        size_t curSize = (_revLongsIndPtr - _revLongsInd);
        utl::arrayGrow(_revLongsInd, _revLongsIndPtr, _revLongsIndLim, utl::max(utl::KB(4), curSize + 3));
        _revLongsIndSize = _revLongsIndLim - _revLongsInd;
    }

    *_revLongsIndPtr++ = array[idx];
    *_revLongsIndPtr++ = idx;
    *_revLongsIndPtr++ = (size_t)&array;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Manager::revSetLongArrayInd(size_t*& array, uint_t idx, uint_t size)
{
    // grow if necessary
    uint_t growth = size + 3;
    uint_t room = (_revLongArraysIndLim - _revLongArraysIndPtr);
    if (room < growth)
    {
        size_t curSize = (_revLongArraysIndPtr - _revLongArraysInd);
        utl::arrayGrow(_revLongArraysInd, _revLongArraysIndPtr, _revLongArraysIndLim,
                       utl::max(utl::KB(4), curSize + growth));
        _revLongArraysIndSize = _revLongArraysIndLim - _revLongArraysInd;
    }

    // copy array into _revLongArraysInd
    size_t* lhs = _revLongArraysIndPtr;
    size_t* rhs = array + idx;
    _revLongArraysIndPtr += size;
    while (lhs < _revLongArraysIndPtr)
    {
        *lhs++ = *rhs++;
    }

    // size, index, and start address
    *_revLongArraysIndPtr++ = size;
    *_revLongArraysIndPtr++ = idx;
    *_revLongArraysIndPtr++ = (size_t)&array;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Manager::revSetInt(uint_t& i)
{
    // grow if necessary
    if (_revIntsPtr == _revIntsLim)
    {
        utl::arrayGrow(_revInts, _revIntsPtr, _revIntsLim, utl::max(utl::KB(4), (_revIntsSize + 2)));
        _revIntsSize = _revIntsLim - _revInts;
    }

    *_revIntsPtr++ = (size_t)&i;
    *_revIntsPtr++ = i;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Manager::revSetIntArray(uint_t* array, uint_t size)
{
    // grow if necessary
    uint_t growth = size + 2;
    uint_t room = (_revIntArraysLim - _revIntArraysPtr);
    if (room < growth)
    {
        size_t curSize = (_revIntArraysPtr - _revIntArrays);
        utl::arrayGrow(_revIntArrays, _revIntArraysPtr, _revIntArraysLim,
                       utl::max(utl::KB(4), curSize + growth));
        _revIntArraysSize = _revIntArraysLim - _revIntArrays;
    }

    // copy array into _revIntArrays
    size_t* lhs = _revIntArraysPtr;
    uint_t* rhs = array;
    _revIntArraysPtr += size;
    while (lhs < _revIntArraysPtr)
    {
        *lhs++ = *rhs++;
    }

    // size and start address
    *_revIntArraysPtr++ = size;
    *_revIntArraysPtr++ = (size_t)array;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Manager::revSetIntInd(uint_t*& array, uint_t idx)
{
    // grow if necessary
    uint_t room = (_revIntsIndLim - _revIntsIndPtr);
    if (room < 3)
    {
        size_t curSize = (_revIntsIndPtr - _revIntsInd);
        utl::arrayGrow(_revIntsInd, _revIntsIndPtr, _revIntsIndLim, utl::max(utl::KB(4), curSize + 3));
        _revIntsIndSize = _revIntsIndLim - _revIntsInd;
    }

    *_revIntsIndPtr++ = array[idx];
    *_revIntsIndPtr++ = idx;
    *_revIntsIndPtr++ = (size_t)&array;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Manager::revSetIntArrayInd(uint_t*& array, uint_t idx, uint_t size)
{
    // grow if necessary
    uint_t growth = size + 3;
    uint_t room = (_revIntArraysIndLim - _revIntArraysIndPtr);
    if (room < growth)
    {
        size_t curSize = (_revIntArraysIndPtr - _revIntArraysInd);
        utl::arrayGrow(_revIntArraysInd, _revIntArraysIndPtr, _revIntArraysIndLim,
                       utl::max(utl::KB(4), curSize + growth));
        _revIntArraysIndSize = _revIntArraysIndLim - _revIntArraysInd;
    }

    // copy array into _revIntArraysInd
    size_t* lhs = _revIntArraysIndPtr;
    uint_t* rhs = array + idx;
    _revIntArraysIndPtr += size;
    while (lhs < _revIntArraysIndPtr)
    {
        *lhs++ = *rhs++;
    }

    // size, index, and start address
    *_revIntArraysIndPtr++ = size;
    *_revIntArraysIndPtr++ = idx;
    *_revIntArraysIndPtr++ = (size_t)&array;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool
Manager::backtrack(uint_t label)
{
    ASSERTD(!_cpStack.empty());

    _boundPropagator->clearPropQ();

    for (uint_t depth = _cpStackSize; depth > 0; depth--)
    {
        ChoicePoint* cp = _topCP;

        // backtrack on the choice point
        backtrackCP(cp);

        // can we resume the search at this choice point?
        if (cp->hasRemainingChoice() &&
            (!cp->isLabeled() || (label == uint_t_max) || (label == cp->label())))
        {
            Or* orGoal = cp->orGoal();
            _goalStack.push(orGoal);
            orGoal->addRef();
            return true;
        }
        else if (depth > 1) // never get rid of root choice point
        {
            popChoicePoint();
        }
    }

    return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Manager::backtrackCP(ChoicePoint* cp)
{
    // backtrack the choice point
    cp->backtrack(_goalStack);

    // backtrack rev-long-arrays
    size_t* revLongArraysBegin = _revLongArrays + cp->getRevLongArraysIdx();
    while (_revLongArraysPtr > revLongArraysBegin)
    {
        size_t* array = (size_t*)*(--_revLongArraysPtr);
        size_t size = *(--_revLongArraysPtr);

        // copy array
        size_t* rhsLim = _revLongArraysPtr;
        _revLongArraysPtr -= size;
        size_t* lhs = array;
        size_t* rhs = _revLongArraysPtr;
        while (rhs < rhsLim)
        {
            *lhs++ = *rhs++;
        }
    }

    // backtrack rev-longs
    size_t* revLongsBegin = _revLongs + cp->getRevLongsIdx();
    while (_revLongsPtr > revLongsBegin)
    {
        size_t v = *(--_revLongsPtr);
        size_t* i = (size_t*)*(--_revLongsPtr);
        *i = v;
    }

    // backtrack indirect rev-long-arrays
    size_t* revLongArraysIndBegin = _revLongArraysInd + cp->getRevLongArraysIndIdx();
    while (_revLongArraysIndPtr > revLongArraysIndBegin)
    {
        size_t** arrayPtr = (size_t**)*(--_revLongArraysIndPtr);
        size_t idx = *(--_revLongArraysIndPtr);
        size_t size = *(--_revLongArraysIndPtr);

        // copy array
        size_t* rhsLim = _revLongArraysIndPtr;
        _revLongArraysIndPtr -= size;
        size_t* lhs = *arrayPtr + idx;
        size_t* rhs = _revLongArraysIndPtr;
        while (rhs < rhsLim)
        {
            *lhs++ = *rhs++;
        }
    }

    // backtrack indirect rev-longs
    size_t* revLongsIndBegin = _revLongsInd + cp->getRevLongsIndIdx();
    while (_revLongsIndPtr > revLongsIndBegin)
    {
        size_t** arrayPtr = (size_t**)*(--_revLongsIndPtr);
        size_t idx = *(--_revLongsIndPtr);
        size_t val = *(--_revLongsIndPtr);
        (*arrayPtr)[idx] = val;
    }

    // backtrack rev-int-arrays
    size_t* revIntArraysBegin = _revIntArrays + cp->getRevIntArraysIdx();
    while (_revIntArraysPtr > revIntArraysBegin)
    {
        uint_t* array = (uint_t*)*(--_revIntArraysPtr);
        size_t size = *(--_revIntArraysPtr);

        // copy array
        size_t* rhsLim = _revIntArraysPtr;
        _revIntArraysPtr -= size;
        uint_t* lhs = array;
        size_t* rhs = _revIntArraysPtr;
        while (rhs < rhsLim)
        {
            *lhs++ = *rhs++;
        }
    }

    // backtrack rev-ints
    size_t* revIntsBegin = _revInts + cp->getRevIntsIdx();
    while (_revIntsPtr > revIntsBegin)
    {
        uint_t v = *(--_revIntsPtr);
        uint_t* i = (uint_t*)*(--_revIntsPtr);
        *i = v;
    }

    // backtrack indirect rev-int-arrays
    size_t* revIntArraysIndBegin = _revIntArraysInd + cp->getRevIntArraysIndIdx();
    while (_revIntArraysIndPtr > revIntArraysIndBegin)
    {
        uint_t** arrayPtr = (uint_t**)*(--_revIntArraysIndPtr);
        size_t idx = *(--_revIntArraysIndPtr);
        size_t size = *(--_revIntArraysIndPtr);

        // copy array
        size_t* rhsLim = _revIntArraysIndPtr;
        _revIntArraysIndPtr -= size;
        uint_t* lhs = *arrayPtr + idx;
        size_t* rhs = _revIntArraysIndPtr;
        while (rhs < rhsLim)
        {
            *lhs++ = *rhs++;
        }
    }

    // backtrack indirect rev-ints
    size_t* revIntsIndBegin = _revIntsInd + cp->getRevIntsIndIdx();
    while (_revIntsIndPtr > revIntsIndBegin)
    {
        uint_t** arrayPtr = (uint_t**)*(--_revIntsIndPtr);
        size_t idx = *(--_revIntsIndPtr);
        uint_t val = *(--_revIntsIndPtr);
        (*arrayPtr)[idx] = val;
    }

    // backtrack changed variables
    ConstrainedVar** revDeltaVarsBegin = _revDeltaVars + cp->getRevDeltaVarsIdx();
    while (_revDeltaVarsPtr > revDeltaVarsBegin)
    {
        ConstrainedVar* var = *(--_revDeltaVarsPtr);
        var->backtrack();
    }

    // toggle flags that were toggled
    bool** revTogglesBegin = _revToggles + cp->getRevTogglesIdx();
    while (_revTogglesPtr > revTogglesBegin)
    {
        bool* b = *(--_revTogglesPtr);
        *b = !*b;
    }

    // remove constraints that were added
    Constraint** revCtsBegin = _revCts + cp->getRevCtsIdx();
    while (_revCtsPtr > revCtsBegin)
    {
        Constraint* ct = *(--_revCtsPtr);
        remove(ct);
        ct->removeRef();
    }

    // run backtracking actions
    Functor** revActionsBegin = _revActions + cp->getRevActionsIdx();
    while (_revActionsPtr > revActionsBegin)
    {
        Functor* action = *(--_revActionsPtr);
        action->execute();
        delete action;
    }

    // delete objects that were allocated
    Object** revAllocationsBegin = _revAllocations + cp->getRevAllocationsIdx();
    while (_revAllocationsPtr > revAllocationsBegin)
    {
        Object* object = *(--_revAllocationsPtr);
        delete object;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_END;
