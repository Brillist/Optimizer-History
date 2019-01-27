#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <gop/RevOperator.h>
#include <gop/StringInd.h>
#include <cls/BrkActivity.h>
#include <cse/ClevorDataSet.h>
#include <cse/SchedulingContext.h>
#include <cse/JobGroup.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Change the alternative job selection for a mnaufacture item,
   and randomly select an alternative process plan for the item.

   \author Joe Zhou
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class AltJobMutate : public gop::RevOperator
{
    UTL_CLASS_DECL(AltJobMutate, gop::RevOperator);

public:
    /**
       Constructor.
       \param p probability
       \param rng PRNG
    */
    AltJobMutate(double p, lut::rng_t* rng = nullptr)
        : RevOperator("AltJobMutate", p, rng)
    {
        init();
    }

    virtual void copy(const utl::Object& rhs);

    virtual void initialize(const gop::DataSet* dataSet = nullptr);

    virtual bool execute(gop::Ind* ind = nullptr,
                         gop::IndBuilderContext* context = nullptr,
                         bool singleStep = false);

    virtual void accept();

    virtual void undo();

    virtual void setAltJobGroups(const ClevorDataSet* dataSet);

private:
    typedef std::vector<uint_t> uint_vector_t;
    typedef std::vector<uint_vector_t*> uint_vector_vector_t;

private:
    void init();
    void deInit();

private:
    uint_t _numPlanChoices;
    jobgroup_vector_t _jobGroups;
    gop::StringInd<uint_t>* _moveSchedule;
    uint_t _moveGroupIdx;
    uint_t _moveJobIdx;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;
