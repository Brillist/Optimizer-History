#include "libgop.h"
#include "Ind.h"
#include "Objective.h"
#include "Population.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL_ABC(gop::Ind);
UTL_CLASS_IMPL_ABC(gop::IndOrdering);
UTL_CLASS_IMPL(gop::IndScoreOrdering);
UTL_CLASS_IMPL(gop::IndFitnessOrdering);

////////////////////////////////////////////////////////////////////////////////////////////////////

GOP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Ind ////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void
Ind::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(Ind));
    const Ind& ind = (const Ind&)rhs;
    _pop = nullptr;
    _scores = ind._scores;
    _fitness = ind._fitness;
    _parentScores = ind._parentScores;
    _opIdx = ind._opIdx;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

utl::String
Ind::toString() const
{
    std::ostringstream ss;
    if ((getScore() >= 0.0) || (getFitness() >= 0.0))
    {
        ss << "score = " << getScore();
        if (getFitness() >= 0.0)
        {
            ss << ", fitness = " << getFitness();
        }
    }
    return ss.str().c_str();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool
Ind::dominates(const Ind* rhs, const objective_vector_t& objectives) const
{
    // self dominates rhs if all self's scores are better
    uint_t i;
    uint_t numScores = _scores.size();
    for (i = 0; i < numScores; ++i)
    {
        auto objective = objectives[i];
        auto lhsScore = _scores[i];
        auto rhsScore = rhs->_scores[i];
        if (objective->compare(lhsScore, rhsScore) <= 0)
        {
            return false;
        }
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Ind::setScore(uint_t idx, double score)
{
    if (_pop != nullptr)
    {
        _pop->onChangeScore();
    }
    if (idx >= _scores.size())
    {
        _scores.resize(idx + 1);
    }
    _scores[idx] = score;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Ind::setFitness(double fitness)
{
    if (_pop != nullptr)
    {
        _pop->onChangeFitness();
    }
    _fitness = fitness;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Ind::init()
{
    _pop = nullptr;
    _scores.resize(1);
    _scores[0] = double_t_max;
    _fitness = -1.0;
    _parentScores.resize(1);
    _parentScores[0] = double_t_max;
    _opIdx = uint_t_max;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Ind::deInit()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// IndScoreOrdering ///////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

int
IndScoreOrdering::compare(const Ind* lhs, const Ind* rhs) const
{
    if (inverted())
    {
        return lut::compare(rhs->getScore(_objectiveIdx), lhs->getScore(_objectiveIdx));
    }
    else
    {
        return lut::compare(lhs->getScore(_objectiveIdx), rhs->getScore(_objectiveIdx));
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// IndFitnessOrdering /////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

int
IndFitnessOrdering::compare(const Ind* lhs, const Ind* rhs) const
{
    if (inverted())
    {
        return lut::compare(rhs->getFitness(), lhs->getFitness());
    }
    else
    {
        return lut::compare(lhs->getFitness(), rhs->getFitness());
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

GOP_NS_END;
