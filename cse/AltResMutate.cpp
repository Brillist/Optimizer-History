#include "libcse.h"
#include <libutl/BufferedFDstream.h>
#include <clp/FailEx.h>
#include "AltResMutate.h"
#include "ClevorDataSet.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef DEBUG
#define DEBUG_UNIT
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;
GOP_NS_USE;
CLP_NS_USE;
CLS_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(cse::AltResMutate);

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

void
AltResMutate::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(AltResMutate));
    const AltResMutate& arm = (const AltResMutate&)rhs;
    RevOperator::copy(arm);

    _numResGroupReqChoices = arm._numResGroupReqChoices;
    _resGroupReqs = arm._resGroupReqs;

    _moveSchedule = arm._moveSchedule;
    _moveResGroupReqIdx = arm._moveResGroupReqIdx;
    _moveResIdx = arm._moveResIdx;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
AltResMutate::initialize(const gop::DataSet* p_dataSet)
{
    RevOperator::initialize();

    ASSERTD(dynamic_cast<const ClevorDataSet*>(p_dataSet) != nullptr);
    const ClevorDataSet* dataSet = (const ClevorDataSet*)p_dataSet;
    setResGroupReqs(dataSet);

    //init  _numResGroupReqChoices
    uint_t numResGroupReqs = _resGroupReqs.size();
    for (uint_t i = 0; i < numResGroupReqs; i++)
    {
        cls::DiscreteResourceRequirement* clsResGroupReq = _resGroupReqs[i];
        const Hashtable& resCapPtsSet = clsResGroupReq->resCapPtsSet();
        uint_t numAltRess = resCapPtsSet.size();
        ASSERTD(numAltRess > 1);
        BrkActivity* act = clsResGroupReq->activity();
        JobOp* op = (JobOp*)act->owner();
        addOperatorVar(i, 1, 2, op->job()->activeP());
        _numResGroupReqChoices += numAltRess;
    }
    setNumChoices(_numResGroupReqChoices);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool
AltResMutate::execute(gop::Ind* ind, gop::IndBuilderContext* p_context, bool singleStep)
{
    ASSERTD(dynamic_cast<SchedulingContext*>(p_context) != nullptr);
    ASSERTD(dynamic_cast<StringInd<uint_t>*>(ind) != nullptr);
    SchedulingContext* context = (SchedulingContext*)p_context;
    Manager* mgr = context->manager();
    _moveSchedule = (StringInd<uint_t>*)ind;
    gop::String<uint_t>& string = _moveSchedule->string();

    // choose a resGroupReq
    uint_t resGroupReqIdx = this->varIdx();
    cls::DiscreteResourceRequirement* resGroupReq = _resGroupReqs[resGroupReqIdx];
    _moveResGroupReqIdx = _stringBase + resGroupReqIdx;
    _moveResIdx = string[_moveResGroupReqIdx];

    // choose resource
    uint_t numResources = resGroupReq->resCapPtsSet().size();
    ASSERTD(numResources > 1);
    uint_t resIdx;
    if (singleStep)
    {
        if (_moveResIdx == 0)
        {
            resIdx = _moveResIdx + 1;
        }
        else if (_moveResIdx == numResources - 1)
        {
            resIdx = _moveResIdx - 1;
        }
        else
        {
            uint_t randomNum = _rng->uniform(0, 1);
            if (randomNum == 0)
            {
                resIdx = _moveResIdx - 1;
            }
            else
            {
                resIdx = _moveResIdx + 1;
            }
        }
        ASSERTD(resIdx >= 0 && resIdx <= numResources - 1);
    }
    else
    {
        resIdx = _rng->uniform((uint_t)0, numResources - 2);
        if (resIdx >= _moveResIdx)
            ++resIdx;
    }
    string[_moveResGroupReqIdx] = resIdx;
    uint_t resId = resGroupReq->resIdxCapPts(resIdx)->resourceId();

#ifdef DEBUG_UNIT
    auto act = resGroupReq->activity();
    utl::cout << "                                                   "
              << "task:" << act->id()
              << ", oldAltRes:" << resGroupReq->resIdxCapPts(_moveResIdx)->resource()->id()
              << "(idx=" << _moveResIdx << ")"
              << ", newAltRes:" << resGroupReq->resIdxCapPts(resIdx)->resource()->id()
              << "(idx=" << resIdx << ")" << utl::endl;
#endif
    resGroupReq->selectResource(resId);
    mgr->propagate();
    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
AltResMutate::accept()
{
    ASSERTD(_moveSchedule != nullptr);
    if (_moveSchedule->newString())
        _moveSchedule->acceptNewString();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
AltResMutate::undo()
{
    ASSERTD(_moveSchedule != nullptr);
    if (_moveSchedule->newString())
        _moveSchedule->deleteNewString();
    gop::String<uint_t>& string = _moveSchedule->string();

    if (_moveResGroupReqIdx != uint_t_max)
    {
        ASSERTD(_moveResIdx != uint_t_max);
        string[_moveResGroupReqIdx] = _moveResIdx;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
AltResMutate::init()
{
    _numResGroupReqChoices = 0;
    _moveSchedule = nullptr;
    _moveResGroupReqIdx = uint_t_max;
    _moveResIdx = uint_t_max;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
AltResMutate::deInit()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
AltResMutate::setResGroupReqs(const ClevorDataSet* dataSet)
{
    // init _resGroupReqs
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

CSE_NS_END;
