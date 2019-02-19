#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <gop/Ind.h>
#include <gop/Operator.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

GOP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Individual construction strategy (abstract).

   Each individual (Ind) contains a plan for its own construction.  These instructions are
   interpreted by a subclass of IndBuilder that reads them in order to construct the individual.

   \ingroup gop
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class IndBuilder : public utl::Object
{
    UTL_CLASS_DECL_ABC(IndBuilder, utl::Object);
    UTL_CLASS_DEFID;

public:
    /** Set the string base of an operator. */
    virtual void setStringBase(Operator* op) const = 0;

    /**
       Initialize.
       \param dataSet active DataSet
       \param stringBase base index within the string (gop::String)
    */
    virtual void
    initialize(const DataSet* dataSet = nullptr, uint_t stringBase = 0)
    {
    }

    /**
       Initialize the given individual.
       \param ind individual (Ind) to initialize
       \param dataSet active DataSet
       \param rng (optional) PRNG for the run
       \param param (optional) implementation-specific initialization parameter
    */
    virtual void
    initializeInd(Ind* ind,
                  const DataSet* dataSet,
                  lut::rng_t* rng = nullptr,
                  void* param = nullptr)
    {
    }

    /**
       Build the individual.
       \param ind individual to be built
       \param context construction context
    */
    virtual void run(Ind* ind, IndBuilderContext* context) const = 0;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

GOP_NS_END;
