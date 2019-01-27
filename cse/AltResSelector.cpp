#include "libcse.h"
#include <libutl/BufferedFDstream.h>
#include <cls/BrkActivity.h>
#include <cse/AltResMutate.h>
#include <gop/ConfigEx.h>
#include "ForwardScheduler.h"
#include "AltResSelector.h"
#include "MinCostHeuristics.h"

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

UTL_CLASS_IMPL(cse::AltResSelector);

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

void
AltResSelector::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(AltResSelector));
    const AltResSelector& ars = (const AltResSelector&)rhs;
    Scheduler::copy(ars);
    _resGroupReqs = ars._resGroupReqs;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
AltResSelector::setStringBase(Operator* op) const
{
    if (dynamic_cast<AltResMutate*>(op) != nullptr)
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
AltResSelector::stringSize(const ClevorDataSet& dataSet) const
{
    ASSERTD(_nestedScheduler != nullptr);
    uint_t stringSize = _resGroupReqs.size();
    stringSize += _nestedScheduler->stringSize(dataSet);
    return stringSize;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
AltResSelector::initialize(const gop::DataSet* p_dataSet, uint_t stringBase)
{
    ASSERTD(_nestedScheduler != nullptr);
    ASSERTD(dynamic_cast<const ClevorDataSet*>(p_dataSet) != nullptr);
    const ClevorDataSet* dataSet = (const ClevorDataSet*)p_dataSet;

    setResGroupReqs(dataSet);
    _stringBase = stringBase;
    _nestedScheduler->initialize(dataSet, _stringBase + _resGroupReqs.size());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
AltResSelector::initializeInd(Ind* p_ind, const gop::DataSet* p_dataSet, rng_t* rng, void*)
{
    ASSERTD(dynamic_cast<StringInd<uint_t>*>(p_ind) != nullptr);
    StringInd<uint_t>* ind = (StringInd<uint_t>*)p_ind;
    gop::String<uint_t>& string = ind->string();

    ASSERTD(dynamic_cast<const ClevorDataSet*>(p_dataSet) != nullptr);
    const ClevorDataSet* dataSet = (const ClevorDataSet*)p_dataSet;
    const MinCostHeuristics* minCostHeuristics = dataSet->minCostHeuristics();

    uint_t numResGroupReqs = _resGroupReqs.size();
    if (_stringBase == 0)
    {
        string.setSize(stringSize(*dataSet));
    }
    for (uint_t i = 0; i < numResGroupReqs; ++i)
    {
        cls::DiscreteResourceRequirement* resGroupReq = _resGroupReqs[i];
        BrkActivity* act = resGroupReq->activity();
        JobOp* op = (JobOp*)(act->owner());
        const MinCostAltResPt* minCostAltResPt = minCostHeuristics->getMinCostAltResPt(op);
        uint_vector_t altResIdxs = minCostAltResPt->altResIdxs();
        for (uint_t j = 0; j < altResIdxs.size(); j++)
        {
            i = i + j;
            uint_t resIdx = altResIdxs[j];
            string[_stringBase + i] = resIdx;
        }
    }
    _nestedScheduler->initializeInd(ind, dataSet, rng, (void*)size_t_max);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
AltResSelector::initializeRandomInd(Ind* p_ind, const gop::DataSet* p_dataSet, rng_t* rng, void*)
{
    ASSERTD(dynamic_cast<StringInd<uint_t>*>(p_ind) != nullptr);
    StringInd<uint_t>* ind = (StringInd<uint_t>*)p_ind;
    gop::String<uint_t>& string = ind->string();

    ASSERTD(dynamic_cast<const ClevorDataSet*>(p_dataSet) != nullptr);
    const ClevorDataSet* dataSet = (const ClevorDataSet*)p_dataSet;

    uint_t numResGroupReqs = _resGroupReqs.size();
    if (_stringBase == 0)
    {
        string.setSize(stringSize(*dataSet));
    }
    for (uint_t i = 0; i < numResGroupReqs; ++i)
    {
        string[_stringBase + i] = 0;
    }
    _nestedScheduler->initializeRandomInd(ind, dataSet, rng, (void*)size_t_max);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
AltResSelector::run(Ind* p_ind, IndBuilderContext* p_context) const
{
    ASSERTD(dynamic_cast<StringInd<uint_t>*>(p_ind) != nullptr);
    StringInd<uint_t>* ind = (StringInd<uint_t>*)p_ind;

    ASSERTD(dynamic_cast<SchedulingContext*>(p_context) != nullptr);
    SchedulingContext* context = (SchedulingContext*)p_context;

    setSelectedResources(ind, context);
    _nestedScheduler->run(ind, context);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
AltResSelector::init()
{
    _nestedScheduler = new ForwardScheduler();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
AltResSelector::setResGroupReqs(const ClevorDataSet* dataSet)
{
    // note all alternate resource-constraints
    _resGroupReqs.clear();
    const jobop_set_id_t& ops = dataSet->sops();
    jobop_set_id_t::const_iterator it;
    for (it = ops.begin(); it != ops.end(); ++it)
    {
        JobOp* op = *it;
        if (!op->breakable() || op->frozen())
            continue;
        uint_t numResGroupReqs = op->numResGroupReqs();
        for (uint_t i = 0; i < numResGroupReqs; ++i)
        {
            cse::ResourceGroupRequirement* cseResGroupReq = op->getResGroupReq(i);
            cls::DiscreteResourceRequirement* clsResGroupReq = cseResGroupReq->clsResReq();
            uint_t numResources = clsResGroupReq->resCapPtsSet().size();
            if (numResources > 1)
                _resGroupReqs.push_back(clsResGroupReq);
        }
    }
    std::sort(_resGroupReqs.begin(), _resGroupReqs.end(), ObjectOrdering());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
AltResSelector::setSelectedResources(StringInd<uint_t>* ind, SchedulingContext* context) const
{
    ASSERTD(_config != nullptr);
    gop::String<uint_t>& string = ind->string();
    Manager* mgr = context->manager();
    uint_t numResGroupReqs = _resGroupReqs.size();
    for (uint_t i = 0; i < numResGroupReqs; ++i)
    {
        uint_t resIdx = string[_stringBase + i];
        cls::DiscreteResourceRequirement* resGroupReq = _resGroupReqs[i];

        BrkActivity* act = resGroupReq->activity();
        JobOp* op = (JobOp*)act->owner();
        if (!op->job()->active())
            continue;

        const clp::IntExp* possibleRes = resGroupReq->possibleResources();
        const clp::IntExp* selectedRes = resGroupReq->selectedResources();
        uint_t resId = resGroupReq->resIdxCapPts(resIdx)->resourceId();

        // resource already selected?
        if (selectedRes->size() > 0)
        {
            ASSERT((possibleRes->size() == 0) && (selectedRes->size() == 1));
            if (!selectedRes->has(resId))
            {
                if (!ind->newString())
                    ind->createNewString();
                gop::String<uint_t>& newStr = *(ind->newString());
                uint_t newResId = selectedRes->getValue();
                uint_t newResIdx = uint_t_max;
                uint_t j;
                for (j = 0; j < resGroupReq->numCapPts(); ++j)
                {
                    if (newResId == resGroupReq->resIdxCapPts(j)->resourceId())
                    {
                        newResIdx = j;
                        break;
                    }
                }
                ASSERT(newResIdx != uint_t_max);
                newStr[_stringBase + i] = newResIdx;
            }
            continue;
        }

        // is resId possible?
        if (possibleRes->has(resId))
        {
            resGroupReq->selectResource(resId);
        }
        else
        {
            ASSERT(possibleRes->size() > 0);
            if (!ind->newString())
                ind->createNewString();
            gop::String<uint_t>& newStr = *(ind->newString());
            int prevResId = possibleRes->getPrev(resId);
            int nextResId = possibleRes->getNext(resId);
            uint_t newResIdx = uint_t_max;
            uint_t idx = resIdx;
            if ((prevResId > 0) && ((int)resId - prevResId) < (nextResId - (int)resId))
            {
                do
                {
                    --idx;
                    int newResId = resGroupReq->resIdxCapPts(idx)->resourceId();
                    if (newResId == prevResId)
                    {
                        newResIdx = idx;
                        break;
                    }
                } while (idx != 0);
                resGroupReq->selectResource(prevResId);
            }
            else
            {
                do
                {
                    ++idx;
                    int newResId = resGroupReq->resIdxCapPts(idx)->resourceId();
                    if (newResId == nextResId)
                    {
                        newResIdx = idx;
                        break;
                    }
                } while (idx != (resGroupReq->resCapPtsSet().size() - 1));
                resGroupReq->selectResource(nextResId);
            }
            ASSERTD(newResIdx != uint_t_max);
            newStr[_stringBase + i] = newResIdx;

#ifdef DEBUG_UNIT
            utl::cout << "Warning(AltResSelector.cpp): " << resId << "(idx=" << resIdx
                      << ") is not a valid resId in " << possibleRes->toString().get()
                      << ". New resId is set to "
                      << resGroupReq->resIdxCapPts(newResIdx)->resourceId() << "(idx=" << newResIdx
                      << ")." << utl::endl;
#endif
        }

        mgr->propagate();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;
