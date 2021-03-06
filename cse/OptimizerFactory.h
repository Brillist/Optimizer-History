#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <lut/Factory.h>
#include <gop/Optimizer.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Optimizer factory.

   OptimizerFactory can make instances of named Optimizer-derived classes.

   The available optimizers are:

   - \b HillClimber
   - \b SAoptimizer
   - \b MultiStartHC
   - \b MultiStartSA

   \see gop::Optimizer
   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class OptimizerFactory : public lut::Factory<gop::Optimizer>
{
    UTL_CLASS_DECL(OptimizerFactory, lut::Factory<gop::Optimizer>);
    UTL_CLASS_DEFID;

public:
    /**
      Make an instance of a named optimizer class.
      \return newly created object (nullptr if invalid name given)
   */
    virtual gop::Optimizer* make(void* param = nullptr, void* param1 = nullptr) const;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;
