#include "libcse.h"
#include <libutl/BufferedFDstream.h>
#include <clp/FailEx.h>
#include "OpSeqMutate.h"
#include "DiscreteResource.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef DEBUG
// #define DEBUG_UNIT
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;
GOP_NS_USE;
CLP_NS_USE;
CLS_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(cse::OpSeqMutate);

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

void
OpSeqMutate::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(OpSeqMutate));
    const OpSeqMutate& jsmutate = (const OpSeqMutate&)rhs;
    RevOperator::copy(jsmutate);
    _ops = jsmutate._ops;
    _swapOpsMap = jsmutate._swapOpsMap;

    _moveSchedule = jsmutate._moveSchedule;
    _moveOpSid = jsmutate._moveOpSid;
    _moveOps = jsmutate._moveOps;
    _moveOpIdxs = jsmutate._moveOpIdxs;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
OpSeqMutate::initialize(const gop::DataSet* p_dataSet)
{
    RevOperator::initialize();

    ASSERTD(dynamic_cast<const ClevorDataSet*>(p_dataSet) != nullptr);
    const ClevorDataSet* dataSet = (const ClevorDataSet*)p_dataSet;
    setOps(dataSet);

    uint_t numChoices = 0;
    uint_t i = 0;
    jobop_jobopvector_map_t::iterator it;
    for (it = _swapOpsMap.begin(); it != _swapOpsMap.end(); it++)
    {
        JobOp* op = (*it).first;
        jobop_vector_t* opVect = (*it).second;
        uint_t numSwapOps = numActiveSwapOps(opVect);
        if (numSwapOps == 0)
        {
            addOperatorVar(i, 0, 2, op->job()->activeP());
        }
        else
        {
            addOperatorVar(i, 1, 2, op->job()->activeP());
        }
        if (op->job()->active())
        {
            numChoices += numSwapOps;
        }
        i++;
    }
    ASSERTD(numChoices % 2 == 0);
    setNumChoices(numChoices / 2);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool
OpSeqMutate::execute(gop::Ind* ind, gop::IndBuilderContext* p_context, bool singleStep)
{
    ASSERTD(dynamic_cast<SchedulingContext*>(p_context) != nullptr);
    ASSERTD(dynamic_cast<StringInd<uint_t>*>(ind) != nullptr);
    SchedulingContext* context = (SchedulingContext*)p_context;
    Manager* mgr = context->manager();
    _moveSchedule = (StringInd<uint_t>*)ind;
    gop::String<uint_t>& string = _moveSchedule->string();

    //init all ops sid for failed initOptRun and multiple strings +
    uint_t numOps = _ops.size();
    uint_t i;
    for (i = 0; i < numOps; i++)
    {
        JobOp* op1 = _ops[i];
        op1->serialId() = string[_stringBase + i];
    }

    //select an op
    uint_t opIdx = this->varIdx();
    _moveOpIdx = opIdx;
    JobOp* op = _ops[opIdx];
    ASSERTD(op->serialId() != uint_t_max);

    //select a swap op
    jobop_jobopvector_map_t::iterator opMapIt = _swapOpsMap.find(op);
    ASSERTD(opMapIt != _swapOpsMap.end());
    jobop_vector_t* swapOps = (*opMapIt).second;
    JobOp* swapOp = selectActiveSwapOp(swapOps);
    if (swapOp == nullptr)
        swapOp = op;

    // swap two ops
    uint_t opSid = op->serialId();
    uint_t swapOpSid = swapOp->serialId();
    ASSERTD(opSid != uint_t_max && swapOpSid != uint_t_max);
    // collect and sort all ops between op and swapOp
    uint_t minOpSid = utl::min(opSid, swapOpSid);
    uint_t maxOpSid = utl::max(opSid, swapOpSid);
    _moveOpSid = minOpSid; //_moveOpSid
    _moveOps.clear();
    _moveOpIdxs.clear();
    for (uint_t j = 0; j < numOps; j++)
    {
        JobOp* op1 = _ops[j];
        uint_t sid = op1->serialId();
        if (sid >= minOpSid && sid <= maxOpSid)
        {
            _moveOps.push_back(op1);
            _moveOpIdxs.push_back(j);
        }
    }
    std::sort(_moveOps.begin(), _moveOps.end(), JobOpSerialIdOrdering());
    // move firstOp and all its sucessors to the end
    jobop_vector_t changedOps = _moveOps;
    JobOp* firstOp = *(changedOps.begin());
    const cg_revset_t& allSuccCGs = firstOp->esCG()->allSuccCGs();
    JobOp* nextOp = firstOp;
    jobop_vector_t::iterator changedOpsEnd = changedOps.end();
    JobOp* lastOp = *(--changedOpsEnd);
    jobop_vector_t::iterator startIt = changedOps.begin();
    while (nextOp != lastOp)
    {
        if ((nextOp == firstOp) || (allSuccCGs.find(nextOp->esCG()) != allSuccCGs.end()))
        {
            changedOps.erase(startIt);
            changedOps.push_back(nextOp);
        }
        else
        {
            startIt++;
        }
        nextOp = *startIt;
    }
    // re-assign sid
    uint_t newSid = minOpSid;
    for (jobop_vector_t::iterator it = changedOps.begin(); it != changedOps.end(); ++it)
    {
        JobOp* op1 = *it;
        op1->serialId() = newSid++;
    }
    // update string
    for (uint_vector_t::iterator it = _moveOpIdxs.begin(); it != _moveOpIdxs.end(); it++)
    {
        uint_t idx = *it;
        JobOp* op1 = _ops[idx];
        string[_stringBase + idx] = op1->serialId();
    }
    ASSERTD(op->breakable() || op->interruptible());
    ASSERTD(swapOp->breakable() || swapOp->interruptible());
    Activity* act1 = op->activity();
    Activity* act2 = swapOp->activity();
    const uint_set_t& opResIds = act1->allResIds();
    const uint_set_t& swapOpResIds = act2->allResIds();
    uint_set_t commonResIds;
    std::set_intersection(opResIds.begin(), opResIds.end(), swapOpResIds.begin(),
                          swapOpResIds.end(), std::inserter(commonResIds, commonResIds.begin()));
    ASSERT(commonResIds.size() > 0);
    uint_t resIdx = _rng->uniform((size_t)0, commonResIds.size() - 1);
    uint_t resId = uint_t_max;

    uint_set_t::iterator resIt = commonResIds.begin();
    for (uint_t i = 0; i != resIdx; i++)
        ++resIt;
    resId = *resIt;
    ASSERT(resId != uint_t_max);
    act1->selectResource(resId);
    act2->selectResource(resId);
    mgr->propagate();
#ifdef DEBUG_UNIT
    utl::cout << "                                                   "
              << "job1:" << op->job()->id() << ", op1:" << op->id() << "(" << opSid << "->"
              << op->serialId() << "), job2:" << swapOp->job()->id() << ", op2:" << swapOp->id()
              << "(" << swapOpSid << "->" << swapOp->serialId() << "), seleRes:" << resId
              << utl::endl;
#endif
    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
OpSeqMutate::accept()
{
    ASSERTD(_moveSchedule != nullptr);
    if (_moveSchedule->newString())
        _moveSchedule->acceptNewString();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
OpSeqMutate::undo()
{
    ASSERTD(_moveSchedule != nullptr);
    ASSERTD(_moveOps.size() != 0);
    ASSERTD(_moveOpIdxs.size() != 0);
    if (_moveSchedule->newString())
        _moveSchedule->deleteNewString();
    gop::String<uint_t>& string = _moveSchedule->string();
    uint_t startSid = _moveOpSid;
    for (uint_t i = 0; i < _moveOps.size(); i++)
    {
        JobOp* op = _moveOps[i];
        op->serialId() = startSid++;
    }
    for (uint_vector_t::iterator it = _moveOpIdxs.begin(); it != _moveOpIdxs.end(); it++)
    {
        uint_t idx = *it;
        JobOp* op = _ops[idx];
        string[_stringBase + idx] = op->serialId();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
OpSeqMutate::setOps(const ClevorDataSet* dataSet)
{
    //initialize _ops and _swapOpsMap
    _ops.clear();
    deleteMapSecond(_swapOpsMap);
    const jobop_set_id_t& ops = dataSet->sops();
    jobop_set_id_t::const_iterator opIt;
    for (opIt = ops.begin(); opIt != ops.end(); opIt++)
    {
        JobOp* op = *opIt;
        _ops.push_back(op);
        jobop_vector_t* opVect = new jobop_vector_t();
        _swapOpsMap.insert(jobop_jobopvector_map_t::value_type(op, opVect));
    }

    // iterate over resources in the data-set
    const res_set_id_t& resources = dataSet->resources();
    res_set_id_t::const_iterator it;
    for (it = resources.begin(); it != resources.end(); ++it)
    {
        DiscreteResource* res = dynamic_cast<DiscreteResource*>(*it);

        // skip non-discrete resource or disRes with no cap
        // or disRes with 0 maxResCap
        if (res == nullptr || res->maxCap() == 0)
            continue;
        cls::DiscreteResource* clsRes = (cls::DiscreteResource*)res->clsResource();
        if (clsRes->maxReqCap() == 0)
            continue;

        // resOps = {all ops that may require this resource}
        jobop_set_id_t resOps;
        for (opIt = ops.begin(); opIt != ops.end(); ++opIt)
        {
            JobOp* op = *opIt;

            //skip if (op.frozen()) or (act != ptAct  and act != intAct)
            //or (op.pt == 0)
            if (op->frozen() || (!op->breakable() && !op->interruptible()))
            {
                continue;
            }
            Activity* act = op->activity();
            ASSERTD(act != nullptr);
            if (act->isA(PtActivity))
            {
                PtActivity* ptact = (PtActivity*)act;
                const IntVar* ptExp = ptact->possiblePts();
                if ((ptExp->isBound() && ptExp->getValue() == 0))
                {
                    continue;
                }
            }

            // can this activity require res?
            const uint_set_t& actResIds = act->allResIds();
            if (actResIds.find(res->id()) != actResIds.end())
            {
                resOps.insert(op);
            }
        }

        //check code DO NOT DELETE
        //         jobop_set_id_t::iterator it;
        //         utl::cout << "RES:" << res->id() << ", resOps:";
        //         for (it = resOps.begin(); it != resOps.end(); it++)
        //         {
        //            utl::cout << (*it)->id()
        //                      << "(" << ((JobOp*)(*it))->job()->id()
        //                      << "), ";
        //         }
        //         utl::cout << utl::endl << utl::endl;

        uint_t numOps = resOps.size();
        if (numOps < 2)
            continue;
        jobop_set_id_t::iterator it1;
        jobop_set_id_t::iterator it2;
        for (it1 = resOps.begin(); it1 != resOps.end(); it1++)
        {
            JobOp* op = (*it1);
            jobop_jobopvector_map_t::iterator opMapIt;
            opMapIt = _swapOpsMap.find(op);
            ASSERT(opMapIt != _swapOpsMap.end());
            jobop_vector_t* opVect = (*opMapIt).second;

            CycleGroup* cg = op->esCG();
            const cg_revset_t& allPredCGs = cg->allPredCGs();
            const cg_revset_t& allSuccCGs = cg->allSuccCGs();

            it2 = it1;
            for (++it2; it2 != resOps.end(); it2++)
            {
                JobOp* candOp = (*it2);
                CycleGroup* candCG = candOp->esCG();

                // skip candCG if it has precedence relationship with cg
                if (allPredCGs.find(candCG) != allPredCGs.end())
                    continue;
                if (allSuccCGs.find(candCG) != allSuccCGs.end())
                    continue;

                jobop_jobopvector_map_t::iterator candOpMapIt;
                candOpMapIt = _swapOpsMap.find(candOp);
                ASSERT(candOpMapIt != _swapOpsMap.end());
                jobop_vector_t* candOpVect = (*candOpMapIt).second;
                opVect->push_back(candOp);
                candOpVect->push_back(op);
            }
        }
    }

    //     uint_t i = 0;
    jobop_jobopvector_map_t::iterator mapIt;
    for (mapIt = _swapOpsMap.begin(); mapIt != _swapOpsMap.end(); mapIt++)
    {
        jobop_vector_t* opVect = (*mapIt).second;
        std::sort(opVect->begin(), opVect->end(), JobOpIdOrdering());
        jobop_vector_t::iterator it;
        JobOp* prevOp = nullptr;
        for (it = opVect->begin(); it != opVect->end(); it++)
        {
            JobOp* op = (*it);
            if (prevOp && prevOp->id() == op->id())
            {
                opVect->erase(it);
                it--;
                continue;
            }
            prevOp = op;
        }
        //check code. DO NOT DELETE
        //         utl::cout << "i:" << i++
        //                   << ", op(job):" << (*mapIt).first->id()
        //                   << "(" << ((JobOp*)(*mapIt).first)->job()->id() << ")"
        //                   << ", #swapOps:" << Uint(opVect->size())
        //                   << ", swapOps:";
        //         for (it = opVect->begin(); it != opVect->end(); it++)
        //            utl::cout << (*it)->id()
        //                      << "(" << ((JobOp*)(*it))->job()->id() << "), ";
        //         utl::cout << utl::endl << utl::endl;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

uint_t
OpSeqMutate::numActiveSwapOps(const jobop_vector_t* opVect)
{
    uint_t n = 0;
    jobop_vector_t::const_iterator it;
    for (it = opVect->begin(); it != opVect->end(); it++)
    {
        JobOp* op = (*it);
        if (op->job()->active())
            n++;
    }
    return n;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

JobOp*
OpSeqMutate::selectActiveSwapOp(const jobop_vector_t* opVect)
{
    jobop_vector_t ops;
    jobop_vector_t::const_iterator it;
    for (it = opVect->begin(); it != opVect->end(); it++)
    {
        JobOp* op = (*it);
        if (op->job()->active())
            ops.push_back(op);
    }
    if (ops.size() == 0)
        return nullptr;
    uint_t idx = _rng->uniform((size_t)0, ops.size() - 1);
    return ops[idx];
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
OpSeqMutate::init()
{
    _moveOpSid = uint_t_max;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
OpSeqMutate::deInit()
{
    deleteMapSecond(_swapOpsMap);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;
