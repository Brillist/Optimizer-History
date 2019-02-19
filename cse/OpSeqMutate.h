#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <gop/RevOperator.h>
#include <gop/StringInd.h>
#include <cse/ClevorDataSet.h>
#include <cse/JobOp.h>
#include <cse/SchedulingContext.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Change the sequence of the operations for forward scheduler 
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class OpSeqMutate : public gop::RevOperator
{
    UTL_CLASS_DECL(OpSeqMutate, gop::RevOperator);

public:
    virtual void copy(const utl::Object& rhs);

    virtual void initialize(const gop::DataSet* dataSet = nullptr);

    virtual bool execute(gop::Ind* ind = nullptr,
                         gop::IndBuilderContext* context = nullptr,
                         bool singleStep = false);

    virtual void accept();

    virtual void undo();

    // swap between inactive ops or between an inactive op and an active op
    // can also generate different schedules. However, same results can be
    // got by swap between two active ops.
    // For efficiency reason, we only use active job/op for swapping in both
    // JobSeqMutate and OpSeqMutate. Joe, Nov 14, 2006
    virtual uint_t numActiveSwapOps(const jobop_vector_t* opVect);

    virtual JobOp* selectActiveSwapOp(const jobop_vector_t* opVect);

private:
    typedef std::vector<uint_t> uint_vector_t;
    typedef std::set<uint_t> uint_set_t;
    /*    typedef std::vector<JobOp*> jobop_vector_t; */
    typedef std::vector<jobop_vector_t*> jobop_vector_vector_t;
    typedef std::vector<uint_set_t*> uint_vector_set_t;
    typedef std::map<JobOp*, jobop_vector_t*, JobOpIdOrdering> jobop_jobopvector_map_t;

private:
    void init();
    void deInit();

private:
    void setOps(const ClevorDataSet* dataSet);

    jobop_vector_t _ops; //for indexing ops
    jobop_jobopvector_map_t _swapOpsMap;

    gop::StringInd<uint_t>* _moveSchedule;
    uint_t _moveOpIdx;
    uint_t _moveOpSid;
    jobop_vector_t _moveOps;
    uint_vector_t _moveOpIdxs;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;
