#include "libgop.h"
#include "WeightedEvaluator.h"

//////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;

//////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(gop::WeightedEvaluator, gop::IndEvaluator);

//////////////////////////////////////////////////////////////////////////////

GOP_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////

void
WeightedEvaluator::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(WeightedEvaluator));
    const WeightedEvaluator& we = (const WeightedEvaluator&)rhs;
    IndEvaluator::copy(we);
    deleteCont(_evaluators);
    uint_t numEvaluators = we._evaluators.size();
    for (uint_t i = 0; i < numEvaluators; ++i)
    {
        IndEvaluator* evaluator = we._evaluators[i];
        evaluator = evaluator->clone();
        _evaluators.push_back(evaluator);
    }
    _weights = we._weights;
}

//////////////////////////////////////////////////////////////////////////////

void
WeightedEvaluator::add(IndEvaluator* evaluator, double weight)
{
    _evaluators.push_back(evaluator);
    _weights.push_back(weight);
}

//////////////////////////////////////////////////////////////////////////////

Score*
WeightedEvaluator::eval(const IndBuilderContext* context) const
{
//     double score = 0.0;
//     uint_t numEvaluators = _evaluators.size();
//     for (uint_t i = 0; i < numEvaluators; ++i)
//     {
//         IndEvaluator* evaluator = _evaluators[i];
//         double weight = _weights[i];
//         double thisScore = evaluator->eval(context);
//         score += thisScore * weight;
//     }
//     return score;
    ABORT();
    return nullptr;
}

//////////////////////////////////////////////////////////////////////////////

void
WeightedEvaluator::init()
{
}

//////////////////////////////////////////////////////////////////////////////

void
WeightedEvaluator::deInit()
{
    deleteCont(_evaluators);
}

//////////////////////////////////////////////////////////////////////////////

GOP_NS_END;
