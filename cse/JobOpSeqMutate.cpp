 #include "libcse.h"
#include <libutl/BufferedFDstream.h>
#include <clp/FailEx.h>
#include <cls/ESbound.h>
#include <cls/ESboundInt.h>
#include "JobOpSeqMutate.h"
#include "DiscreteResource.h"

//////////////////////////////////////////////////////////////////////////////

#ifdef DEBUG
// #define DEBUG_UNIT
#endif

//////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;
GOP_NS_USE;
CLP_NS_USE;
CLS_NS_USE;

//////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(cse::JobOpSeqMutate, gop::RevOperator);

//////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////

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

//////////////////////////////////////////////////////////////////////////////

void
JobOpSeqMutate::initialize(const gop::DataSet* p_dataSet)
{
    RevOperator::initialize();

    ASSERTD(dynamic_cast<const ClevorDataSet*>(p_dataSet) != nullptr);
    const ClevorDataSet* dataSet = (const ClevorDataSet*)p_dataSet;
    setJobOps(dataSet);

    uint_t numJobs = _jobs.size();
    uint_t numChoices = 0;
    for (uint_t i = 0; i < numJobs; i++)
    {
        Job* job = _jobs[i];
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
        JobOp* op = _ops[i];
        jobop_vector_t* swapops = _swapOps[i];
        if (swapops->size() == 0)
        {
            addOperatorVar(i,0,2,op->job()->activeP());
        }
        else
        {
            addOperatorVar(i,1,2,op->job()->activeP());
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

bool
JobOpSeqMutate::execute(
    gop::Ind* ind,
    gop::IndBuilderContext* p_context,
    bool singleStep)
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

    //select an op and its job - (jobOpIdx and jobIdx)
    uint_t opIdx = getSelectedVarIdx();
#ifdef DEBUG_UNIT
    utl::cout << "          varSucRate:" << getSelectedVarP()
              << ", idx:" << opIdx;
#endif

    uint_t jobIdx = uint_t_max;
    for (i = 0; i < numJobs; i++)
    {
        Job* job1 = _jobs[i];
        if (job1->allSops().size() == 0) continue;
        uint_t jobStartPos = _jobStrPositions[i] - _stringBase;
        uint_t jobEndPos = jobStartPos + job1->allSops().size() - 1;
        if (opIdx >= jobStartPos && opIdx <= jobEndPos)
        {
            jobIdx = i;
            break;
        }
    }
    ASSERTD(jobIdx != uint_t_max);
    _moveJobIdx = jobIdx;// _moveJobIdx
    Job* job = _jobs[jobIdx];
    uint_t numOps = job->allSops().size();
    uint_t initPos = _jobStrPositions[jobIdx] - _stringBase;

    //select a swap op - (swapOpidx)
    jobop_vector_t* swapOps = _swapOps[opIdx];
    ASSERTD(swapOps->size() > 0);
    uint_t swapOpIdx = _rng->evali(swapOps->size());

    //swap two ops
    JobOp* op = _ops[opIdx];//
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
    ////move firstOp and all its sucessors to the end
    jobop_vector_t changedOps = _moveOps;
    JobOp* firstOp = *(changedOps.begin());
    const cg_revset_t& allSuccCGs = firstOp->esCG()->allSuccCGs();
    JobOp* nextOp = firstOp;
    jobop_vector_t::iterator changedOpsEnd = changedOps.end();
    JobOp* lastOp = *(--changedOpsEnd);
    jobop_vector_t::iterator startIt = changedOps.begin();
    while (nextOp != lastOp)
    {
        if ((nextOp == firstOp)
            || (allSuccCGs.find(nextOp->esCG()) != allSuccCGs.end()))
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
    ////re-assign sid
    uint_t newSid = minOpSid;
    for (jobop_vector_t::iterator it = changedOps.begin();
         it != changedOps.end();
         ++it)
    {
        JobOp* op3 = *it;
        op3->serialId() = newSid++;
    }
    ////update string
    for (uint_vector_t::iterator it = _moveOpIdxs.begin();
         it != _moveOpIdxs.end(); it++)
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
    std::set_intersection(
        opResIds.begin(), opResIds.end(),
        swapOpResIds.begin(), swapOpResIds.end(),
        std::inserter(commonResIds, commonResIds.begin()));
    ASSERT(commonResIds.size() > 0);
    uint_t resIdx = _rng->evali(commonResIds.size());

    uint_set_t::iterator resIt = commonResIds.begin();
    for (uint_t i = 0; i != resIdx; i++) ++resIt;
    resId = *resIt;
    ASSERT(resId != uint_t_max);
    act1->selectResource(resId);
    act2->selectResource(resId);
    mgr->propagate();

#ifdef DEBUG_UNIT
    utl::cout
        << "                                                   "
        << "job:" << job->id()
        << ", op1:" << op->id() << "(" << opSid << "->" << op->serialId()
        << "), op2:" << swapOp->id() << "(" << swapOpSid
        << "->" << swapOp->serialId()
        << "), seleRes:" << resId << utl::endl;
#endif
    return true;
}

//////////////////////////////////////////////////////////////////////////////

void
JobOpSeqMutate::accept()
{
    ASSERTD(_moveSchedule != nullptr);
    if (_moveSchedule->newString()) _moveSchedule->acceptNewString();
}

//////////////////////////////////////////////////////////////////////////////

void
JobOpSeqMutate::undo()
{
    ASSERTD(_moveSchedule != nullptr);
    ASSERTD(_moveJobIdx != uint_t_max);
    ASSERTD(_moveOps.size() != 0);
    ASSERTD(_moveOpIdxs.size() != 0);
    if (_moveSchedule->newString()) _moveSchedule->deleteNewString();
    gop::String<uint_t>& string = _moveSchedule->string();
    //Job* job = _jobs[_moveJobIdx];
    uint_t startSid = _moveOpSid;
    for (uint_t i = 0; i < _moveOps.size(); i++)
    {
        JobOp* op = _moveOps[i];
        op->serialId() = startSid++;
    }
    //uint_t numOps = job->allSops().size();
    uint_t initPos = _jobStrPositions[_moveJobIdx] - _stringBase;
    for (uint_vector_t::iterator it = _moveOpIdxs.begin();
         it != _moveOpIdxs.end(); it++)
    {
        uint_t idx = *it;
        JobOp* op = _ops[initPos + idx];
        string[_jobStrPositions[_moveJobIdx] + idx] = op->serialId();
    }
}

//////////////////////////////////////////////////////////////////////////////

void
JobOpSeqMutate::setJobOps(const ClevorDataSet* dataSet)
{
    //initialize _jobStrPositions, _jobs and _ops
    const job_set_id_t& jobs = dataSet->jobs();
    uint_t strPosition = _stringBase;
    job_set_id_t::const_iterator jobIt;
    jobop_set_id_t::const_iterator opIt;
    for (jobIt = jobs.begin(); jobIt != jobs.end(); jobIt++)
    {
        Job* job = *jobIt;
        _jobs.push_back(job); //_jobs
        jobop_set_id_t jobOps = job->allSops();
        uint_t numOps = jobOps.size();
        for (opIt = jobOps.begin(); opIt != jobOps.end(); opIt++)
        {
            JobOp* op = *opIt;
            _ops.push_back(op); //_ops
        }
        _jobStrPositions.push_back(strPosition); //jobStrPosition
        strPosition += numOps;
    }

    //init _jobNumChoices and _swapOps
    for (jobIt = jobs.begin(); jobIt != jobs.end(); jobIt++)
    {
        Job* job = *jobIt;
        const jobop_set_id_t& jobOps = job->allSops();

        // jobOpCGs = list of all CGs associated with job's ops
        cg_set_id_t jobOpCGs;
        for (opIt = jobOps.begin(); opIt != jobOps.end(); ++opIt)
        {
            JobOp* op = *opIt;
            jobOpCGs.insert(op->esCG());
        }

        // for each of the job's ops
        uint_t jobNumChoices = 0;
        for (opIt = jobOps.begin(); opIt != jobOps.end(); opIt++)
        {
            JobOp* op = *opIt;
            jobop_vector_t* opVect = new jobop_vector_t();

            //skip if (op.frozen()) or (act != ptAct  and act != intAct)
            //or (op.pt == 0)
            if (op->frozen() ||
                (!op->breakable() && !op->interruptible()))
            {
                _swapOps.push_back(opVect);
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
                    _swapOps.push_back(opVect);
                    continue;
                }
            }

            CycleGroup* cg = op->esCG();
            const cg_revset_t& allPredCGs = cg->allPredCGs();
            const cg_revset_t& allSuccCGs = cg->allSuccCGs();
            cg_set_id_t tempCandidates, cgCandidates;

            // cgCandidates = CGs that neither precede nor succeed cg
            std::set_difference(
                jobOpCGs.begin(), jobOpCGs.end(),
                allPredCGs.begin(), allPredCGs.end(),
                std::inserter(tempCandidates, tempCandidates.begin()),
                CycleGroupIdOrdering());
            std::set_difference(
                tempCandidates.begin(), tempCandidates.end(),
                allSuccCGs.begin(), allSuccCGs.end(),
                std::inserter(cgCandidates, cgCandidates.begin()),
                CycleGroupIdOrdering());

            //checking code. DON'T DELETE
//             utl::cout << "OP:" << op->id()
//                       << ", job:" << op->job()->id()
//                       << ", #jobOps:" << Uint(jobOps.size())
//                       << ", #predCGs:" << allPredCGs.size()
//                       << ", #succCGs:" << allSuccCGs.size()
//                       << ", #candidateCGs:" << Uint(cgCandidates.size())
//                       << utl::endl;
//             utl::cout << "allops:";
//             for (jobop_set_id_t::const_iterator it = jobOps.begin();
//                  it != jobOps.end(); it++)
//                 utl::cout <<  (*it)->id() << "("
//                           << ((JobOp*)(*it))->job()->id()
//                           << "), ";
//             utl::cout << utl::endl << utl::endl;
//             utl::cout << "allCGops:";
//             for (cg_set_id_t::iterator it = jobOpCGs.begin();
//                  it != jobOpCGs.end(); it++)
//             {
//                 CycleGroup* candCG = *it;
//                 utl::cout << candCG->toString() << " ";
//             }
//             utl::cout << utl::endl << utl::endl;
//             utl::cout << "allPreds:";
//             for (cg_revset_t::iterator it = allPredCGs.begin();
//                  it != allPredCGs.end(); it++)
//             {
//                 CycleGroup* candCG = *it;
//                 utl::cout << candCG->toString() << " ";
//             }
//             utl::cout << utl::endl << utl::endl;
//             utl::cout << "allSuccs:";
//             for (cg_revset_t::iterator it = allSuccCGs.begin();
//                  it != allSuccCGs.end(); it++)
//             {
//                 CycleGroup* candCG = *it;
//                 utl::cout << candCG->toString() << " ";
//             }
//             utl::cout << utl::endl << utl::endl;
//             utl::cout << "allcands:";
//             for (cg_set_id_t::iterator it = cgCandidates.begin();
//                  it != cgCandidates.end(); it++)
//             {
//                 CycleGroup* candCG = *it;
//                 utl::cout << candCG->toString() << " ";
//             }
//             utl::cout << utl::endl << utl::endl;

            const uint_set_t& opRess = act->allResIds();
            cg_set_id_t::iterator candIt;
            for (candIt = cgCandidates.begin();
                 candIt != cgCandidates.end();
                 ++candIt)
            {
                CycleGroup* candCG = *candIt;
                CycleGroup::iterator cbIt, cbEnd = candCG->end();
                for (cbIt = candCG->begin(); cbIt != cbEnd; ++cbIt)
                {
                    ConstrainedBound* cb = *cbIt;
                    //skip if (act != ptAct && act != intAct)
                    //or (op.frozen) or (op.pt == 0)
                    if (!cb->isA(cls::ESbound) &&
                        !cb->isA(cls::ESboundInt)) continue;
                    Activity* candidateAct
                        = (Activity*)cb->owner();
                    JobOp* candidateOp = (JobOp*)candidateAct->owner();
                    if (candidateOp->frozen()) continue;
                    if (candidateAct->isA(PtActivity))
                    {
                        PtActivity* candidatePtact = (PtActivity*)candidateAct;
                        const IntVar* candidatePtExp = candidatePtact->possiblePts();
                        if (candidatePtExp->isBound() &&
                            candidatePtExp->getValue() == 0) continue;
                    }

                    const uint_set_t& candOpRess
                        = candidateAct->allResIds();

                    uint_set_t tempUintVec;
                    std::set_intersection(
                        opRess.begin(), opRess.end(),
                        candOpRess.begin(), candOpRess.end(),
                        std::inserter(tempUintVec,tempUintVec.begin()));
                    //remove dumy resources from tempUintVec
                    for (uint_set_t::iterator resIt = tempUintVec.begin();
                         resIt != tempUintVec.end(); resIt++)
                    {
                        uint_t resId = *resIt;
                        DiscreteResource* dres =
                            dynamic_cast<DiscreteResource*>(dataSet->findResource(resId));
                        ASSERTD(dres != nullptr);
                        cls::DiscreteResource* clsRes =
                            (cls::DiscreteResource*)dres->clsResource();
                        if (dres->maxCap() == 0 || clsRes->maxReqCap() == 0)
                        {
                            tempUintVec.erase(resIt);
                            continue;
                        }

                    }
                    if (candidateOp != op && tempUintVec.size() > 0)
                    {
                        ASSERTD(op->job()->id() == candidateOp->job()->id());
                        opVect->push_back(candidateOp);

                        //checking code DON'T DELETE
//                         utl::cout << "    op(job):" << op->id()
//                                   << "(" << op->job()->id() << ")"
//                                   << ", opResIds:";
//                         for (uint_set_t::const_iterator it = opRess.begin();
//                              it != opRess.end();
//                              ++it)
//                         {
//                             utl::cout << *it << ", ";
//                         }
//                         utl::cout << "    swapOp(job):" << candidateOp->id()
//                                   << "(" << candidateOp->job()->id() << ")"
//                                   << ", ResIds:";
//                         uint_set_t::const_iterator it;
//                         for (it = candOpRess.begin();
//                              it != candOpRess.end();
//                              ++it)
//                         {
//                             utl::cout << *it << ",  ";
//                         }
//                         utl::cout << utl::endl;
                    }
                }
            }
            std::sort(opVect->begin(), opVect->end(), JobOpIdOrdering());
            _swapOps.push_back(opVect); // _swapOps
            jobNumChoices += opVect->size();
        }
        _jobNumChoices.push_back(jobNumChoices); //_jobNumChoices
    }
}

//////////////////////////////////////////////////////////////////////////////

void
JobOpSeqMutate::init()
{
    _moveJobIdx = uint_t_max;
    _moveOpSid = uint_t_max;
}

//////////////////////////////////////////////////////////////////////////////

void
JobOpSeqMutate::deInit()
{
    deleteCont(_swapOps);
}

//////////////////////////////////////////////////////////////////////////////

CSE_NS_END;
