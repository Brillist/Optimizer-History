#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <clp/CycleGroup.h>
#include <gop/RevOperator.h>
#include <gop/StringInd.h>
#include <cse/ClevorDataSet.h>
#include <cse/JobOp.h>
#include <cse/SchedulingContext.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Change the sequence of the operations in a job for forward scheduler 
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class JobOpSeqMutate : public gop::RevOperator
{
    UTL_CLASS_DECL(JobOpSeqMutate, gop::RevOperator);
public:
    /**
      Constructor.
      \param p probability
      \param rng PRNG
   */
    JobOpSeqMutate(double p, lut::rng_t* rng = nullptr)
        : RevOperator("JobOpSeqMutate", p, rng)
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

private:
    typedef std::vector<JobOp*> jobop_vector_t;
    typedef std::vector<jobop_vector_t*> jobop_vector_vector_t;
    typedef std::vector<lut::uint_set_t*> uint_vector_set_t;
private:
    void init();
    void deInit();

    void setJobOps(const ClevorDataSet* dataSet);

    job_vector_t _jobs;
    jobop_vector_t _ops;

    // store each job's string positionnumber of choices, and swap-ops
    uint_vector_t _jobStrPositions; // each job's base index in the string
    uint_vector_t _jobNumChoices;   // each job's number of choices
    jobop_vector_vector_t _swapOps; // each op's swap-ops

    gop::StringInd<uint_t>* _moveSchedule;
    uint_t _moveJobIdx;
    uint_t _moveOpSid;
    jobop_vector_t _moveOps;
    uint_vector_t _moveOpIdxs;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;
