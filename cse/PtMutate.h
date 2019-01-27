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

class PtMutate : public gop::RevOperator
{
    UTL_CLASS_DECL(PtMutate, gop::RevOperator);

public:
    /**
       Constructor.
       \param p probability
       \param rng PRNG
    */
    PtMutate(double p, lut::rng_t* rng = nullptr)
        : RevOperator("PtMutate", p, rng)
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

    virtual void setActPtExps(const ClevorDataSet* dataSet);

private:
    typedef std::vector<cls::DiscreteResourceRequirement*> rr_vector_t;
    typedef std::vector<uint_t> uint_vector_t;
    typedef std::vector<cls::PtActivity*> act_vector_t;
    typedef std::vector<uint_vector_t*> uint_vector_vector_t;

private:
    void init();
    void deInit();

private:
    uint_t _numPtChoices;
    act_vector_t _acts;
    uint_vector_vector_t _ptExps;
    gop::StringInd<uint_t>* _moveSchedule;
    uint_t _movePtIdx;
    uint_t _movePt;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;
