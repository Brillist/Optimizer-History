#include "libcse.h"
#include <libutl/BufferedFDstream.h>
#include <libutl/Time.h>
#include <gop/ConfigEx.h>
#include <cse/ForwardScheduler.h>
#include "ReleaseTimeMutate.h"
#include "ReleaseTimeSelector.h"

// #include <clp/BoundPropagator.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef DEBUG
// #define DEBUG_UNIT
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;
CLP_NS_USE;
CLS_NS_USE;
GOP_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(cse::ReleaseTimeSelector, cse::Scheduler);

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ReleaseTimeSelector::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(ReleaseTimeSelector));
    const ReleaseTimeSelector& rts = (const ReleaseTimeSelector&)rhs;
    Scheduler::copy(rts);
    _acts = rts._acts;
    _minRlsTimes = rts._minRlsTimes;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ReleaseTimeSelector::setStringBase(Operator* op) const
{
    if (dynamic_cast<ReleaseTimeMutate*>(op) != nullptr)
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
ReleaseTimeSelector::stringSize(const ClevorDataSet& dataSet) const
{
    ASSERTD(_nestedScheduler != nullptr);
    uint_t stringSize = _acts.size();
    stringSize += _nestedScheduler->stringSize(dataSet);
    return stringSize;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ReleaseTimeSelector::initialize(const gop::DataSet* p_dataSet, uint_t stringBase)
{
    ASSERTD(_nestedScheduler != nullptr);
    ASSERTD(dynamic_cast<const ClevorDataSet*>(p_dataSet) != nullptr);
    const ClevorDataSet* dataSet = (const ClevorDataSet*)p_dataSet;

    setActs(dataSet);
    _stringBase = stringBase;
    _nestedScheduler->initialize(dataSet, _stringBase + _acts.size());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ReleaseTimeSelector::initializeInd(Ind* p_ind,
                                   const gop::DataSet* p_dataSet,
                                   RandNumGen* rng,
                                   void*)
{
    ASSERTD(dynamic_cast<StringInd<uint_t>*>(p_ind) != nullptr);
    StringInd<uint_t>* ind = (StringInd<uint_t>*)p_ind;
    gop::String<uint_t>& string = ind->string();

    ASSERTD(dynamic_cast<const ClevorDataSet*>(p_dataSet) != nullptr);
    const ClevorDataSet* dataSet = (const ClevorDataSet*)p_dataSet;

    uint_t numActs = _acts.size();
    if (_stringBase == 0)
    {
        string.setSize(stringSize(*dataSet));
    }
    for (uint_t i = 0; i < numActs; ++i)
    {
        string[_stringBase + i] = _minRlsTimes[i];
    }

    _nestedScheduler->initializeInd(ind, dataSet, rng, (void*)size_t_max);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ReleaseTimeSelector::run(Ind* p_ind, IndBuilderContext* p_context) const
{
    ASSERTD(_nestedScheduler != nullptr);
    ASSERTD(dynamic_cast<StringInd<uint_t>*>(p_ind) != nullptr);
    StringInd<uint_t>* ind = (StringInd<uint_t>*)p_ind;
    ASSERTD(dynamic_cast<SchedulingContext*>(p_context) != nullptr);
    SchedulingContext* context = (SchedulingContext*)p_context;

    setReleaseTimes(ind, context);
    _nestedScheduler->run(ind, context);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ReleaseTimeSelector::init()
{
    _nestedScheduler = new ForwardScheduler();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ReleaseTimeSelector::setActs(const ClevorDataSet* dataSet)
{
    //init _acts and _minRlsTimes
    const job_set_id_t& jobs = dataSet->jobs();
    job_set_id_t::iterator jobIt;
    for (jobIt = jobs.begin(); jobIt != jobs.end(); jobIt++)
    {
        Job* job = (*jobIt);
        SummaryOp* op = job->rootSummaryOp();
        ASSERT(op != nullptr);
#ifdef DEBUG_UNIT
        utl::cout << op->toString() << utl::endlf;
#endif
        const unaryct_vect_t& unaryCts = op->unaryCts();
        if (unaryCts.size() == 0)
            continue;
        unaryct_vect_t::const_iterator it;
        bool hasRlsTime = false;
        uint_t rlsTime = uint_t_max;
        for (it = unaryCts.begin(); it != unaryCts.end(); it++)
        {
            UnaryCt* uct = *it;
            if (uct->type() == uct_startNoSoonerThan)
            {
                hasRlsTime = true;
                rlsTime = dataSet->schedulerConfig()->timeToTimeSlot(uct->time());
#ifdef DEBUG_UNIT
                utl::cout << "summaryOp:" << op->id()
                          << " has a release date:" << Time(uct->time()).toString() << utl::endlf;
#endif
                break;
            }
        }
        if (!hasRlsTime)
            continue;
        Activity* act = op->activity();
#ifdef DEBUG_UNIT
        utl::cout << act->toString() << ", rlsTime:" << rlsTime << ", act->es():" << act->es()
                  << utl::endlf;
#endif
        _acts.push_back(act);
        _minRlsTimes.push_back(min(rlsTime, (uint_t)act->es())); // _minRlsTimes
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ReleaseTimeSelector::setReleaseTimes(StringInd<uint_t>* ind, SchedulingContext* context) const
{
#ifdef DEBUG_UNIT
    utl::cout << "In ReleaseTimeSelector::setReleaseTime() ..." << utl::endlf;
#endif
    ASSERTD(_config != nullptr);
    gop::String<uint_t>& string = ind->string();
    uint_t numActs = _acts.size();
    for (uint_t i = 0; i < numActs; ++i)
    {
        Activity* act = _acts[i];
        uint_t releaseTime = string[_stringBase + i];

#ifdef DEBUG_UNIT
        utl::cout << act->toString() << ", rlsT:" << releaseTime << utl::endlf;
#endif

        int es = act->es();
        if (es >= 0 && (uint_t)es < releaseTime)
        {
            act->start().setLB(releaseTime);
        }
    }
    context->manager()->propagate();
#ifdef DEBUG_UNIT
    utl::cout << "End of ReleaseTimeSelector::setReleaseTime() ..." << utl::endlf;
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;
