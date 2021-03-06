#include "libgop.h"
#include "IndEvaluator.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL_ABC(gop::IndEvaluator);

////////////////////////////////////////////////////////////////////////////////////////////////////

GOP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

int
IndEvaluator::getComponentScore(const std::string& componentName) const
{
    auto it = _componentScores.find(componentName);
    if (it == _componentScores.end())
    {
        return int_t_max;
    }
    else
    {
        return (*it).second;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
IndEvaluator::setComponentScore(const std::string& componentName, int score) const
{
    auto it = _componentScores.find(componentName);
    if (it == _componentScores.end())
    {
        _componentScores.insert(string_int_map_t::value_type(componentName, score));
    }
    else
    {
        (*it).second = score;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

GOP_NS_END;
