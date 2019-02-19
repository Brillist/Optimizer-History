#include "libcse.h"
#include <libutl/BufferedFDstream.h>
#include <gop/ConfigEx.h>
#include <cse/ForwardScheduler.h>
#include <cse/AltJobMutate.h>
#include "AltJobSelector.h"

#include <clp/BoundPropagator.h>

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

UTL_CLASS_IMPL(cse::AltJobSelector);

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

void
AltJobSelector::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(AltJobSelector));
    const AltJobSelector& pps = (const AltJobSelector&)rhs;
    Scheduler::copy(pps);
    _jobGroups = pps._jobGroups;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
AltJobSelector::setStringBase(Operator* op) const
{
    if (dynamic_cast<AltJobMutate*>(op) != nullptr)
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
AltJobSelector::stringSize(const ClevorDataSet& dataSet) const
{
    ASSERTD(_nestedScheduler != nullptr);
    uint_t stringSize = _jobGroups.size();
    stringSize += _nestedScheduler->stringSize(dataSet);
    return stringSize;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
AltJobSelector::initialize(const gop::DataSet* p_dataSet, uint_t stringBase)
{
    ASSERTD(_nestedScheduler != nullptr);
    ASSERTD(dynamic_cast<const ClevorDataSet*>(p_dataSet) != nullptr);
    const ClevorDataSet* dataSet = (const ClevorDataSet*)p_dataSet;

    setAltJobGroups(dataSet);
    _stringBase = stringBase;
    _nestedScheduler->initialize(dataSet, _stringBase + _jobGroups.size());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
AltJobSelector::initializeInd(Ind* p_ind, const gop::DataSet* p_dataSet, lut::rng_t* rng, void*)
{
    ASSERTD(dynamic_cast<StringInd<uint_t>*>(p_ind) != nullptr);
    StringInd<uint_t>* ind = (StringInd<uint_t>*)p_ind;
    gop::String<uint_t>& string = ind->string();

    ASSERTD(dynamic_cast<const ClevorDataSet*>(p_dataSet) != nullptr);
    const ClevorDataSet* dataSet = (const ClevorDataSet*)p_dataSet;

    uint_t numGroups = _jobGroups.size();
    if (_stringBase == 0)
    {
        string.setSize(stringSize(*dataSet));
    }
    for (uint_t i = 0; i < numGroups; ++i)
    {
        string[_stringBase + i] = 0;
    }

    _nestedScheduler->initializeInd(ind, dataSet, rng, (void*)size_t_max);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
AltJobSelector::run(Ind* p_ind, IndBuilderContext* p_context) const
{
    ASSERTD(_nestedScheduler != nullptr);
    ASSERTD(dynamic_cast<StringInd<uint_t>*>(p_ind) != nullptr);
    StringInd<uint_t>* ind = (StringInd<uint_t>*)p_ind;
    ASSERTD(dynamic_cast<SchedulingContext*>(p_context) != nullptr);
    SchedulingContext* context = (SchedulingContext*)p_context;

    setJobs(ind, context);
    _nestedScheduler->run(ind, context);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
AltJobSelector::init()
{
    _nestedScheduler = new ForwardScheduler();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
AltJobSelector::setAltJobGroups(const ClevorDataSet* dataSet)
{
    ASSERT(dataSet != nullptr);
    jobgroup_set_id_t::const_iterator jobgroupIt;
    for (jobgroupIt = dataSet->jobGroups().begin(); jobgroupIt != dataSet->jobGroups().end();
         jobgroupIt++)
    {
        JobGroup* jobGroup = *jobgroupIt;
        _jobGroups.push_back(jobGroup);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
AltJobSelector::setJobs(StringInd<uint_t>* ind, SchedulingContext* context) const
{
    ASSERTD(_config != nullptr);
    gop::String<uint_t>& string = ind->string();
    Manager* mgr = context->manager();
    uint_t numGroups = _jobGroups.size();
    for (uint_t i = 0; i < numGroups; ++i)
    {
        JobGroup* group = _jobGroups[i];
        ASSERTD(group->jobs().size() > 1);
        uint_t jobIdx = string[_stringBase + i];
        group->setActiveJob(jobIdx);

        const job_set_pref_t& jobs = group->jobs();
        for (job_set_pref_t::const_iterator it = jobs.begin(); it != jobs.end(); it++)
        {
            Job* job = (*it);
            if (!job->active())
            {
                job->finalize(mgr);
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;
