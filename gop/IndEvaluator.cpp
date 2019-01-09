#include "libgop.h"
#include "IndEvaluator.h"

//////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;

//////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL_ABC(gop::IndEvaluator, utl::Object);

//////////////////////////////////////////////////////////////////////////////

GOP_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////

int
IndEvaluator::getComponentScore(const std::string& componentName) const
{
    string_int_map_t::const_iterator it =
        _componentScores.find(componentName);
    if (it == _componentScores.end())
    {
        return int_t_max;
    }
    return (*it).second;
}

//////////////////////////////////////////////////////////////////////////////

void
IndEvaluator::setComponentScore(
    const std::string& componentName,
    int score) const
{
    string_int_map_t::iterator it =
        _componentScores.find(componentName);
    if (it == _componentScores.end())
    {
        _componentScores.insert(
            string_int_map_t::value_type(componentName, score));
        return;
    }
    (*it).second = score;
}

//////////////////////////////////////////////////////////////////////////////

GOP_NS_END;
