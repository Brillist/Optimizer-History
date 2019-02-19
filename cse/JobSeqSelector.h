#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <cse/ClevorDataSet.h>
#include <cse/DiscreteResource.h>
#include <cse/Scheduler.h>
#include <cse/SchedulingContext.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

class JobSeqSelector : public Scheduler
{
    UTL_CLASS_DECL(JobSeqSelector, Scheduler);

public:
    virtual void copy(const Object& rhs);

    virtual void setStringBase(gop::Operator* op) const;

    virtual uint_t stringSize(const ClevorDataSet& dataSet) const;

    virtual void initialize(const gop::DataSet* dataSet = nullptr, uint_t stringBase = 0);

    virtual void initializeInd(gop::Ind* ind,
                               const gop::DataSet* dataSet,
                               lut::rng_t* rng = nullptr,
                               void* param = nullptr);

    /** Make a schedule. */
    virtual void run(gop::Ind* ind, gop::IndBuilderContext* context) const;

private:
    typedef std::vector<cse::Resource*> res_vector_t;

private:
    void init();
    void
    deInit()
    {
    }

    /** Select Job Sequence. */
    void setSelectedJobSeq(gop::StringInd<uint_t>* ind, SchedulingContext* context) const;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;
