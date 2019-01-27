#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <gop/Optimizer.h>
#include <gop/StringScore.h>
#include <gop/Score.h>
#include <gop/RevOperator.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

GOP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/** 
    History of SA (Simulated Annealing), a general purpose optimization 
    algorithm. 
    In 1953, Metropolis et al. proposed an algorithm for the efficient
    simulation of the evolution of a solid to thermal equilibrium. It took
    almost 30 years before Kirkpatrick et al. (1982) and, independently,
    Cerny realized that there exists a profound analogy between minimizing 
    the cost function of a combinatorial optimization problem and the slow 
    cooling of a solid until it reaches its low energy ground state and 
    that the optimization process can be realized by applying the 
    Metropolis criterion. Kirkpatrick and his co-workers created such a 
    combinatorial optimization algorithm, and called it 'simulated 
    annealing'.

    Joe Zhou
    Dec. 2005
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class SAoptimizer : public Optimizer
{
    UTL_CLASS_DECL(SAoptimizer, Optimizer);

public:
    /** Initialize. */
    virtual void initialize(const OptimizerConfiguration* config);

    /** set _acceptedScore. */
    void
    setAcceptedScore(Score* score)
    {
        delete _acceptedScore;
        _acceptedScore = score;
    }

    /** set _bestStrScore. */
    void
    setBestStrScore(StringScore* strScore)
    {
        delete _bestStrScore;
        _bestStrScore = strScore;
    }

    /** store score difference. */
    void storeScoreDiff(double diff);

    /** reset _initTemp. */
    void resetInitTemp(double acceptanceRatio, bool reinit = false);

    /** decide whether to accept the new solution. */
    virtual void acceptanceEval(RevOperator* op);

    /** Run an iteration and return complete() or not. */
    virtual bool SAiterationRun();

    /** Run the SA. */
    virtual bool run();

    /** dump a string of temperature parameters. */
    utl::String temperatureString() const;

    /** Audit the result */
    virtual void audit();

protected:
    StringScore* _bestStrScore;
    Score* _acceptedScore;
    double _scoreDiff;
    double _tempDcrRate;
    double _initTemp;    //initial temperature
    double _currentTemp; //current temperature
    double _stopTemp;    //stop temperature

    double _totalScoreDiff;          //total socre changes for every move
    uint_t _totalScoreDiffIter; //total #iters for _totalScoreDiff

    /** Optimizer customization parameters */
    bool _fixedInitTemp;
    uint_t _populationSize; // #tries at each temperature.
    uint_t _tempIteration;  // count #iters at each temperature.
private:
    void init();
    void deInit();
};

////////////////////////////////////////////////////////////////////////////////////////////////////

GOP_NS_END;
