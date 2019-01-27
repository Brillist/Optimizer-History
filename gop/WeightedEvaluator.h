#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <gop/IndEvaluator.h>
#include <gop/Score.h>
////////////////////////////////////////////////////////////////////////////////////////////////////

GOP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Weighted evaluator.

   A weighted evaluator combines two or more evaluators to form a single
   metric, using a weighted average of the scores.

   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class WeightedEvaluator : public IndEvaluator
{
    UTL_CLASS_DECL(WeightedEvaluator, IndEvaluator);

public:
    /** Copy another instance. */
    virtual void copy(const utl::Object& rhs);

    /** Add an evaluator. */
    void add(IndEvaluator* evaluator, double weight);

    /** Evaluate a constructed individual. */
    virtual Score* eval(const IndBuilderContext* context) const;

    /** 
        get the best fail score which is the boundary score 
        between fail and success.
    */
    /*     virtual double bestFailScore(const IndBuilderContext* context) const */
    /*     { return 0.0;} */

    /** get the fail score from a failed run. */
    /*     virtual double failScore(const IndBuilderContext* context) const */
    /*     { return 0.0; } */
private:
    typedef std::vector<IndEvaluator*> evaluator_vector_t;
    typedef std::vector<double> double_vector_t;

private:
    void init();
    void deInit();

private:
    evaluator_vector_t _evaluators;
    double_vector_t _weights;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

GOP_NS_END;
