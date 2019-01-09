#include "libcse.h"
#include <gop/RandomWalk.h>
#include <gop/HillClimber.h>
#include <gop/SAoptimizer.h>
#include <gop/ID_SAoptimizer.h>
#include <gop/AR_SAoptimizer.h>
#include <gop/MultistartHC.h>
#include <gop/MultistartSA.h>
#include "OptimizerFactory.h"

//////////////////////////////////////////////////////////////////////////////

LUT_NS_USE;
GOP_NS_USE;

//////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(cse::OptimizerFactory, lut::Factory<gop::Optimizer>);

//////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////

Optimizer*
OptimizerFactory::make(void* param, void*) const
{
   ASSERTD(param != nullptr);
   Optimizer* optimizer = nullptr;
   std::string name((const char*)param);
   if (name == "RandomWalk")
   {
      optimizer = new RandomWalk();
   }
   else if (name == "HillClimber")
   {
      optimizer = new HillClimber();
   }
   else if (name == "SAoptimizer")
   {
      optimizer = new SAoptimizer();
   }
   else if (name == "ID_SAoptimizer")
   {
      optimizer = new ID_SAoptimizer();
   }
   else if (name == "AR_SAoptimizer")
   {
      optimizer = new AR_SAoptimizer();
   }
   else if (name == "MultistartHC")
   {
      optimizer = new MultistartHC();
   }
   else if (name == "MultistartSA")
   {
      optimizer = new MultistartSA();
   }
   return optimizer;
}

//////////////////////////////////////////////////////////////////////////////

CSE_NS_END;
