#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

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

   Optimization strategies are implemented as specializations of Optimizer.

   \ingroup gop
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class Optimizer : public utl::Object
{
    UTL_CLASS_DECL_ABC(Optimizer, utl::Object);

public:
    /// \name Run Control
    //@{
    /** Is the run complete? */
    bool complete() const;

    /** Stop the run. */
    void stop();

    /**
       Initialize.
       \param config configuration
    */
    virtual void initialize(const OptimizerConfiguration* config);

    /**
       Iterate until the run is complete.
       \return true if new best result or run complete, false otherwise
       \see complete
    */
    virtual bool run() = 0;

    /** Iteration run: run just one iteration. */
    virtual bool iterationRun(Operator* op = nullptr, bool audit = false);

    virtual void updateRunStatus(bool complete);
    //@}

    /// \name Accessors (const)
    //@{
    /** Get the random number generator. */
    lut::rng_t*
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
    uint_t
    iteration() const
    {
        return _iteration;
    }

    /** Get the last improvement iteration number. */
    uint_t
    improvementIteration() const
    {
        return _improvementIteration;
    }

    /** Get the maximum number of iterations. */
    uint_t
    maxIterations() const
    {
        return _maxIterations;
    }

    /** Get the optimization goal for the given objective. */
    minmax_goal_t
    goal(uint_t idx = 0) const
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

    /** Get the best Score for the given objective. */
    Score* bestScore(uint_t objectiveIdx = 0) const;

    /** Get the best Score for the given objective. */
    Score* bestScore(const std::string& objectiveName) const;

    /** Get a named component of the best score. */
    int bestScoreComponent(const std::string& objectiveName,
                           const std::string& componentName) const;

    /** Get the audit text for the best score. */
    const std::string& bestScoreAudit() const;

    /** Get the number of operators. */
    uint_t
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

    /** Get _runStatus. */
    const RunStatus*
    runStatus() const
    {
        return _runStatus;
    }
    //@}

    /** Audit the run result. */
    virtual void audit() = 0;

protected:
    void initializeObjectives();
    void initializeOps(StringInd<uint_t>* ind = nullptr);

    /** Choose an operator randomly for multiple step move in a direction. */
    Operator* chooseRandomOp() const;

    /** Choose the Operator randomly for single step move in a direction. */
    Operator* chooseRandomStepOp() const;

    /** Choose the Operator with highest success rate. */
    Operator* chooseSuccessOp() const;

    // for debug output
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
    // misc
    mutable lut::rng_t* _rng;
    uint_t _iteration;
    uint_t _improvementIteration;
    uint_t _minIterations;
    uint_t _maxIterations;
    uint_t _improvementGap;
    StringInd<uint_t>* _ind;
    IndBuilder* _indBuilder;
    IndBuilderContext* _context;
    RunStatus* _runStatus;
    bool _singleStep;

    // iteration status
    Score* _initScore;
    Score* _bestScore;
    Score* _newScore;
    bool _fail;
    bool _accept;
    bool _sameScore;
    bool _newBest;

    // objectives and operators
    objective_vector_t _objectives;
    op_vector_t _ops;

private:
    void init();
    void deInit();
};

////////////////////////////////////////////////////////////////////////////////////////////////////

GOP_NS_END;
