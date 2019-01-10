#include "libcse.h"
#include <libutl/BufferedFDstream.h>
#include <cls/BrkActivity.h>
#include <cse/PtMutate.h>
#include <gop/ConfigEx.h>
#include "MinCostHeuristics.h"
#include "ForwardScheduler.h"
#include "PtSelector.h"

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

UTL_CLASS_IMPL(cse::PtSelector, cse::Scheduler);

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

void
PtSelector::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(PtSelector));
    const PtSelector& ps = (const PtSelector&)rhs;
    Scheduler::copy(ps);
    _acts = ps._acts;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
PtSelector::setStringBase(Operator* op) const
{
    if (dynamic_cast<PtMutate*>(op) != nullptr)
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
PtSelector::stringSize(const ClevorDataSet& dataSet) const
{
    ASSERTD(_nestedScheduler != nullptr);
    uint_t stringSize = _acts.size();
    stringSize += _nestedScheduler->stringSize(dataSet);
    return stringSize;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
PtSelector::initialize(const gop::DataSet* p_dataSet, uint_t stringBase)
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
PtSelector::initializeInd(Ind* p_ind, const gop::DataSet* p_dataSet, RandNumGen* rng, void*)
{
    ASSERTD(dynamic_cast<StringInd<uint_t>*>(p_ind) != nullptr);
    StringInd<uint_t>* ind = (StringInd<uint_t>*)p_ind;
    gop::String<uint_t>& string = ind->string();

    ASSERTD(dynamic_cast<const ClevorDataSet*>(p_dataSet) != nullptr);
    const ClevorDataSet* dataSet = (const ClevorDataSet*)p_dataSet;
    const MinCostHeuristics* minCostHeuristics = dataSet->minCostHeuristics();

    uint_t numActs = _acts.size();
    if (_stringBase == 0)
    {
        string.setSize(stringSize(*dataSet));
    }
    for (uint_t i = 0; i < numActs; ++i)
    {
        PtActivity* act = _acts[i];
        JobOp* op = (JobOp*)(act->owner());
        const MinCostAltResPt* minCostAltResPt = minCostHeuristics->getMinCostAltResPt(op);
        string[_stringBase + i] = minCostAltResPt->pt();
    }

    //     for (uint_t i = 0; i < numActs; ++i)
    //     {
    //         string[_stringBase + i] = 0;
    //     }
    _nestedScheduler->initializeInd(ind, dataSet, rng, (void*)size_t_max);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
PtSelector::initializeRandomInd(Ind* p_ind, const gop::DataSet* p_dataSet, RandNumGen* rng, void*)
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
        const IntExp& ptExp = _acts[i]->possiblePts();
        uint_t minPt = ptExp.min();
        uint_t tt = rng->evali(2);
        if (tt == 0)
        {
            string[_stringBase + i] = minPt;
        }
        else
        {
            string[_stringBase + i] = ptExp.getNext(minPt);
        }
    }
    _nestedScheduler->initializeRandomInd(ind, dataSet, rng, (void*)size_t_max);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
PtSelector::run(Ind* p_ind, IndBuilderContext* p_context) const
{
    ASSERTD(_nestedScheduler != nullptr);
    ASSERTD(dynamic_cast<StringInd<uint_t>*>(p_ind) != nullptr);
    StringInd<uint_t>* ind = (StringInd<uint_t>*)p_ind;
    ASSERTD(dynamic_cast<SchedulingContext*>(p_context) != nullptr);
    SchedulingContext* context = (SchedulingContext*)p_context;

    setPts(ind, context);
    _nestedScheduler->run(ind, context);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
PtSelector::init()
{
    _nestedScheduler = new ForwardScheduler();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
PtSelector::setActs(const ClevorDataSet* dataSet)
{
    // note all processing-time vars that are not bound
    _acts.clear();
    const jobop_set_id_t& ops = dataSet->sops();
    jobop_set_id_t::const_iterator it;
    for (it = ops.begin(); it != ops.end(); ++it)
    {
        JobOp* jobOp = *it;
        if (jobOp->frozen())
            continue;
        Activity* act = jobOp->activity();
        ASSERTD(act != nullptr);
        if (!act->isA(PtActivity))
            continue;
        PtActivity* ptact = (PtActivity*)act;
        const IntExp* ptExp = ptact->possiblePts();
        if (!ptExp->isBound())
        {
            _acts.push_back(ptact);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
PtSelector::setPts(StringInd<uint_t>* ind, SchedulingContext* context) const
{
    ASSERTD(_config != nullptr);
    gop::String<uint_t>& string = ind->string();
    Manager* mgr = context->manager();
    uint_t numActs = _acts.size();
    for (uint_t i = 0; i < numActs; ++i)
    {
        PtActivity* act = _acts[i];
        JobOp* op = (JobOp*)act->owner();
        if (!op->job()->active())
            continue;
        const IntExp& ptExp = act->possiblePts();
        uint_t pt = string[_stringBase + i];
        if (ptExp.has(pt))
        {
            act->selectPt(pt);
        }
        else
        {
            if (ind->newString() == nullptr)
                ind->createNewString();
            gop::String<uint_t>& newStr = *(ind->newString());
            int prevPt = ptExp.getPrev(pt);
            int nextPt = ptExp.getNext(pt);
            if (prevPt > 0 && (pt - prevPt) < (nextPt - pt))
            {
                //if (pt == 0) string[_stringBase + i] = prevPt;
                newStr[_stringBase + i] = prevPt;
                act->selectPt(prevPt);
            }
            else
            {
                //if (pt == 0) string[_stringBase + i] = nextPt;
                newStr[_stringBase + i] = nextPt;
                act->selectPt(nextPt);
            }
#ifdef DEBUG_UNIT
            if (pt != 0)
            {
                utl::cout << "Warning(PtSelector.cpp): " << pt
                          << " is not a valid pt. the new pt is set to "
                          << act->possiblePts().toString().get() << utl::endl;
            }
#endif
        }
        mgr->propagate();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;
