#ifndef GOP_REVOPERATOR_H
#define GOP_REVOPERATOR_H

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <gop/Operator.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

GOP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Reversible operator (abstract).

   A reversible operator is an operator whose action can be reversed.
   The action of an operator is to change the instructions for building
   an individual (Ind).  Reversing the action of an operator, then, is
   a simple matter of changing the instructions back to their state before
   the operator was executed.  A reversible operator needs to remember
   the last action it performed, so it can be undone.  The ability to
   undo the actions of an operator is important in some search algorithms
   such as HillClimber.

   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class RevOperator : public Operator
{
    UTL_CLASS_DECL_ABC(RevOperator, Operator);
    UTL_CLASS_DEFID;

public:
    /**
      Constructor.
      \param name operator name
      \param p probability
      \param rng PRNG
   */
    RevOperator(const std::string& name, double p, lut::rng_t* rng = nullptr)
        : Operator(name, p, rng)
    {
    }

    /** Accept the last change. */
    virtual void
    accept()
    {
    }

    /** Undo the last change. */
    virtual void undo() = 0;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

GOP_NS_END;

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
