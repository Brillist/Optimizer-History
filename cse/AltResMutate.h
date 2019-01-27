#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <gop/RevOperator.h>
#include <gop/StringInd.h>
#include <cls/BrkActivity.h>
#include <cse/ClevorDataSet.h>
#include <cse/SchedulingContext.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Change the resource selection for an alternate-resource constraint,
   and randomly select a valid processing-time for the resource.

   \see AltResSelector
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class AltResMutate : public gop::RevOperator
{
    UTL_CLASS_DECL(AltResMutate, gop::RevOperator);

public:
    /**
      Constructor.
      \param p probability
      \param rng PRNG
   */
    AltResMutate(double p, lut::rng_t* rng = nullptr)
        : RevOperator("AltResMutate", p, rng)
    {
        init();
    }

    virtual void copy(const utl::Object& rhs);

    /** Get the number of resource-group requirements. */
    uint_t
    numResGroupReqs() const
    {
        return _resGroupReqs.size();
    }

    virtual void initialize(const gop::DataSet* dataSet = nullptr);

    virtual bool execute(gop::Ind* ind = nullptr,
                         gop::IndBuilderContext* context = nullptr,
                         bool singleStep = false);

    virtual void accept();

    virtual void undo();

private:
    typedef std::vector<cls::DiscreteResourceRequirement*> rgr_vector_t;
    /*    typedef std::vector<cls::ResourceGroupRequirement*> */
    /*       rgr_vector_t; */
private:
    void init();
    void deInit();

    /** Init resource-group requirements array. */
    void setResGroupReqs(const ClevorDataSet* dataSet);

private:
    uint_t _numResGroupReqChoices;
    rgr_vector_t _resGroupReqs;
    gop::StringInd<uint_t>* _moveSchedule;
    uint_t _moveResGroupReqIdx;
    uint_t _moveResIdx;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;
