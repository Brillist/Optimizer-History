#include "libclp.h"
#include "Manager.h"
#include "Or.h"
#include "ChoicePoint.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

//#ifdef DEBUG
//#define DEBUG_UNIT
//#endif

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ChoicePoint::clear()
{
    if (_or != nullptr)
        _or->removeRef();
    _or = nullptr;
    _orIdx = 0;
    _revLongsIdx = 0;
    _revLongArraysIdx = 0;
    _revLongsIndIdx = 0;
    _revLongArraysIndIdx = 0;
    _revIntsIdx = 0;
    _revIntArraysIdx = 0;
    _revIntsIndIdx = 0;
    _revIntArraysIndIdx = 0;
    _revDeltaVarsIdx = 0;
    _revCtsIdx = 0;
    _revTogglesIdx = 0;
    _revAllocationsIdx = 0;
    _revActionsIdx = 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ChoicePoint::set(Or* orGoal, const goal_stack_t& goalStack)
{
    if (_or != nullptr)
    {
        _or->removeRef();
    }
    _or = orGoal;
    if (_or == nullptr)
    {
        _mgr = nullptr;
    }
    else
    {
        _mgr = _or->manager();
        _or->addRef();
    }
    _orIdx = 0;
    saveGoalStack(goalStack);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool
ChoicePoint::hasRemainingChoice() const
{
    // root only has one choice
    if (isRoot())
    {
        return false;
    }

    return (_orIdx < _or->numChoices());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool
ChoicePoint::isLabeled() const
{
    return ((_or == nullptr) || (_or->label() != uint_t_max));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

uint_t
ChoicePoint::label() const
{
    return (_or == nullptr) ? uint_t_max - 1 : _or->label();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Goal*
ChoicePoint::getNextChoice()
{
    ASSERTD(hasRemainingChoice());
    Goal* nextChoice = _or->getChoice(_orIdx++);
    return nextChoice;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ChoicePoint::backtrack(goal_stack_t& goalStack)
{
    // restore goal stack
    restoreGoalStack(goalStack);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ChoicePoint::init()
{
    _or = nullptr;
    clear();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ChoicePoint::deInit()
{
    clear();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ChoicePoint::saveGoalStack(const goal_stack_t& goalStack)
{
    goal_stack_t gscopy = goalStack;
    while (!gscopy.empty())
    {
        Goal* goal = gscopy.top();
        gscopy.pop();
        _goals.push_back(goal);
        goal->addRef();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ChoicePoint::restoreGoalStack(goal_stack_t& goalStack)
{
    // empty the goal stack
    while (!goalStack.empty())
    {
        Goal* goal = goalStack.top();
        goalStack.pop();
        goal->removeRef();
    }

    uint_t numGoals = _goals.size();
    if (numGoals == 0)
    {
        return;
    }

    goal_vector_t::const_iterator it = _goals.end();
    while (it != _goals.begin())
    {
        --it;
        Goal* goal = *it;
        goalStack.push(goal);
        goal->addRef();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_END;
