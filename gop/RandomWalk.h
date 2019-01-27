#ifndef GOP_RANDOMWALK_H
#define GOP_RANDOMWALK_H

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <gop/Optimizer.h>
#include <gop/StringInd.h>
#include <gop/StringScore.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

GOP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Random Walk optimizer.

   It is different from Hill Climber. It ignores the score of current node
   and the best score which has been found  so far, and keeps on walking 
   to the next neighbouring node randomly.

   It is also different from Random Sampling. It walks in the search space 
   one step at a time.

   Joe Zhou
   Oct. 2005
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class RandomWalk : public Optimizer
{
    UTL_CLASS_DECL(RandomWalk, Optimizer);

public:
    virtual bool
    requiresRevOp() const
    {
        return true;
    }

    /** Initialize. */
    virtual void initialize(const OptimizerConfiguration* config);

    void
    setBestStrScore(StringScore* strScore)
    {
        delete _bestStrScore;
        _bestStrScore = strScore;
    }

    /** Run the Random Walk. */
    virtual bool run();

    /** Audit the result */
    virtual void audit();

protected:
    StringScore* _bestStrScore;

private:
    void init();
    void deInit();
};

////////////////////////////////////////////////////////////////////////////////////////////////////

GOP_NS_END;

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
