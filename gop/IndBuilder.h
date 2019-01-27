#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <gop/Ind.h>
#include <gop/Operator.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

GOP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Individual construction strategy (abstract).

   Each individual (Ind) contains a plan for its own construction.  These instructions are meant
   to be interpreted by a subclass of IndBuilder, which knows how to read them in order to construct
   the individual.

   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class IndBuilder : public utl::Object
{
    UTL_CLASS_DECL_ABC(IndBuilder, utl::Object);
    UTL_CLASS_DEFID;

public:
    /** Set the string base of an operator. */
    virtual void setStringBase(Operator* op) const = 0;

    /** Initialize self. */
    virtual void
    initialize(const DataSet* dataSet = nullptr, uint_t stringBase = 0)
    {
    }

    /** Initialize the given individual. */
    virtual void
    initializeInd(Ind* ind,
                  const DataSet* dataSet,
                  lut::rng_t* rng = nullptr,
                  void* param = nullptr)
    {
    }

    //Joe's debug code
    virtual void
    initializeRandomInd(Ind* ind,
                        const DataSet* dataSet,
                        lut::rng_t* rng = nullptr,
                        void* param = nullptr)
    {
    }

    /**
       Build the individual.
       \param ind individual to be built
       \param param meaning is implementation specific
    */
    virtual void run(Ind* ind, IndBuilderContext* context) const = 0;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

GOP_NS_END;
