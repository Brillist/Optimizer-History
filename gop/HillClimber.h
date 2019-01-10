#ifndef GOP_HILLCLIMBER_H
#define GOP_HILLCLIMBER_H

////////////////////////////////////////////////////////////////////////////////////////////////////

/* #include <gop/IndEvaluator.h> */
/* #include <gop/Objective.h> */
#include <gop/Optimizer.h>
/* #include <gop/StringInd.h> */
#include <gop/RevOperator.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

GOP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Hill-climbing optimizer.

   The operation of a hill-climber is easy to understand.
   The following pseudo-code should make it clear:

   \code
   S = initial solution
   for (iter = 0; iter < numIterations; ++iter)
   {
      P = randomly selected operator
      S' = P(S)
      if (S'.score > S.score)
      {
         S = S'         
      }
   }
   \endcode

   This algorithm has the appeal of being very simple.  However, the danger
   of this search strategy is that it may happily climb a very small hill,
   and be unable to jump to a taller hill.

   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class HillClimber : public Optimizer
{
    UTL_CLASS_DECL(HillClimber);

public:
    //     virtual bool requiresRevOp() const
    //     { return true; }

    /** Initialize. */
    virtual void initialize(const OptimizerConfiguration* config);

    /** decide whether to accept the new solution. */
    virtual void acceptanceEval(RevOperator* op);

    /** Run an iteration and return complete() or not. */
    virtual bool HCiterationRun();

    /** Run the hill-climber. */
    virtual bool run();

    /** Audit the result. */
    virtual void audit();

private:
    void
    init()
    {
    }
    void
    deInit()
    {
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////

GOP_NS_END;

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
