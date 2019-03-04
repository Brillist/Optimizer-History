#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <gop/Optimizer.h>
#include <gop/RevOperator.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

GOP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Hill-climbing optimizer.

   Hill-climber finds the best solution that is reachable by a series of steps that each result
   in improvement.

   The following pseudo-code explains the operation of HillClimber:

   \code
   S = initial solution
   for (iter = 0; iter != numIterations; ++iter)
   {
      P = randomly selected operator
      S' = P(S)
      if (score(S') > score(S))
      {
         S = S'         
      }
   }
   \endcode

   \ingroup gop
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class HillClimber : public Optimizer
{
    UTL_CLASS_DECL(HillClimber, Optimizer);
    UTL_CLASS_DEFID;

public:
    virtual void initialize(const OptimizerConfiguration* config);

    virtual bool run();

    virtual void audit();

    /** Run an iteration and return complete() or not. */
    virtual bool HCiterationRun();

private:
    virtual void acceptanceEval(RevOperator* op);
};

////////////////////////////////////////////////////////////////////////////////////////////////////

GOP_NS_END;
