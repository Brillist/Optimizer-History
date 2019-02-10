#include "libcse.h"
#include <libutl/BufferedFDstream.h>
#include <cls/BrkActivity.h>
#include <cse/JobSeqMutate.h>
#include <gop/ConfigEx.h>
#include "ForwardScheduler.h"
#include "JobSeqSelector.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef DEBUG
#define DEBUG_UNIT
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;
CLP_NS_USE;
CLS_NS_USE;
GOP_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(cse::JobSeqSelector);

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

void
JobSeqSelector::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(JobSeqSelector));
    const JobSeqSelector& rcs = (const JobSeqSelector&)rhs;
    Scheduler::copy(rcs);
    //_resources = rcs._resources;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
JobSeqSelector::setStringBase(Operator* op) const
{
    if (dynamic_cast<JobSeqMutate*>(op) != nullptr)
    {
        op->setStringBase(_stringBase);
        return;
    }
    if (_nestedScheduler == nullptr)
    {
        throw ConfigEx();
    }
    _nestedScheduler->setStringBase(op);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

uint_t
JobSeqSelector::stringSize(const ClevorDataSet& dataSet) const
{
    ASSERTD(_nestedScheduler != nullptr);
    uint_t stringSize = dataSet.jobs().size();
    stringSize += _nestedScheduler->stringSize(dataSet);
    return stringSize;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
JobSeqSelector::initialize(const gop::DataSet* p_dataSet, uint_t stringBase)
{
    ASSERTD(_nestedScheduler != nullptr);
    ASSERTD(dynamic_cast<const ClevorDataSet*>(p_dataSet) != nullptr);
    const ClevorDataSet* dataSet = (const ClevorDataSet*)p_dataSet;
    //setSortedJobs(dataSet);
    _stringBase = stringBase;
    _nestedScheduler->initialize(dataSet, _stringBase + dataSet->jobs().size());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
JobSeqSelector::initializeInd(Ind* p_ind, const gop::DataSet* p_dataSet, rng_t* rng, void*)
{
    ASSERTD(dynamic_cast<StringInd<uint_t>*>(p_ind) != nullptr);
    ASSERTD(dynamic_cast<const ClevorDataSet*>(p_dataSet) != nullptr);
    StringInd<uint_t>* ind = (StringInd<uint_t>*)p_ind;
    gop::String<uint_t>& string = ind->string();
    const ClevorDataSet* dataSet = (const ClevorDataSet*)p_dataSet;

    uint_t numJobs = dataSet->jobs().size();
    if (_stringBase == 0)
    {
        string.setSize(stringSize(*dataSet));
    }
    for (uint_t i = 0; i < numJobs; ++i)
    {
        string[_stringBase + i] = uint_t_max;
    }
    _nestedScheduler->initializeInd(ind, dataSet, rng, (void*)size_t_max);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
JobSeqSelector::initializeRandomInd(Ind* p_ind, const gop::DataSet* p_dataSet, rng_t* rng, void*)
{
    utl::cout << "ERROR(JobSeqSelector.cpp): in the incompleted code!" << utl::endl;
    ABORT();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
JobSeqSelector::run(Ind* p_ind, IndBuilderContext* p_context) const
{
    ASSERTD(dynamic_cast<StringInd<uint_t>*>(p_ind) != nullptr);
    ASSERTD(dynamic_cast<SchedulingContext*>(p_context) != nullptr);
    StringInd<uint_t>* ind = (StringInd<uint_t>*)p_ind;
    SchedulingContext* context = (SchedulingContext*)p_context;

    setSelectedJobSeq(ind, context);
    _nestedScheduler->run(ind, context);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
JobSeqSelector::init()
{
    _nestedScheduler = new ForwardScheduler();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
JobSeqSelector::setSelectedJobSeq(StringInd<uint_t>* ind, SchedulingContext* context) const
{
    gop::String<uint_t>& string = ind->string();
    const job_set_id_t& jobs = context->clevorDataSet()->jobs();
    uint_t i = 0;
    job_set_id_t::const_iterator it;
    for (it = jobs.begin(); it != jobs.end(); ++it)
    {
        // processPlanSelector -> jobSeqSelector
        // .. this propagation is a partial one because processPlanMutate can change jobSid,
        // .. but jobSeqMutate cannot change job's active status
        Job* job = *it;
        bool active = job->active();
        uint_t sid = string[_stringBase + i];
        if ((active && job->serialId() != uint_t_max && sid == uint_t_max) ||
            (!active && sid != uint_t_max))
        {
            if (ind->newString() == nullptr)
                ind->createNewString();
            gop::String<uint_t>& newStr = *(ind->newString());
            newStr[_stringBase + i] = job->serialId();
        }
        else
        {
            job->serialId() = sid;
        }
        i++;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;
