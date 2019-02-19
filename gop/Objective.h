#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <gop/IndBuilderContext.h>
#include <gop/IndEvaluator.h>
#include <gop/Score.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

GOP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Minimization or maximization goal.

   \ingroup gop
*/
enum minmax_goal_t
{
    mmg_minimize, /**< minimize score */
    mmg_maximize, /**< maximize score */
    mmg_undefined
};

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Optimization objective.

   An optimization objective is represented as the goal of minimizing or maximizing the value of
   a particular metric.

   \ingroup gop
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class Objective : public utl::Object
{
    UTL_CLASS_DECL(Objective, utl::Object);

public:
    /**
       Constructor.
       \param indEvaluator individual score evaluator
       \param mmg minimize or maximize the evaluation score?
    */
    Objective(IndEvaluator* indEvaluator, minmax_goal_t mmg);

    virtual void copy(const utl::Object& rhs);

    virtual void serialize(utl::Stream& stream, uint_t io, uint_t mode = utl::ser_default);

    /** Get this objective's name. */
    virtual std::string
    name() const
    {
        ASSERTD(_indEvaluator != nullptr);
        return _indEvaluator->name();
    }

    /** Get the score evaluator. */
    IndEvaluator*
    indEvaluator() const
    {
        return _indEvaluator;
    }

    /** Get the goal of this objective (i.e. minimization or maximization of score). */
    minmax_goal_t
    goal() const
    {
        return _mmg;
    }

    /** Minimizing the score? */
    bool
    minimize() const
    {
        return (_mmg == mmg_minimize);
    }

    /** Maximizing the score? */
    bool
    maximize() const
    {
        return (_mmg == mmg_maximize);
    }

    /** Get the best score. */
    Score*
    getBestScore() const
    {
        ASSERTD(_bestScore != nullptr);
        return _bestScore;
    }

    /** Set the best score. */
    void setBestScore(Score* bestScore);

    /** Get a best score component. */
    int getBestScoreComponent(const std::string& componentName) const;

    /** Get the best possible score (as a Score object). */
    Score* bestPossibleScore() const;

    /** Get the worst possible score (as a Score object). */
    Score* worstPossibleScore() const;

    /** Get the worst possible score (as a numerical value). */
    double worstPossibleScoreValue() const;

    /** Evaluate the given individual. */
    Score* eval(IndBuilderContext* context) const;

    /**
       Compare two scores.
       \return -1 if lhs is worse than rhs, 
                0 if lhs == rhs, 
                1 0 if lhs is better than rhs
    */
    int compare(double lhs, double rhs) const;
    int compare(Score* lhs, Score* rhs) const;

    /** 
        get the score difference between two scores
        \return < 0 if lhs is worse than rhs,
                  0 if lhs == rhs,
                > 0 if lhs is better than rhs.
    */
    double scoreDiff(Score* lhs, Score* rhs) const;
    double scoreDiff(double lhs, double rhs) const;

private:
    typedef std::map<std::string, int> string_int_map_t;

private:
    void init();
    void deInit();

    IndEvaluator* _indEvaluator;
    minmax_goal_t _mmg;
    Score* _bestScore;
    string_int_map_t _bestScoreComponents;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   A \c std::vector of Objective pointers.
   \ingroup gop
*/
using objective_vector_t = std::vector<Objective*>;

////////////////////////////////////////////////////////////////////////////////////////////////////

GOP_NS_END;
