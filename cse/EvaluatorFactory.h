#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <lut/Factory.h>
#include <gop/Optimizer.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Schedule evaluator factory.

   EvaluatorFactory can make instances of named IndEvaluator-derived
   classes that can evaluate schedules.

   \see gop::Optimizer
   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class EvaluatorFactory : public lut::Factory<gop::IndEvaluator>
{
    UTL_CLASS_DECL(EvaluatorFactory, lut::Factory<gop::IndEvaluator>);
    UTL_CLASS_DEFID;

public:
    /**
       Make an instance of a named optimizer class.
       \return newly created object (nullptr if invalid name given)
    */
    virtual gop::IndEvaluator* make(void* param = nullptr, void* param1 = nullptr) const;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;
