#ifndef CSE_CAPMUTATE_H
#define CSE_CAPMUTATE_H

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <gop/RevOperator.h>
#include <gop/StringInd.h>
#include <cls/IntActivity.h>
#include <cse/ClevorDataSet.h>
#include <cse/SchedulingContext.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

class CapMutate : public gop::RevOperator
{
    UTL_CLASS_DECL(CapMutate, gop::RevOperator);

public:
    /**
       Constructor.
       \param p probability
       \param rng PRNG
    */
    CapMutate(double p, lut::rng_t* rng = nullptr)
        : RevOperator("CapMutate", p, rng)
    {
        init();
    }

    virtual void copy(const utl::Object& rhs);

    virtual void initialize(const gop::DataSet* dataSet = nullptr);

    virtual bool execute(gop::Ind* ind = nullptr,
                         gop::IndBuilderContext* context = nullptr,
                         bool singStep = false);

    virtual void accept();

    virtual void undo();

    virtual void setActs(const ClevorDataSet* dataSet);

private:
    typedef std::vector<cls::DiscreteResourceRequirement*> rr_vector_t;
    typedef std::vector<uint_t> uint_vector_t;
    typedef std::vector<cls::IntActivity*> act_vector_t;
    typedef std::vector<uint_vector_t*> uint_vector_vector_t;

private:
    void init();
    void deInit();

private:
    uint_t _numCapChoices;
    act_vector_t _acts;
    uint_vector_t _minCapMultiples;
    uint_vector_t _maxCapMultiples;
    gop::StringInd<uint_t>* _moveSchedule;
    uint_t _moveCapIdx;
    uint_t _moveCap;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
