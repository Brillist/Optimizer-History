#ifndef GOP_OPTIMIZER_H
#define GOP_OPTIMIZER_H

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <libutl/RandNumGen.h>
/* #include <libutl/OStimer.h> */
#include <gop/Operator.h>
#include <gop/OptimizerConfiguration.h>
#include <gop/IndBuilder.h>
#include <gop/IndEvaluator.h>
#include <gop/Population.h>
#include <gop/RunStatus.h>
#include <gop/Score.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

GOP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Optimizer (abstract).

   All optimization algorithms are represented as sub-classes of Optimizer.
   Optimizer is the home of methods and data that are common among
   different optimization strategies.

   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class Optimizer : public utl::Object
{
    UTL_CLASS_DECL_ABC(Optimizer);

public:
    /// \name Run Control
    //@{
    /** Is the run complete? */
    bool complete() const;

    /** stop the run. */
    void stop();

    /** Initialize. */
    virtual void initialize(const OptimizerConfiguration* config);

    /**
       Iterate the given number of times.
       \return true if new best result or run complete, false otherwise
       \param numIterations maximum number of iterations to run
    */
    virtual bool run() = 0;

    /** iteration run: run just one iteration. */
    virtual bool iterationRun(Operator* op = nullptr, bool audit = false);

    virtual void updateRunStatus(bool complete);
    //@}

    /// \name Accessors
    //@{
    /** Get the random number generator. */
    utl::RandNumGen*
    rng() const
    {
        return _rng;
    }

    /** Get the ind-builder. */
    const IndBuilder*
    indBuilder() const
    {
        return _indBuilder;
    }

    /** Get the iteration number. */
    utl::uint_t
    iteration() const
    {
        return _iteration;
    }

    /** Get the last improvement iteration number. */
    utl::uint_t
    improvementIteration() const
    {
        return _improvementIteration;
    }

    /** Get the maximum number of iterations. */
    utl::uint_t
    maxIterations() const
    {
        return _maxIterations;
    }

    /** Get the optimization goal for the given objective. */
    minmax_goal_t
    goal(utl::uint_t idx = 0) const
    {
        ASSERTD(_objectives.size() > idx);
        return _objectives[idx]->goal();
    }

    /** Get the list of objectives. */
    const std::vector<Objective*>&
    objectives() const
    {
        return _objectives;
    }

    /** Get the best score for the given objective. */
    Score* bestScore(utl::uint_t objectiveIdx = 0) const;

    /** Get the best score for the given objective. */
    Score* bestScore(const std::string& objectiveName) const;

    /** Get a named component of the best score. */
    int bestScoreComponent(const std::string& objectiveName,
                           const std::string& componentName) const;

    /** Get the audit text for the best score. */
    const std::string& bestScoreAudit() const;

    /** Get the number of operators. */
    utl::uint_t
    numOperators() const
    {
        return _ops.size();
    }

    /** Get the list of operators. */
    const std::vector<Operator*>&
    operators() const
    {
        return _ops;
    }

    //    /** Get the statistics flag. */
    //    bool calcStats() const
    //    { return _calcStatsEnable; }

    //    /** Set the statistic flag. */
    //    void setCalcStats(bool calcStats)
    //    { _calcStatsEnable = calcStats; }

    //    /** Requires reversible operators? */
    //    virtual bool requiresRevOp() const
    //    { return false; }

    /** Get _runStatus. */
    RunStatus*
    runStatus()
    {
        return _runStatus;
    }
    //@}

    /** Audit the run result. */
    virtual void audit() = 0;

    /** Get a human-readable status string. */
    virtual utl::String iterationString() const;
    virtual utl::String initString(bool feasible) const;
    virtual utl::String finalString(bool feasible) const;

    void
    setBestScore(Score* score)
    {
        delete _bestScore;
        _bestScore = score;
    }

    void
    setInitScore(Score* score)
    {
        delete _initScore;
        _initScore = score;
    }

    void
    setNewScore(Score* score)
    {
        delete _newScore;
        _newScore = score;
    }

protected:
    void initializeStats();
    void initializeObjectives();
    void initializeOps(StringInd<utl::uint_t>* ind = nullptr);

    /** Choose an operator randomly
        for multiple step move in a direction. */
    Operator* chooseRandomOp() const;

    /** Choose the Operator randomly 
        for single step move in a direction. */
    Operator* chooseRandomStepOp() const;

    /** Choose the Operator with highest success rate. */
    Operator* chooseSuccessOp() const;

    //    virtual void calcStats(const Population& pop) const;
protected:
    // misc
    mutable utl::RandNumGen* _rng;
    utl::uint_t _iteration;
    utl::uint_t _improvementIteration;
    utl::uint_t _minIterations;
    utl::uint_t _maxIterations;
    utl::uint_t _improvementGap;
    StringInd<utl::uint_t>* _ind;
    IndBuilder* _indBuilder;
    IndBuilderContext* _context;
    RunStatus* _runStatus;
    bool _singleStep; //singleStep Mutate

    // iteration status
    Score* _initScore;
    Score* _bestScore;
    Score* _newScore;
    bool _fail;
    bool _accept;
    bool _sameScore;
    bool _newBest;

    // objectives
    std::vector<Objective*> _objectives;

    // operators
    std::vector<Operator*> _ops;
    //    double* _opsCumPct;

    // timer
    /*     utl::OStimer* _timer; */

    //    // statistics
    //    bool _calcStatsEnable;
    //    OperatorStats* _opStats;
private:
    void init();
    void deInit();
};

////////////////////////////////////////////////////////////////////////////////////////////////////

GOP_NS_END;

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
