#include "libcse.h"
#include <libutl/BufferedFDstream.h>
#include <cls/IntActivity.h>
#include <cls/ESboundInt.h>
#include <cse/CapMutate.h>
#include <gop/ConfigEx.h>
#include "MinCostHeuristics.h"
#include "ForwardScheduler.h"
#include "CapSelector.h"

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

UTL_CLASS_IMPL(cse::CapSelector);

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

void
CapSelector::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(CapSelector));
    const CapSelector& ps = (const CapSelector&)rhs;
    Scheduler::copy(ps);
    _acts = ps._acts;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
CapSelector::setStringBase(Operator* op) const
{
    if (dynamic_cast<CapMutate*>(op) != nullptr)
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
CapSelector::stringSize(const ClevorDataSet& dataSet) const
{
    ASSERTD(_nestedScheduler != nullptr);
    uint_t stringSize = _acts.size();
    stringSize += _nestedScheduler->stringSize(dataSet);
    return stringSize;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
CapSelector::initialize(const gop::DataSet* p_dataSet, uint_t stringBase)
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
CapSelector::initializeInd(Ind* p_ind, const gop::DataSet* p_dataSet, rng_t* rng, void*)
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
        IntActivity* act = _acts[i];
        cls::ESboundInt& esb = (ESboundInt&)act->esBound();
        //         string[_stringBase + i] = esb.minMultiple();
        string[_stringBase + i] = esb.maxMultiple();
    }

    _nestedScheduler->initializeInd(ind, dataSet, rng, (void*)size_t_max);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
CapSelector::run(Ind* p_ind, IndBuilderContext* p_context) const
{
    ASSERTD(_nestedScheduler != nullptr);
    ASSERTD(dynamic_cast<StringInd<uint_t>*>(p_ind) != nullptr);
    StringInd<uint_t>* ind = (StringInd<uint_t>*)p_ind;
    ASSERTD(dynamic_cast<SchedulingContext*>(p_context) != nullptr);
    SchedulingContext* context = (SchedulingContext*)p_context;

    setCaps(ind, context);
    _nestedScheduler->run(ind, context);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
CapSelector::init()
{
    _nestedScheduler = new ForwardScheduler();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
CapSelector::setActs(const ClevorDataSet* dataSet)
{
    // init _acts
    _acts.clear();
    const jobop_set_id_t& ops = dataSet->sops();
    jobop_set_id_t::const_iterator it;
    for (it = ops.begin(); it != ops.end(); ++it)
    {
        JobOp* jobOp = *it;
        if (jobOp->frozen() || !jobOp->interruptible())
            continue;
        IntActivity* act = jobOp->intact();
        uint_t minCapMultiple, maxCapMultiple;
        if (act->forward())
        {
            cls::ESboundInt& esb = (ESboundInt&)act->esBound();
            minCapMultiple = esb.minMultiple();
            maxCapMultiple = esb.maxMultiple();
        }
        else
        {
            ABORT();
        }
        ASSERTD(minCapMultiple <= maxCapMultiple);
        if (minCapMultiple == maxCapMultiple)
            continue;
        _acts.push_back(act);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
CapSelector::setCaps(StringInd<uint_t>* ind, SchedulingContext* context) const
{
    ASSERTD(_config != nullptr);
    gop::String<uint_t>& string = ind->string();
    uint_t numActs = _acts.size();
    for (uint_t i = 0; i < numActs; ++i)
    {
        IntActivity* act = _acts[i];
        JobOp* op = (JobOp*)act->owner();
        if (!op->job()->active())
            continue;
        uint_t cap = string[_stringBase + i];
        if (act->forward())
        {
            cls::ESboundInt& esb = (ESboundInt&)act->esBound();
            esb.setMaxMultiple(cap);
        }
        else
        {
            ABORT();
        }
        //no propagation is needed
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;
