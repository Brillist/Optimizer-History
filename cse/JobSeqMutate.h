#ifndef CSE_JOBSEQMUTATE_H
#define CSE_JOBSEQMUTATE_H

//////////////////////////////////////////////////////////////////////////////

#include <gop/RevOperator.h>
#include <gop/StringInd.h>
#include <cse/ClevorDataSet.h>
#include <cse/SchedulingContext.h>

//////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////

/**
   Change the job scheduling sequence for forward scheduler 
*/

//////////////////////////////////////////////////////////////////////////////

class JobSeqMutate : public gop::RevOperator
{
    UTL_CLASS_DECL(JobSeqMutate);
public:
    /**
       Constructor.
       \param p probability
       \param rng PRNG
    */
    JobSeqMutate(
        double p,
        utl::RandNumGen* rng = nullptr)
        : RevOperator("JobSeqMutate", p, rng)
    { init(); }

    virtual void copy(const utl::Object& rhs);

    virtual void initialize(const gop::DataSet* dataSet = nullptr);

    virtual bool execute(
        gop::Ind* ind = nullptr,
        gop::IndBuilderContext* context = nullptr,
        bool singleStep = false);
   
    virtual void accept();
   
    virtual void undo();

   // swap between inactive jobs or between an inactive job and an active job
   // can also generate different schedules. However, same results can be 
   // got by swap between two active jobs.
   // For efficiency reason, we only use active job/op for swapping in both
   // JobSeqMutate and OpSeqMutate. Joe, Nov 14, 2006
    virtual utl::uint_t numActiveSwapJobs(
        const job_vector_t& swapJobs);

    virtual Job* selectActiveSwapJob(const job_vector_t& swapJobs);
private:
    typedef std::vector<utl::uint_t> uint_vector_t;
    typedef std::vector<uint_vector_t*> uint_vector_vector_t;
    typedef std::vector<job_vector_t*> job_vector_vector_t;
    typedef std::set<Job*> job_set_t;
private:
    void init();
    void deInit();

    void setJobs(const ClevorDataSet* dataSet);

    job_vector_t _jobs;
    job_vector_vector_t _swapJobs;

   gop::StringInd<utl::uint_t>* _moveSchedule;
   utl::uint_t _moveJobIdx; //for successRate
   utl::uint_t _moveJobSid; //necessary
   job_vector_t _moveJobs; //necessary
   uint_vector_t _moveJobIdxs; //necessary
};

//////////////////////////////////////////////////////////////////////////////

CSE_NS_END;

//////////////////////////////////////////////////////////////////////////////

#endif
