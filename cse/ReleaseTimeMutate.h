#ifndef CSE_RELEASETIMEMUTATE_H
#define CSE_RELEASETIMEMUTATE_H

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <gop/RevOperator.h>
#include <gop/StringInd.h>
/* #include <cls/BrkActivity.h> */
#include <cse/ClevorDataSet.h>
#include <cse/SchedulingContext.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Change the release time selection for a workorder,
   and randomly increase or decrease the release time by a _changeStep 
   (day by default).

   \author Joe Zhou

   \date created on March 4, 2007
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class ReleaseTimeMutate : public gop::RevOperator
{
    UTL_CLASS_DECL(ReleaseTimeMutate, gop::RevOperator);

public:
    /**
       Constructor.
       \param p probability
       \param rng PRNG
    */
    ReleaseTimeMutate(double p, lut::rng_t* rng = nullptr)
        : RevOperator("ReleaseTimeMutate", p, rng)
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

    virtual void setActs(const ClevorDataSet* dataSet);

private:
    void init();
    void deInit();

private:
    uint_t _numRlsTimeChoices;
    cls::act_vect_t _acts; // root op's act only
    lut::uint_vect_t _minRlsTimes;
    uint_t _changeStep;

    gop::StringInd<uint_t>* _moveSchedule;
    uint_t _moveActIdx;
    uint_t _moveActRlsTime;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
