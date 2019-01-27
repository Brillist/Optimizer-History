#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <clp/Goal.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Logical AND of goals.

   The And goal requires \b all of a series of goals to be executed.
   The provided goals are pushed onto the Manager's goal stack in reverse
   order, so they will be executed in the same order as they are listed.

   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class And : public Goal
{
    UTL_CLASS_DECL(And, Goal);

public:
    /** Constructor. */
    And(Goal* g0, Goal* g1);

    /** Constructor. */
    And(Goal* g0, Goal* g1, Goal* g2);

    /** Constructor. */
    And(Goal* g0, Goal* g1, Goal* g2, Goal* g3);

    /** Constructor. */
    And(Goal* g0, Goal* g1, Goal* g2, Goal* g3, Goal* g4);

    virtual void copy(const utl::Object& rhs);

    /** Clear the list of goals. */
    void clear();

    /** Push all goals onto the goal stack in reverse order. */
    virtual void execute();

private:
    typedef std::vector<Goal*> goal_vector_t;
    typedef goal_vector_t::iterator iterator;
    typedef goal_vector_t::const_iterator const_iterator;

private:
    void
    init()
    {
    }
    void
    deInit()
    {
        clear();
    }
    void addRefs();

private:
    goal_vector_t _goals;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_END;
