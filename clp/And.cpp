#include "libclp.h"
#include "And.h"
#include "Manager.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(clp::And, utl::Object);

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

And::And(Goal* g0, Goal* g1)
    : Goal(g0->manager())
{
    init();
    _goals.push_back(g0);
    _goals.push_back(g1);
    addRefs();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

And::And(Goal* g0, Goal* g1, Goal* g2)
    : Goal(g0->manager())
{
    init();
    _goals.push_back(g0);
    _goals.push_back(g1);
    _goals.push_back(g2);
    addRefs();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

And::And(Goal* g0, Goal* g1, Goal* g2, Goal* g3)
    : Goal(g0->manager())
{
    init();
    _goals.push_back(g0);
    _goals.push_back(g1);
    _goals.push_back(g2);
    _goals.push_back(g3);
    addRefs();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

And::And(Goal* g0, Goal* g1, Goal* g2, Goal* g3, Goal* g4)
    : Goal(g0->manager())
{
    init();
    _goals.push_back(g0);
    _goals.push_back(g1);
    _goals.push_back(g2);
    _goals.push_back(g3);
    _goals.push_back(g4);
    addRefs();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
And::copy(const Object& rhs)
{
    ASSERTD(dynamic_cast<const And*>(&rhs) != nullptr);
    const And& andGoal = (const And&)rhs;
    Goal::copy(andGoal);
    clear();
    copyVector(_goals, andGoal._goals);
    addRefs();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
And::clear()
{
    removeRefCont(_goals);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
And::execute()
{
    Manager* mgr = manager();

    ASSERTD(!_goals.empty());

    goal_vector_t::iterator it = _goals.end();
    for (--it;; --it)
    {
        Goal* goal = *it;
        mgr->add(goal);
        if (it == _goals.begin())
        {
            break;
        }
    }
    clear();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
And::addRefs()
{
    goal_vector_t::const_iterator it;
    for (it = _goals.begin(); it != _goals.end(); ++it)
    {
        Goal* goal = *it;
        goal->addRef();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_END;
