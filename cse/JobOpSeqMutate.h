#ifndef CSE_JOBOPSEQMUTATE_H
#define CSE_JOBOPSEQMUTATE_H

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
    UTL_CLASS_DECL(JobOpSeqMutate);
    //public:
    //   typedef std::set<utl::uint_t> uint_set_t;
public:
    /**
      Constructor.
      \param p probability
      \param rng PRNG
   */
    JobOpSeqMutate(double p, utl::RandNumGen* rng = nullptr)
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
    typedef std::vector<utl::uint_t> uint_vector_t;
    typedef std::set<utl::uint_t> uint_set_t;
    typedef std::vector<JobOp*> jobop_vector_t;
    typedef std::vector<jobop_vector_t*> jobop_vector_vector_t;
    typedef std::vector<uint_set_t*> uint_vector_set_t;
    //typedef std::map<JobOp*, uint_set_t*> jobop_uintset_map_t;
private:
    void init();
    void deInit();

    void setJobOps(const ClevorDataSet* dataSet);

    job_vector_t _jobs;
    jobop_vector_t _ops;

    uint_vector_t _jobStrPositions;
    uint_vector_t _jobNumChoices;
    jobop_vector_vector_t _swapOps;

    gop::StringInd<utl::uint_t>* _moveSchedule;
    utl::uint_t _moveJobIdx;
    utl::uint_t _moveOpSid;
    jobop_vector_t _moveOps;
    uint_vector_t _moveOpIdxs;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
