#include "libcse.h"
#include "EvaluatorFactory.h"
#include "MakespanEvaluator.h"
#include "TotalCostEvaluator.h"

//////////////////////////////////////////////////////////////////////////////

LUT_NS_USE;
GOP_NS_USE;

//////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(cse::EvaluatorFactory, lut::Factory<gop::IndEvaluator>);

//////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////

IndEvaluator*
EvaluatorFactory::make(void* param, void*) const
{
    ASSERTD(param != nullptr);
    IndEvaluator* evaluator = nullptr;
    std::string name((const char*)param);
    if (name == "MakespanEvaluator")
    {
        evaluator = new MakespanEvaluator();
    }
    else if (name == "TotalCostEvaluator")
    {
        evaluator = new TotalCostEvaluator();
    }
    return evaluator;
}

//////////////////////////////////////////////////////////////////////////////

CSE_NS_END;
