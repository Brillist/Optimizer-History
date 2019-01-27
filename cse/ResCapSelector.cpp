#include "libcse.h"
#include <libutl/BufferedFDstream.h>
#include <cls/BrkActivity.h>
#include <cse/ResCapMutate.h>
#include <gop/ConfigEx.h>
#include "ForwardScheduler.h"
#include "ResCapSelector.h"

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

UTL_CLASS_IMPL(cse::ResCapSelector);

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ResCapSelector::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(ResCapSelector));
    const ResCapSelector& rcs = (const ResCapSelector&)rhs;
    Scheduler::copy(rcs);
    _resources = rcs._resources;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ResCapSelector::setStringBase(Operator* op) const
{
    if (dynamic_cast<ResCapMutate*>(op) != nullptr)
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
ResCapSelector::stringSize(const ClevorDataSet& dataSet) const
{
    ASSERTD(_nestedScheduler != nullptr);
    uint_t stringSize = _resources.size();
    stringSize += _nestedScheduler->stringSize(dataSet);
    return stringSize;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ResCapSelector::initialize(const gop::DataSet* p_dataSet, uint_t stringBase)
{
    ASSERTD(_nestedScheduler != nullptr);
    ASSERTD(dynamic_cast<const ClevorDataSet*>(p_dataSet) != nullptr);
    const ClevorDataSet* dataSet = (const ClevorDataSet*)p_dataSet;

    setResources(dataSet);
    _stringBase = stringBase;
    _nestedScheduler->initialize(dataSet, _stringBase + _resources.size());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ResCapSelector::initializeInd(Ind* p_ind, const gop::DataSet* p_dataSet, rng_t* rng, void*)
{
    ASSERTD(dynamic_cast<StringInd<uint_t>*>(p_ind) != nullptr);
    StringInd<uint_t>* ind = (StringInd<uint_t>*)p_ind;
    gop::String<uint_t>& string = ind->string();

    ASSERTD(dynamic_cast<const ClevorDataSet*>(p_dataSet) != nullptr);
    const ClevorDataSet* dataSet = (const ClevorDataSet*)p_dataSet;

    uint_t numResources = _resources.size();
    if (_stringBase == 0)
    {
        string.setSize(stringSize(*dataSet));
    }
    for (uint_t i = 0; i < numResources; ++i)
    {
        DiscreteResource* res = _resources[i];
        cls::DiscreteResource* clsRes = (cls::DiscreteResource*)res->clsResource();
        uint_t maxReqCap = roundUp(clsRes->maxReqCap(), (uint_t)100);
        uint_t initCap = utl::max(res->minCap(), utl::min(res->maxCap(), maxReqCap));
        string[_stringBase + i] = initCap;
    }
    _nestedScheduler->initializeInd(ind, dataSet, rng, (void*)size_t_max);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ResCapSelector::initializeRandomInd(Ind* p_ind, const gop::DataSet* p_dataSet, rng_t* rng, void*)
{
    auto ind = utl::cast<StringInd<uint_t>>(p_ind);
    gop::String<uint_t>& string = ind->string();

    ASSERTD(dynamic_cast<const ClevorDataSet*>(p_dataSet) != nullptr);
    const ClevorDataSet* dataSet = (const ClevorDataSet*)p_dataSet;

    uint_t numResources = _resources.size();
    if (_stringBase == 0)
    {
        string.setSize(stringSize(*dataSet));
    }
    for (uint_t i = 0; i < numResources; ++i)
    {
        uint_t range = (_resources[i]->maxCap() - _resources[i]->minCap()) / 100;
        ASSERTD(range != 0);
        uint_t cap = _resources[i]->minCap() + (rng->uniform((uint_t)0, range - 1)) * 100;
        string[_stringBase + i] = cap;
    }
    _nestedScheduler->initializeRandomInd(ind, dataSet, rng, (void*)size_t_max);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ResCapSelector::run(Ind* p_ind, IndBuilderContext* p_context) const
{
    ASSERTD(dynamic_cast<StringInd<uint_t>*>(p_ind) != nullptr);
    StringInd<uint_t>* ind = (StringInd<uint_t>*)p_ind;

    ASSERTD(dynamic_cast<SchedulingContext*>(p_context) != nullptr);
    SchedulingContext* context = (SchedulingContext*)p_context;

    setSelectedResCaps(ind, context);
    _nestedScheduler->run(ind, context);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ResCapSelector::init()
{
    _nestedScheduler = new ForwardScheduler();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ResCapSelector::setResources(const ClevorDataSet* dataSet)
{
    // note all resources with minCap < maxCap
    _resources.clear();
    res_set_id_t::const_iterator it;
    for (it = dataSet->resources().begin(); it != dataSet->resources().end(); ++it)
    {
        if (!(*it)->isA(DiscreteResource))
            continue;
        auto res = utl::cast<DiscreteResource>(*it);
        auto clsRes = utl::cast<cls::DiscreteResource>(res->clsResource());
        uint_t minReqCap = roundUp(clsRes->minReqCap(), (uint_t)100);
        uint_t maxReqCap = roundUp(clsRes->maxReqCap(), (uint_t)100);
        ASSERTD(res->maxCap() >= res->minCap());
        ASSERTD(res->maxCap() >= minReqCap);
        if ((res->maxCap() - res->minCap()) >= res->stepCap())
        {
            uint_t minCap = utl::max(res->minCap(), minReqCap);
            uint_t maxCap = utl::max(minCap, utl::min(res->maxCap(), maxReqCap));
            if (maxCap > minCap)
            {
                _resources.push_back(res);
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ResCapSelector::setSelectedResCaps(StringInd<uint_t>* ind, SchedulingContext* context) const
{
    ASSERTD(_config != nullptr);
    gop::String<uint_t>& string = ind->string();
    Manager* mgr = context->manager();
    uint_t numResources = _resources.size();
    for (uint_t i = 0; i < numResources; ++i)
    {
        const cse::DiscreteResource* res = _resources[i];
        if (res->selectedCap() != uint_t_max)
            continue;
        cls::DiscreteResource* clsRes = res->clsResource();
        uint_t selCap = string[_stringBase + i];
        uint_t minReqCap = roundUp(clsRes->minReqCap(), (uint_t)100);

        if (selCap < minReqCap)
        {
            if (!ind->newString())
                ind->createNewString();
            gop::String<uint_t>& newStr = *(ind->newString());
#ifdef DEBUG_UNIT
            utl::cout << "Warning(ResCapSelector.cpp): " << selCap << " is less the minReqCap("
                      << minReqCap << ") of resource(" << clsRes->id() << ")." << utl::endl;
#endif
            selCap = minReqCap;
            newStr[_stringBase + i] = selCap;
        }
        clsRes->selectCapacity(selCap, res->maxCap());
        mgr->propagate();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;
