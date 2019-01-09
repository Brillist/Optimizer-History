#include "libclp.h"
#include "Or.h"

//////////////////////////////////////////////////////////////////////////////

LUT_NS_USE;

//////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(clp::Or, clp::Goal);

//////////////////////////////////////////////////////////////////////////////

CLP_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////

Or::Or(
    Goal* g0,
    Goal* g1,
    utl::uint_t label)
    : Goal(g0->manager())
{
    init();
    _label = label;
    _choices.push_back(g0);
    _choices.push_back(g1);
    addRefs();
}

//////////////////////////////////////////////////////////////////////////////

Or::Or(
    Goal* g0,
    Goal* g1,
    Goal* g2,
    utl::uint_t label)
    : Goal(g0->manager())
{
    init();
    _label = label;
    _choices.push_back(g0);
    _choices.push_back(g1);
    _choices.push_back(g2);
    addRefs();
}

//////////////////////////////////////////////////////////////////////////////

Or::Or(
    Goal* g0,
    Goal* g1,
    Goal* g2,
    Goal* g3,
    utl::uint_t label)
    : Goal(g0->manager())
{
    init();
    _label = label;
    _choices.push_back(g0);
    _choices.push_back(g1);
    _choices.push_back(g2);
    _choices.push_back(g3);
    addRefs();
}

//////////////////////////////////////////////////////////////////////////////

Or::Or(
    Goal* g0,
    Goal* g1,
    Goal* g2,
    Goal* g3,
    Goal* g4,
    utl::uint_t label)
    : Goal(g0->manager())
{
    init();
    _label = label;
    _choices.push_back(g0);
    _choices.push_back(g1);
    _choices.push_back(g2);
    _choices.push_back(g3);
    _choices.push_back(g4);
    addRefs();
}

//////////////////////////////////////////////////////////////////////////////

void
Or::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(Or));
    const Or& orGoal = (const Or&)rhs;
    Goal::copy(orGoal);
    clear();
    copyVector(_choices, orGoal._choices);
    addRefs();
}

//////////////////////////////////////////////////////////////////////////////

void
Or::clear()
{
    removeRefCont(_choices);
}

//////////////////////////////////////////////////////////////////////////////

void
Or::addRefs()
{
    goal_vector_t::const_iterator it;
    for (it = _choices.begin(); it != _choices.end(); ++it)
    {
        Goal* goal = *it;
        goal->addRef();
    }
}

//////////////////////////////////////////////////////////////////////////////

CLP_NS_END;
