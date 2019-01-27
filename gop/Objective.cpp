#include "libgop.h"
#include "Objective.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(gop::Objective);

////////////////////////////////////////////////////////////////////////////////////////////////////

GOP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

Objective::Objective(IndEvaluator* indEvaluator, minmax_goal_t mmg)
{
    _indEvaluator = indEvaluator;
    _mmg = mmg;
    setBestScore(worstPossibleScore());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Objective::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(Objective));
    const Objective& obj = (const Objective&)rhs;
    delete _indEvaluator;
    _indEvaluator = obj._indEvaluator->clone();
    _mmg = obj._mmg;
    setBestScore(utl::clone(obj._bestScore));
    _bestScoreComponents = obj._bestScoreComponents;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Objective::serialize(Stream& stream, uint_t io, uint_t)
{
    utl::serialize(_indEvaluator, stream, io, ser_default);
    utl::serialize((uint_t&)_mmg, stream, io);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Objective::setBestScore(Score* bestScore)
{
    delete _bestScore;
    _bestScore = bestScore;
    ASSERTD(_indEvaluator != nullptr);
    _bestScoreComponents = _indEvaluator->componentScores();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int
Objective::getBestScoreComponent(const std::string& componentName) const
{
    string_int_map_t::const_iterator it = _bestScoreComponents.find(componentName);
    if (it == _bestScoreComponents.end())
    {
        return int_t_max;
    }
    return (*it).second;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Score*
Objective::bestPossibleScore() const
{
    Score* score = new Score();
    score->setValue(-worstPossibleScoreValue());
    return score;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Score*
Objective::worstPossibleScore() const
{
    Score* score = new Score();
    score->setValue(this->worstPossibleScoreValue());
    return score;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

double
Objective::worstPossibleScoreValue() const
{
    return (_mmg == mmg_minimize) ? utl::double_t_max : utl::double_t_min;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Score*
Objective::eval(IndBuilderContext* context) const
{
    ASSERTD(_indEvaluator != nullptr);
    return _indEvaluator->eval(context);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int
Objective::compare(Score* lhs, Score* rhs) const
{
    ASSERTD(lhs->getType() != score_undefined);
    ASSERTD(rhs->getType() != score_undefined);
    if (lhs->getType() == rhs->getType())
        return compare(lhs->getValue(), rhs->getValue());
    if (lhs->getType() > rhs->getType())
    {
        if (_mmg == mmg_minimize)
        {
            return 1;
        }
        else
        {
            return -1;
        }
    }
    else
    {
        if (_mmg == mmg_minimize)
        {
            return -1;
        }
        else
        {
            return 1;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
int
Objective::compare(double lhs, double rhs) const
{
    if (_mmg == mmg_minimize)
    {
        return lut::compare(rhs, lhs);
    }
    else // (_mmg == mmg_maximize)
    {
        return lut::compare(lhs, rhs);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

double
Objective::scoreDiff(Score* lhs, Score* rhs) const
{
    ASSERTD(lhs->getType() != score_undefined);
    ASSERTD(rhs->getType() != score_undefined);
    if (lhs->getType() == rhs->getType())
        return scoreDiff(lhs->getValue(), rhs->getValue());
    if (lhs->getType() > rhs->getType())
    {
        if (_mmg == mmg_minimize)
        {
            return utl::double_t_max;
        }
        else
        {
            return -utl::double_t_max;
        }
    }
    else
    {
        if (_mmg == mmg_minimize)
        {
            return -utl::double_t_max;
        }
        else
        {
            return utl::double_t_max;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

double
Objective::scoreDiff(double lhs, double rhs) const
{
    if (_mmg == mmg_minimize)
    {
        return rhs - lhs;
    }
    else // (_mmg == mmg_maximize)
    {
        return lhs - rhs;
        ;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Objective::init()
{
    _indEvaluator = nullptr;
    _mmg = mmg_undefined;
    _bestScore = nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Objective::deInit()
{
    delete _indEvaluator;
    delete _bestScore;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

GOP_NS_END;
