#include "libcse.h"
#include <libutl/BufferedFDstream.h>
#include <clp/FailEx.h>
#include <cls/ESbound.h>
#include <cls/ESboundInt.h>
#include "JobOpSeqMutate.h"
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

UTL_CLASS_IMPL(cse::JobOpSeqMutate);

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

void
JobOpSeqMutate::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(JobOpSeqMutate));
    const JobOpSeqMutate& jsmutate = (const JobOpSeqMutate&)rhs;
    RevOperator::copy(jsmutate);
    _jobs = jsmutate._jobs;
    _ops = jsmutate._ops;
    _jobStrPositions = jsmutate._jobStrPositions;
    _jobNumChoices = jsmutate._jobNumChoices;
    _swapOps = jsmutate._swapOps;

    _moveSchedule = jsmutate._moveSchedule;
    _moveJobIdx = jsmutate._moveJobIdx;
    _moveOpSid = jsmutate._moveOpSid;
    _moveOps = jsmutate._moveOps;
    _moveOpIdxs = jsmutate._moveOpIdxs;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
JobOpSeqMutate::initialize(const gop::DataSet* p_dataSet)
{
    RevOperator::initialize();

    auto dataSet = utl::cast<ClevorDataSet>(p_dataSet);
    setJobOps(dataSet);

    uint_t numJobs = _jobs.size();
    uint_t numChoices = 0;
    for (uint_t i = 0; i < numJobs; i++)
    {
        auto job = _jobs[i];
        if (job->active())
        {
            numChoices += _jobNumChoices[i];
        }
    }
    ASSERTD(numChoices % 2 == 0);
    setNumChoices(numChoices / 2);

    uint_t numOps = _swapOps.size();
    ASSERTD(_ops.size() == _swapOps.size());
    for (uint_t i = 0; i < numOps; i++)
    {
        auto op = _ops[i];
        auto swapops = _swapOps[i];
        if (swapops->size() == 0)
        {
            addOperatorVar(i, 0, 2, op->job()->activeP());
        }
        else
        {
            addOperatorVar(i, 1, 2, op->job()->activeP());
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool
JobOpSeqMutate::execute(gop::Ind* ind, gop::IndBuilderContext* p_context, bool singleStep)
{
    ASSERTD(dynamic_cast<SchedulingContext*>(p_context) != nullptr);
    ASSERTD(dynamic_cast<StringInd<uint_t>*>(ind) != nullptr);
    SchedulingContext* context = (SchedulingContext*)p_context;
    Manager* mgr = context->manager();
    _moveSchedule = (StringInd<uint_t>*)ind;
    gop::String<uint_t>& string = _moveSchedule->string();

    //init all ops sid for failed initOptRun and multiple strings
    uint_t numJobs = _jobs.size();
    uint_t totalNumOps = _ops.size();
    uint_t i, j;
    for (i = 0; i < totalNumOps; i++)
    {
        JobOp* op1 = _ops[i];
        op1->serialId() = string[_stringBase + i];
    }

    // select an op and its job - (jobOpIdx and jobIdx)
    uint_t opIdx = getSelectedVarIdx();
#ifdef DEBUG_UNIT
    utl::cout << "          varSucRate:" << getSelectedVarP() << ", idx:" << opIdx;
#endif

    uint_t jobIdx = uint_t_max;
    for (i = 0; i < numJobs; i++)
    {
        Job* job1 = _jobs[i];
        if (job1->allSops().size() == 0)
            continue;
        uint_t jobStartPos = _jobStrPositions[i] - _stringBase;
        uint_t jobEndPos = jobStartPos + job1->allSops().size() - 1;
        if (opIdx >= jobStartPos && opIdx <= jobEndPos)
        {
            jobIdx = i;
            break;
        }
    }
    ASSERTD(jobIdx != uint_t_max);
    _moveJobIdx = jobIdx; // _moveJobIdx
    Job* job = _jobs[jobIdx];
    uint_t numOps = job->allSops().size();
    uint_t initPos = _jobStrPositions[jobIdx] - _stringBase;

    //select a swap op - (swapOpidx)
    jobop_vector_t* swapOps = _swapOps[opIdx];
    ASSERTD(swapOps->size() > 0);
    uint_t swapOpIdx = _rng->uniform((size_t)0, swapOps->size() - 1);

    //swap two ops
    JobOp* op = _ops[opIdx]; //
    uint_t opSid = op->serialId();
    JobOp* swapOp = (*swapOps)[swapOpIdx];
    uint_t swapOpSid = swapOp->serialId();
    ASSERTD(op->job()->id() == swapOp->job()->id());
    ASSERTD(op->job()->active());
    ASSERTD(opSid != uint_t_max && swapOpSid != uint_t_max);

    ////collect and sort all ops between op and swapOp
    uint_t minOpSid = utl::min(opSid, swapOpSid);
    uint_t maxOpSid = utl::max(opSid, swapOpSid);
    _moveOpSid = minOpSid; //_moveOpSid
    _moveOps.clear();
    _moveOpIdxs.clear();
    for (j = 0; j < numOps; j++)
    {
        JobOp* op1 = _ops[initPos + j];
        uint_t sid = op1->serialId();
        if (sid >= minOpSid && sid <= maxOpSid)
        {
            _moveOps.push_back(op1);
            _moveOpIdxs.push_back(j);
        }
    }
    std::sort(_moveOps.begin(), _moveOps.end(), JobOpSerialIdOrdering());
    // move firstOp and all its successors to the end
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
            startIt = changedOps.erase(startIt);
            changedOps.push_back(nextOp);
        }
        else
        {
            startIt++;
        }
        nextOp = *startIt;
    }
    ////re-assign sid
    uint_t newSid = minOpSid;
    for (jobop_vector_t::iterator it = changedOps.begin(); it != changedOps.end(); ++it)
    {
        JobOp* op3 = *it;
        op3->serialId() = newSid++;
    }
    ////update string
    for (uint_vector_t::iterator it = _moveOpIdxs.begin(); it != _moveOpIdxs.end(); it++)
    {
        uint_t idx = *it;
        JobOp* op4 = _ops[initPos + idx];
        string[_jobStrPositions[jobIdx] + idx] = op4->serialId();
    }
    ASSERTD((op->serialId() - swapOp->serialId() == 1) ||
            (swapOp->serialId() - op->serialId() == 1));

    // force the use of a common resource
    ASSERTD(op->breakable() || op->interruptible());
    ASSERTD(swapOp->breakable() || swapOp->interruptible());
    uint_t resId = uint_t_max;
    Activity* act1 = op->activity();
    Activity* act2 = swapOp->activity();
    const uint_set_t& opResIds = act1->allResIds();
    const uint_set_t& swapOpResIds = act2->allResIds();
    uint_set_t commonResIds;
    std::set_intersection(opResIds.begin(), opResIds.end(), swapOpResIds.begin(),
                          swapOpResIds.end(), std::inserter(commonResIds, commonResIds.begin()));
    ASSERT(commonResIds.size() > 0);
    uint_t resIdx = _rng->uniform((size_t)0, commonResIds.size() - 1);

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
              << "job:" << job->id() << ", op1:" << op->id() << "(" << opSid << "->"
              << op->serialId() << "), op2:" << swapOp->id() << "(" << swapOpSid << "->"
              << swapOp->serialId() << "), seleRes:" << resId << utl::endl;
#endif
    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
JobOpSeqMutate::accept()
{
    ASSERTD(_moveSchedule != nullptr);
    if (_moveSchedule->newString())
        _moveSchedule->acceptNewString();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
JobOpSeqMutate::undo()
{
    ASSERTD(_moveSchedule != nullptr);
    ASSERTD(_moveJobIdx != uint_t_max);
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
    //uint_t numOps = job->allSops().size();
    uint_t initPos = _jobStrPositions[_moveJobIdx] - _stringBase;
    for (uint_vector_t::iterator it = _moveOpIdxs.begin(); it != _moveOpIdxs.end(); it++)
    {
        uint_t idx = *it;
        JobOp* op = _ops[initPos + idx];
        string[_jobStrPositions[_moveJobIdx] + idx] = op->serialId();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
JobOpSeqMutate::setJobOps(const ClevorDataSet* dataSet)
{
    // initialize _jobStrPositions, _jobs and _ops
    auto& jobs = dataSet->jobs();
    uint_t strPosition = _stringBase;
    for (auto job : jobs)
    {
        _jobs.push_back(job);
        auto& jobOps = job->allSops();
        uint_t numOps = jobOps.size();
        for (auto op : jobOps)
        {
            _ops.push_back(op);
        }
        // note position for this job, and update the position
        _jobStrPositions.push_back(strPosition);
        strPosition += numOps;
    }

    // init _jobNumChoices and _swapOps
    for (auto job : jobs)
    {
        auto& jobOps = job->allSops();

        // jobOpCGs = list of all CGs associated with job's ops
        cg_set_id_t jobOpCGs;
        for (auto op : jobOps)
        {
            jobOpCGs.insert(op->esCG());
        }

        // for each of the job's ops
        uint_t jobNumChoices = 0;
        for (auto op : jobOps)
        {
            // every op has a swap-ops list (even if it's empty)
            auto swapOps = new jobop_vector_t();
            _swapOps.push_back(swapOps);

            // skip op that is frozen or (not breakable and not interruptible)
            if (op->frozen() || (!op->breakable() && !op->interruptible()))
                continue;

            // skip op for pt-activity with pt=0
            auto act = op->activity();
            ASSERTD(act != nullptr);
            if (act->isA(PtActivity))
            {
                auto ptact = utl::cast<PtActivity>(act);
                auto& ptExp = ptact->possiblePts();
                if ((ptExp.isBound() && ptExp.getValue() == 0))
                    continue;
            }

            // cg = op's CG (for earliest-start)
            // allPredCGs = cg's predecessor CGs
            // allSuccCGs = cg's successor CGs
            auto cg = op->esCG();
            auto& allPredCGs = cg->allPredCGs();
            auto& allSuccCGs = cg->allSuccCGs();

            //cg_set_id_t tempCandidates, cgCandidates;

            // tempCandidates = job CGs that do not precede this op's CG
            cg_vector_t tempCandidates;
            tempCandidates.reserve(jobOpCGs.size());
            std::set_difference(
                jobOpCGs.begin(), jobOpCGs.end(), allPredCGs.begin(), allPredCGs.end(),
                std::inserter(tempCandidates, tempCandidates.begin()), CycleGroupIdOrdering());

            // cgCandidates = job CGs that neither precede nor succeed this op's CG
            cg_vector_t cgCandidates;
            cgCandidates.reserve(tempCandidates.size());
            std::set_difference(tempCandidates.begin(), tempCandidates.end(), allSuccCGs.begin(),
                                allSuccCGs.end(), std::inserter(cgCandidates, cgCandidates.begin()),
                                CycleGroupIdOrdering());

            auto& opRess = act->allResIds();
            for (auto candCG : cgCandidates)
            {
                for (auto cb : *candCG)
                {
                    // skip non-ES bound
                    if (!cb->isA(cls::ESbound) && !cb->isA(cls::ESboundInt))
                        continue;

                    // skip frozen op
                    auto candidateAct = utl::cast<Activity>(cb->owner());
                    auto candidateOp = utl::cast<JobOp>(candidateAct->owner());
                    if (candidateOp->frozen())
                        continue;

                    // skip pt-activity with pt=0
                    if (candidateAct->isA(PtActivity))
                    {
                        auto candidatePtact = utl::cast<PtActivity>(candidateAct);
                        auto& candidatePtExp = candidatePtact->possiblePts();
                        if (candidatePtExp.isBound() && (candidatePtExp.getValue() == 0))
                            continue;
                    }

                    // candOpRess = candOp's possible resources
                    auto& candOpRess = candidateAct->allResIds();

                    // commonRess = op and candOp's common resources
                    uint_vector_t commonResIds;
                    std::set_intersection(opRess.begin(), opRess.end(), candOpRess.begin(),
                                          candOpRess.end(),
                                          std::inserter(commonResIds, commonResIds.begin()));

                    // remove dummy resources from commonResIds
                    for (auto it = commonResIds.begin(); it != commonResIds.end();)
                    {
                        uint_t resId = *it;
                        auto dres = utl::cast<DiscreteResource>(dataSet->findResource(resId));
                        auto clsRes = utl::cast<cls::DiscreteResource>(dres->clsResource());
                        if (dres->maxCap() == 0 || clsRes->maxReqCap() == 0)
                        {
                            it = commonResIds.erase(it);
                        }
                        else
                        {
                            ++it;
                        }
                    }

                    if ((candidateOp != op) && (commonResIds.size() > 0))
                    {
                        ASSERTD(op->job()->id() == candidateOp->job()->id());
                        swapOps->push_back(candidateOp);
                    }
                } // for (auto cb : *candCG)
            } // for (auto candCG : cgCandidates)

            // sort swap-ops by id
            std::sort(swapOps->begin(), swapOps->end(), JobOpIdOrdering());

            // add to the required string space
            jobNumChoices += swapOps->size();
        } // for (auto op : jobOps)
        _jobNumChoices.push_back(jobNumChoices);
    } // for (auto job : jobs)
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
JobOpSeqMutate::init()
{
    _moveJobIdx = uint_t_max;
    _moveOpSid = uint_t_max;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
JobOpSeqMutate::deInit()
{
    deleteCont(_swapOps);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;
