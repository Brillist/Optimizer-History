#ifndef GOP_OBJECTIVE_H
#define GOP_OBJECTIVE_H

//////////////////////////////////////////////////////////////////////////////

#include <gop/IndBuilderContext.h>
#include <gop/IndEvaluator.h>
#include <gop/Score.h>

//////////////////////////////////////////////////////////////////////////////

GOP_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////

/// minimization or maximization goal
enum minmax_goal_t
{
   mmg_minimize,  /**< minimize score */
   mmg_maximize,  /**< maximize score */
   mmg_undefined  /**< goal is undefined */
};

//////////////////////////////////////////////////////////////////////////////

/**
   Optimization objective.

   An optimization objective is represented as the goal of minimizing or
   maximizing the value of a particular metric.

   \author Adam McKee
*/

//////////////////////////////////////////////////////////////////////////////

class Objective : public utl::Object
{
    UTL_CLASS_DECL(Objective);
public:
    /** Constructor. */
    Objective(IndEvaluator* indEvaluator, minmax_goal_t mmg);
   
    /** Copy another instance. */
    virtual void copy(const utl::Object& rhs);

    virtual void serialize(
        utl::Stream& stream,
        utl::uint_t io,
        utl::uint_t mode = utl::ser_default);

    /** Get the name. */
    virtual std::string name() const
    { ASSERTD(_indEvaluator != nullptr); return _indEvaluator->name(); }

    /** Get the evaluator. */
    IndEvaluator* indEvaluator() const
    { return _indEvaluator; }

    /** Get the goal. */
    minmax_goal_t goal() const
    { return _mmg; }

    /** Minimizing? */
    bool minimize() const
    { return (_mmg == mmg_minimize); }

    /** Maximizing? */
    bool maximize() const
    { return (_mmg == mmg_maximize); }

    /** Get the best score. */
    Score* getBestScore() const
    { ASSERTD(_bestScore != nullptr); return _bestScore; }

/*     double getBestScoreValue() const */
/*     { return _bestScore->getValue(); } */


    /** Set the best score. */
    void setBestScore(Score* bestScore);

    /** Get a best score component. */
    int getBestScoreComponent(
        const std::string& componentName) const;

    double worstPossibleScoreValue() const;

    /** Get the best possible score. */
    Score* bestPossibleScore() const;

    /** Get the worst possible score. */
    Score* worstPossibleScore() const;

    /** Evaluate the given individual. */
    Score* eval(IndBuilderContext* context) const;

    /** Get the fail score from the given individual. */
/*     double failScore(IndBuilderContext* context) const; */

    /** Get the best fail score from the given individual. */
/*     double bestFailScore(IndBuilderContext* context) const; */

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

//////////////////////////////////////////////////////////////////////////////

typedef std::vector<Objective*> objective_vector_t;

//////////////////////////////////////////////////////////////////////////////

GOP_NS_END;

//////////////////////////////////////////////////////////////////////////////

#endif
