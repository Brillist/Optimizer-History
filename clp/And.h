#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <clp/Goal.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Logical AND of goals.

   The And goal requires *all* of a series of goals to be executed.  The provided goals are
   pushed onto the Manager's goal stack in reverse order, so they will be executed in the same
   order as they are listed.

   \ingroup clp
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class And : public Goal
{
    UTL_CLASS_DECL(And, Goal);

public:
    /**
       Constructor.
       \param g0 first goal
       \param g1 second goal
     */
    And(Goal* g0, Goal* g1);

    virtual void copy(const utl::Object& rhs);

    /** Clear the list of goals. */
    void clear();

    /** Push all goals onto the goal stack in reverse order. */
    virtual void execute();

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

private:
    goal_vector_t _goals;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_END;
