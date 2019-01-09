#include "libcse.h"
#include <libutl/BufferedFDstream.h>
#include <cls/BrkActivity.h>
#include <cse/OpSeqMutate.h>
#include <gop/ConfigEx.h>
#include "ForwardScheduler.h"
#include "OpSeqSelector.h"

//////////////////////////////////////////////////////////////////////////////

#ifdef DEBUG
#define DEBUG_UNIT
#endif

//////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;
CLP_NS_USE;
CLS_NS_USE;
GOP_NS_USE;

//////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(cse::OpSeqSelector, cse::Scheduler);

//////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////

void
OpSeqSelector::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(OpSeqSelector));
    const OpSeqSelector& rcs = (const OpSeqSelector&)rhs;
    Scheduler::copy(rcs);
}

//////////////////////////////////////////////////////////////////////////////

void
OpSeqSelector::setStringBase(Operator* op) const
{
    if (dynamic_cast<OpSeqMutate*>(op) != nullptr)
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

//////////////////////////////////////////////////////////////////////////////

uint_t
OpSeqSelector::stringSize(const ClevorDataSet& dataSet) const
{
    ASSERTD(_nestedScheduler != nullptr);
    uint_t stringSize = dataSet.sops().size();
    stringSize += _nestedScheduler->stringSize(dataSet);
    return stringSize;
}

//////////////////////////////////////////////////////////////////////////////

void
OpSeqSelector::initialize(
    const gop::DataSet* p_dataSet,
    uint_t stringBase)
{
    ASSERTD(_nestedScheduler != nullptr);
    ASSERTD(dynamic_cast<const ClevorDataSet*>(p_dataSet) != nullptr);
    const ClevorDataSet* dataSet = (const ClevorDataSet*)p_dataSet;
    _stringBase = stringBase;
    ASSERT(dynamic_cast<ForwardScheduler*>(_nestedScheduler) != nullptr);
    ForwardScheduler* fScheduler = (ForwardScheduler*)_nestedScheduler;
    fScheduler->setJobs(dataSet);
    fScheduler->setJobOps(dataSet);
    fScheduler->setOps(dataSet);
    fScheduler->opStartPosition() = _stringBase;
    _nestedScheduler->initialize(dataSet,
        _stringBase + dataSet->sops().size());
}

//////////////////////////////////////////////////////////////////////////////

void
OpSeqSelector::initializeInd(
    Ind* p_ind,
    const gop::DataSet* p_dataSet,
    RandNumGen* rng,
    void*)
{
    ASSERTD(dynamic_cast<StringInd<uint_t>*>(p_ind) != nullptr);
    ASSERTD(dynamic_cast<const ClevorDataSet*>(p_dataSet) != nullptr);
    StringInd<uint_t>* ind = (StringInd<uint_t>*)p_ind;
    gop::String<uint_t>& string = ind->string();
    const ClevorDataSet* dataSet = (const ClevorDataSet*)p_dataSet;

    if (_stringBase == 0)
    {
        string.setSize(stringSize(*dataSet));
    }

    const jobop_set_id_t& ops = dataSet->sops();
    uint_t i = 0;
    jobop_set_id_t::const_iterator it;
    for (it = ops.begin(); it != ops.end(); ++it)
    {
        JobOp* op = *it;
        op->serialId() = uint_t_max;
        string[_stringBase + i] = uint_t_max;
        i++;
    }
    _nestedScheduler->initializeInd(
        ind, dataSet, rng, (void*)size_t_max);
}

//////////////////////////////////////////////////////////////////////////////

void
OpSeqSelector::initializeRandomInd(
    Ind* p_ind,
    const gop::DataSet* p_dataSet,
    RandNumGen* rng,
    void*)
{
    utl::cout << "ERROR(OpSeqSelector.cpp): in the incompleted code!"
              << utl::endl;
    ABORT();
}

//////////////////////////////////////////////////////////////////////////////

void
OpSeqSelector::run(Ind* p_ind, IndBuilderContext* p_context) const
{
    ASSERTD(dynamic_cast<StringInd<uint_t>*>(p_ind) != nullptr);
    ASSERTD(dynamic_cast<SchedulingContext*>(p_context) != nullptr);
    StringInd<uint_t>* ind = (StringInd<uint_t>*)p_ind;
    SchedulingContext* context = (SchedulingContext*)p_context;

    setSelectedOpSeq(ind, context);
    _nestedScheduler->run(ind, context);
}

//////////////////////////////////////////////////////////////////////////////

void
OpSeqSelector::init()
{
    _nestedScheduler = new ForwardScheduler();
}

//////////////////////////////////////////////////////////////////////////////

void
OpSeqSelector::setSelectedOpSeq(
    StringInd<uint_t>* ind,
    SchedulingContext* context) const
{
    gop::String<uint_t>& string = ind->string();
    const jobop_set_id_t& ops = context->clevorDataSet()->sops();
    uint_t i = 0;
    jobop_set_id_t::const_iterator it;
    for (it = ops.begin(); it != ops.end(); it++)
    {
        JobOp* op = *it;
        op->serialId() = string[_stringBase + i];
        i++;
    }
}

//////////////////////////////////////////////////////////////////////////////

CSE_NS_END;
