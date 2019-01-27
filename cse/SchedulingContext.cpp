#include "libcse.h"
#include <libutl/BufferedFDstream.h>
#include <libutl/Time.h>
#include <clp/BoundCt.h>
#include <clp/FailEx.h>
#include <cls/CompositeResourceRequirement.h>
#include <cls/DiscreteResourceRequirement.h>
#include <cls/ESbound.h>
#include <cls/ESboundInt.h>
#include <cls/ResourceCalendarMgr.h>
#include <gop/ConfigEx.h>
#include "CompositeResource.h"
#include "ClevorDataSet.h"
#include "DiscreteResource.h"
#include "Propagator.h"
#include "RuleBasedScheduler.h"
#include "SchedulingContext.h"
#include "SummaryOp.h"

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

UTL_CLASS_IMPL(cse::SchedulingContext);

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

void
SchedulingContext::initialize(ClevorDataSet* dataSet)
{
    // forget old dataSet, remember new one
    if (_dataSetOwner)
        delete _dataSet;
    ;
    _dataSet = dataSet;

    // create the schedule and manager
    delete _schedule;
    delete _mgr;
    _mgr = new Manager();
    _bp = new Propagator(this);
    _mgr->setBoundPropagator(_bp);
    _schedule = new Schedule(_mgr);
    _sjobs.initialize(_mgr);

    // build the model (phase 0)
    _dataSet->modelBuild_0(_schedule);

    // grab config reference
    _config = dataSet->schedulerConfig();

    // set adjusted resource-cap-pts for all ops
    setResCapPtsAdj();

    // initialize summary ops
    initSummaryOps();

    // set status of all ops
    setCompletionStatus();
    setFrozenStatus();

    // build the model (phase 1)
    _dataSet->modelBuild_1();

    // unsuspend CGs with no predecessors
    _bp->unsuspendInitial();
    propagate();

    // schedule frozen ops
    scheduleFrozenOps();

    _afterInitialization = true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
SchedulingContext::clear()
{
    IndBuilderContext::clear();

    // backtrack to root choice-point and make new choice-point
    ASSERTD(_mgr->depth() <= 2);
    if (_mgr->depth() == 2)
    {
        _mgr->popState();
    }
    _mgr->pushState();
    ASSERTD(_mgr->depth() == 2);

    // init scheduling status
    setComplete(false);

    // restore makespan
    _makespan = _frozenMakespan;

    // init numScheduledOps
    _numScheduledOps = 0;

    // restore jobs / ops
    const job_set_id_t& jobs = _dataSet->jobs();
    job_set_id_t::const_iterator jobIt;
    for (jobIt = jobs.begin(); jobIt != jobs.end(); ++jobIt)
    {
        Job* job = *jobIt;
        //         if (!job->active())
        //         {
        //             job->release(_mgr);
        //         }
        job->scheduleClear();
        _mgr->revSet(job->schedulableJobsIdx());
        jobop_set_id_t::const_iterator opIt;
        jobop_set_id_t::const_iterator opsEnd = job->end();
        for (opIt = job->begin(); opIt != opsEnd; ++opIt)
        {
            JobOp* op = *opIt;
            _mgr->revSet(op->schedulableOpsIdx());
#ifdef DEBUG
            Activity* act = op->activity();
            if (act == nullptr)
                continue;
            ConstrainedBound* cb = act->esBound();
            ASSERTD(!cb->queued());
#endif
        }
    }

    // clear list of scheduled ops for each unary resource
    const res_set_id_t& resources = _dataSet->resources();
    res_set_id_t::const_iterator resIt;
    for (resIt = resources.begin(); resIt != resources.end(); ++resIt)
    {
        DiscreteResource* res = dynamic_cast<DiscreteResource*>(*resIt);
        if (res == nullptr)
            continue;
        res->sequenceRuleApplications().clear();
        cls::DiscreteResource* clsRes = (cls::DiscreteResource*)res->clsResource();
        if (clsRes == nullptr || clsRes->actsByStartTime().size() == 0)
            continue;
        act_set_es_t::iterator it = clsRes->actsByStartTime().begin();
        while (it != clsRes->actsByStartTime().end())
        {
            Activity* act = *it;
            JobOp* op = (JobOp*)act->owner();
            if (!op->frozen())
            {
                clsRes->actsByStartTime().erase(it++);
            }
            else
            {
                it++;
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
SchedulingContext::schedule(JobOp* op)
{
#ifdef DEBUG
    ASSERTD(op->schedulable());
#endif

    Activity* act = op->activity();
    ASSERTD(act != nullptr);

    // finalize the ES bound
    if (_config->forward())
    {
        _bp->finalize(act->esBound());
        _mgr->propagate();
        //         _makespan = utl::max(_makespan, act->ef() + 1);
    }
    else
    {
        _bp->finalize(act->lfBound());
        _mgr->propagate();
        //         _makespan = utl::max(_makespan, act->lf() + 1);
    }

#ifdef DEBUG_UNIT
    if (op->id() == 89 || op->id() == 66 || op->id() == 380 || op->id() == 58)
        utl::cout << "schedule op-" << op->id() << " @ "
                  << "[" << act->es() << "," << act->ef() << "]"
                  << ", {" << Time(timeSlotToTime(act->es())).toString() << ","
                  << Time(timeSlotToTime(act->ef())).toString() << utl::endlf;
#endif

    ++_numScheduledOps;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
SchedulingContext::sjobsAdd(Job* job)
{
    //     utl::cout << "SchedulingContext::sjobsAdd:" << job->id()
    //               << utl::endl;
    ASSERTD(job->schedulableJobsIdx() == uint_t_max);
    job->schedulableJobsIdx() = _sjobs.size();
    _sjobs.add(job);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
SchedulingContext::sjobsRemove(Job* job)
{
    //     utl::cout << "SchedulingContext::sjobsRemove:" << job->id()
    //               << utl::endl;
    ASSERTD(job->schedulableJobsIdx() != uint_t_max);
    uint_t idx = job->schedulableJobsIdx();
    uint_t endIdx = _sjobs.size() - 1;
    _sjobs.remove(idx);
    job->schedulableJobsIdx() = uint_t_max;
    if (idx < endIdx)
    {
        _sjobs[idx]->schedulableJobsIdx() = idx;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
SchedulingContext::store()
{
    ASSERT(complete());

    // re-set job status based job->active()
    const job_set_id_t& jobs = _dataSet->jobs();
    job_set_id_t::const_iterator jobIt;
    for (jobIt = jobs.begin(); jobIt != jobs.end(); ++jobIt)
    {
        Job* job = *jobIt;
        if (job->active())
        {
            if ((job->status() == jobstatus_inactive) || (job->status() == jobstatus_undefined))
            {
                job->status() = jobstatus_planned;
            }
        }
        else
        {
            job->status() = jobstatus_inactive;
        }
    }

    const jobop_set_id_t& ops = _dataSet->ops();
    jobop_set_id_t::const_iterator it;
    for (it = ops.begin(); it != ops.end(); ++it)
    {
        JobOp* op = *it;

        if (!op->job()->active())
            continue;
        // ignore summary op
        if (op->type() == op_summary)
            continue;

        // ignore ignorable op
        if (op->ignorable())
            continue;

        // ignore op that has no activity
        Activity* act = op->activity();
        if (act == nullptr)
            continue;

        ASSERT(act->ef() >= (act->es() - 1));

        // reference activity subtype
        BrkActivity* brkact = op->breakable() ? op->brkact() : nullptr;
        IntActivity* intact = op->interruptible() ? op->intact() : nullptr;

        // scheduling agent, start-time
        op->scheduledBy() = sa_clevor;
        if (op->status() == opstatus_started)
        {
            op->scheduledResumeTime() = timeSlotToTime(_config->forward() ? act->es() : act->lf());
        }
        else
        {
            op->scheduledStartTime() = timeSlotToTime(_config->forward() ? act->es() : act->ls());
            op->scheduledResumeTime() = -1;
        }

        // end-time
        op->scheduledEndTime() = timeSlotToTime(_config->forward() ? act->ef() + 1 : act->lf() + 1);

        //if (!op->schedulable()) continue;

        // processing-time
        uint_t pt = uint_t_max;
        if (brkact != nullptr)
        {
            if (&(brkact->possiblePts()) == nullptr)
            {
                String& str = *new String();
                str = "scheduling error detected for op ";
                str += Uint(op->id()).toString();
                throw FailEx(str);
            }
            pt = brkact->possiblePts().getValue();
        }
        if (intact != nullptr)
            pt = intact->processingTime();
        if (pt == uint_t_max)
        {
            op->scheduledProcessingTime() = (op->scheduledEndTime() - op->scheduledStartTime());
        }
        else
        {
            if (op->status() == opstatus_started)
            {
                op->scheduledRemainingPt() = _config->timeSlotToDuration(pt);
            }
            else
            {
                op->scheduledProcessingTime() = _config->timeSlotToDuration(pt);
                op->scheduledRemainingPt() = uint_t_max;
            }
        }

        // remove res-reqs created by system (for non-frozen or interruptible)
        if (!op->frozen() || (intact != nullptr))
        {
            op->removeSystemResReqs();
        }

        // record scheduled-capacity for discrete-res-reqs
        uint_t numResReqs = op->numResReqs();
        uint_t i;
        for (i = 0; i < numResReqs; ++i)
        {
            cse::ResourceRequirement* cseResReq = op->getResReq(i);
            Object* clsResReq = cseResReq->clsResReq();
            if (clsResReq == nullptr)
                continue;
            if (!clsResReq->isA(cls::DiscreteResourceRequirement))
                continue;
            cls::DiscreteResourceRequirement* clsDRR = (cls::DiscreteResourceRequirement*)clsResReq;
            if (clsDRR == nullptr)
                continue;
            const ResourceCapPts* resCapPts = clsDRR->resCapPts();
            ASSERT(pt != uint_t_max);
            resCapPts->selectPt(pt);
            cseResReq->scheduledCapacity() = resCapPts->selectedCap();
        }

        // record resource requirements for interruptible activity
        cls::Resource** resourcesArray = _schedule->resourcesArray();
        if (intact != nullptr)
        {
            IntActivity::revarray_t** allocations;
            uint_t numAllocations;
            intact->getAllocations(allocations, numAllocations);
            for (uint_t j = 0; j < numAllocations; ++j)
            {
                if (allocations[j] == nullptr)
                    continue;
                IntActivity::revarray_t& allocs = *allocations[j];
                uint_t resId = resourcesArray[j]->id();
                uint_t numSpans = allocs.size() / 2;
                uint_t idx = 0;
                for (uint_t k = 0; k < numSpans; ++k)
                {
                    time_t beginTime = timeSlotToTime(allocs[idx++]);
                    time_t endTime = timeSlotToTime(allocs[idx++] + 1);
                    cse::ResourceRequirement* resReq = new ResourceRequirement();
                    resReq->resourceId() = resId;
                    resReq->capacity() = 100;
                    resReq->scheduledCapacity() = 100;
                    resReq->beginTime() = beginTime;
                    resReq->endTime() = endTime;
                    resReq->isSystem() = true;
                    op->addResReq(resReq);
                }
            }
        }

        // do nothing else if op is frozen
        if (op->frozen())
            continue;

        // record resource selections for res-group-reqs
        uint_t numResGroupReqs = op->numResGroupReqs();
        for (i = 0; i < numResGroupReqs; ++i)
        {
            cse::ResourceGroupRequirement* cseResGroupReq = op->getResGroupReq(i);
            cls::DiscreteResourceRequirement* clsResReq = cseResGroupReq->clsResReq();
            if (clsResReq == nullptr)
                continue;
            const clp::IntVar& selectedResources = clsResReq->selectedResources();
            uint_t resId = selectedResources.getValue();
            const ResourceCapPts* resCapPts = clsResReq->resCapPts(resId);
            resCapPts->selectPt(pt);
            cseResGroupReq->scheduledResourceId() = resCapPts->resourceId();
            cseResGroupReq->scheduledCapacity() = resCapPts->selectedCap();

            // add res-req
            cse::ResourceRequirement* resReq = new ResourceRequirement();
            resReq->resourceId() = cseResGroupReq->scheduledResourceId();
            resReq->capacity() = cseResGroupReq->scheduledCapacity();
            resReq->scheduledCapacity() = cseResGroupReq->scheduledCapacity();
            resReq->isSystem() = true;
            op->addResReq(resReq);
        }
    }

    // handle summary ops separately
    const TRBtree<JobOp>& summaryOpsIncSD = _dataSet->summaryOpsIncSD();
    TRBtreeIt<JobOp> incIt;
    for (incIt = summaryOpsIncSD.begin(); incIt != summaryOpsIncSD.end(); ++incIt)
    {
        SummaryOp* op = (SummaryOp*)*incIt;
        op->unschedule();

        // set start-time and end-time based on child ops
        jobop_set_id_t::const_iterator succIt;
        for (succIt = op->childOps().begin(); succIt != op->childOps().end(); ++succIt)
        {
            JobOp* succOp = *succIt;

            // ignore unscheduled child
            if (!succOp->isScheduled())
            {
                continue;
            }

            op->scheduledBy() = sa_clevor;

            // summary.start = min(summary.start, child.start)
            if ((op->scheduledStartTime() == -1) ||
                (succOp->scheduledStartTime() < op->scheduledStartTime()))
            {
                op->scheduledStartTime() = succOp->scheduledStartTime();
            }

            // summary.end = max(summary.end, child.end)
            if ((op->scheduledEndTime() == -1) ||
                (succOp->scheduledEndTime() > op->scheduledEndTime()))
            {
                op->scheduledEndTime() = succOp->scheduledEndTime();
            }
        }

        // processing-time of summary op is (endTime - startTime)
        if (op->scheduledStartTime() != -1)
        {
            ASSERT(op->scheduledEndTime() != -1);
            op->scheduledProcessingTime() = (op->scheduledEndTime() - op->scheduledStartTime());
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
SchedulingContext::init()
{
    _dataSet = nullptr;
    _mgr = nullptr;
    _schedule = nullptr;
    _complete = false;
    _afterInitialization = false;
    _config = nullptr;
    _makespan = 0;
    _frozenMakespan = 0;
    _hardCtScore = 0;
    _numScheduledOps = 0;
    _dataSetOwner = true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
SchedulingContext::deInit()
{
    if (_dataSetOwner)
        delete _dataSet;
    delete _schedule;
    delete _mgr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
SchedulingContext::setResCapPtsAdj()
{
    const jobop_set_id_t& ops = _dataSet->ops();
    jobop_set_id_t::const_iterator it;
    for (it = ops.begin(); it != ops.end(); ++it)
    {
        JobOp* op = *it;
        op->setResCapPtsAdj(_config->timeStep());
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
SchedulingContext::initSummaryOps()
{
    const pct_vector_t& pcts = _dataSet->precedenceCts();
    pct_vector_t::const_iterator pctIt;
    for (pctIt = pcts.begin(); pctIt != pcts.end(); ++pctIt)
    {
        PrecedenceCt* pct = *pctIt;
        JobOp* lhsOp = _dataSet->findOp(pct->lhsOpId());
        JobOp* rhsOp = _dataSet->findOp(pct->rhsOpId());
        ASSERTD(lhsOp != nullptr);
        ASSERTD(rhsOp != nullptr);

        // lhsOp must be summary op
        if (lhsOp->type() != op_summary)
            continue;
        SummaryOp* lhsSummary = (SummaryOp*)lhsOp;

        if (pct->type() != pct_ss)
            continue;

        // lhsOp is parent of rhsOp
        lhsSummary->childOps().insert(rhsOp);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
SchedulingContext::setCompletionStatus()
{
    const jobop_set_id_t& ops = _dataSet->ops();
    const TRBtree<JobOp>& opsDecSD = _dataSet->opsDecSD();
    TRBtreeIt<JobOp> opsDecSDend = opsDecSD.end();

    // initially:
    //     o mark precedence-lag as complete
    //     o mark summary as unstarted
    jobop_set_id_t::const_iterator it;
    for (it = ops.begin(); it != ops.end(); ++it)
    {
        JobOp* op = *it;

        // unschedule op that is improperly scheduled
        if (!op->isScheduled() || (op->type() == op_precedenceLag))
        {
            op->unschedule();
        }

        // precedence-lag is marked as complete
        if (op->type() == op_precedenceLag)
        {
            op->status() = opstatus_complete;
        }
        // summary is marked as unstarted
        else if (op->type() == op_summary)
        {
            op->status() = opstatus_unstarted;
        }
    }

    // successors of incomplete ops are unstarted
    TRBtreeIt<JobOp> dsdIt;
    for (dsdIt = opsDecSD.begin(); dsdIt != opsDecSDend; ++dsdIt)
    {
        JobOp* op = *dsdIt;

        // skip summary op
        if ((op->type() == op_summary) || (op->status() == opstatus_complete))
        {
            continue;
        }

        CycleGroup* cg = op->esCG();
        const cg_revset_t& succCGs = cg->allSuccCGs();
        cg_revset_t::iterator cgIt;
        for (cgIt = succCGs.begin(); cgIt != succCGs.end(); ++cgIt)
        {
            CycleGroup* succCG = *cgIt;
            CycleGroup::iterator cbIt;
            for (cbIt = succCG->begin(); cbIt != succCG->end(); ++cbIt)
            {
                ConstrainedBound* cb = *cbIt;
                Activity* succAct = (Activity*)cb->owner();
                if (succAct == nullptr)
                    continue;
                JobOp* succOp = (JobOp*)succAct->owner();
                if (succOp == nullptr)
                    continue;

                // mark as unstarted
                if ((succOp->status() != opstatus_unstarted) &&
                    (succOp->type() != op_precedenceLag))
                {
                    std::cout << "marking op " << succOp->id() << " " << succOp->name()
                              << " unstarted due to "
                              << "incomplete predecessor" << std::endl;
                }
                succOp->status() = opstatus_unstarted;
            }
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////
    // note: summary ops were all marked as unstarted
    //
    // for each summary op:
    //     - mark it as complete if all its children are complete
    //     - mark it as started if at least one of its children has started
    /////////////////////////////////////////////////////////////////////////////////////////////////

    // iterate over summary ops in order of increasing summary depth
    TRBtreeIt<JobOp> incIt;
    const TRBtree<JobOp>& summaryOpsIncSD = _dataSet->summaryOpsIncSD();
    for (incIt = summaryOpsIncSD.begin(); incIt != summaryOpsIncSD.end(); ++incIt)
    {
        SummaryOp* op = (SummaryOp*)*incIt;

        bool allComplete = true;
        bool anyStarted = false;

        // iterate over successors
        jobop_set_id_t::const_iterator succIt;
        for (succIt = op->childOps().begin(); succIt != op->childOps().end(); ++succIt)
        {
            JobOp* childOp = *succIt;

            allComplete = allComplete && (childOp->status() == opstatus_complete);

            anyStarted = anyStarted || (childOp->status() == opstatus_started);
        }

        if (allComplete)
            op->status() = opstatus_complete;
        else if (anyStarted)
            op->status() = opstatus_started;
    }

    //
    // for each completed precedenceLag op:
    //     o if its successor is started, post a unary-ct for it
    //
    for (it = ops.begin(); it != ops.end(); ++it)
    {
        JobOp* op = *it;

        // skip op that is not precedenceLag type
        if ((op->type() != op_precedenceLag) || (op->status() != opstatus_complete))
        {
            continue;
        }

        // find the precedence-ct linking to successor
        JobOp* succOp = nullptr;
        const pct_vector_t& precedenceCts = _dataSet->precedenceCts();
        pct_vector_t::const_iterator it;
        for (it = precedenceCts.begin(); it != precedenceCts.end(); ++it)
        {
            PrecedenceCt* pct = *it;
            if (pct->lhsOpId() != op->id())
                continue;
            succOp = _dataSet->findOp(pct->rhsOpId());
            break;
        }

        // couldn't find successor op => do nothing else
        if (succOp == nullptr)
        {
            continue;
        }

        // if successor is started, then post unary-ct
        if (succOp->status() == opstatus_started)
        {
            ASSERTD(succOp->scheduledStartTime() != -1);
            UnaryCt* unaryCt = new UnaryCt(uct_startNoSoonerThan, succOp->scheduledStartTime());
            succOp->add(unaryCt);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
SchedulingContext::setFrozenStatus()
{
    const jobop_set_id_t& ops = _dataSet->ops();
    const TRBtree<JobOp>& opsDecSD = _dataSet->opsDecSD();
    TRBtreeIt<JobOp> opsDecSDend = opsDecSD.end();
    time_t originTime = _config->originTime();
    time_t autoFreezeTime = originTime + _config->autoFreezeDuration();

    // predecessors of frozen ops are frozen (initially)
    TRBtreeIt<JobOp> dsdIt;
    for (dsdIt = opsDecSD.begin(); dsdIt != opsDecSDend; ++dsdIt)
    {
        JobOp* op = *dsdIt;

        // manually frozen?
        op->frozen() = op->frozen() || op->manuallyFrozen();

        // skip summary op
        if ((op->type() == op_summary) || !op->frozen())
            continue;

        // not frozen => successors not frozen
        CycleGroup* cg = op->esCG();
        const cg_revset_t& predCGs = cg->allPredCGs();
        cg_revset_t::iterator cgIt;
        for (cgIt = predCGs.begin(); cgIt != predCGs.end(); ++cgIt)
        {
            CycleGroup* predCG = *cgIt;
            CycleGroup::iterator cbIt;
            for (cbIt = predCG->begin(); cbIt != predCG->end(); ++cbIt)
            {
                ConstrainedBound* cb = *cbIt;
                Activity* predAct = (Activity*)cb->owner();
                ASSERTD(predAct != nullptr);
                JobOp* predOp = (JobOp*)predAct->owner();
                ASSERTD(predOp != nullptr);
                predOp->frozen() = true;
            }
        }
    }

    // set frozen status of ops
    jobop_set_id_t::const_iterator it;
    for (it = ops.begin(); it != ops.end(); ++it)
    {
        JobOp* op = *it;

        time_t sst = op->scheduledStartTime();

        // no frozen ops when backward scheduling
        if (_config->backward())
        {
            goto nofreeze;
        }

        // summary op is not frozen
        if (op->type() == op_summary)
        {
            goto nofreeze;
        }

        // useInitialAsSeed?
        if (_config->useInitialAsSeed())
        {
            goto freeze;
        }

        // unscheduled op is not frozen (except precedence-lag)
        if (!op->isScheduled() && (op->type() != op_precedenceLag))
        {
            goto nofreeze;
        }

        // frozen?
        if (op->frozen())
        {
            goto freeze;
        }

        // started or completed?
        if ((op->status() == opstatus_started) || (op->status() == opstatus_complete))
        {
            goto freeze;
        }
        else if (op->status() == opstatus_unstarted)
        {
            // op must start within [origin,autoFreezeTime)
            if ((autoFreezeTime == originTime) || (sst >= autoFreezeTime))
            {
                goto nofreeze;
            }
        }
    freeze:
        op->frozen() = true;
        continue;
    nofreeze:
        op->frozen() = false;
        op->unschedule();
    }

    // successors of unfrozen ops are unfrozen
    for (dsdIt = opsDecSD.begin(); dsdIt != opsDecSDend; ++dsdIt)
    {
        JobOp* op = *dsdIt;

        // skip summary op
        if ((op->type() == op_summary) || op->frozen())
            continue;

        // not frozen => successors not frozen
        CycleGroup* cg = op->esCG();
        const cg_revset_t& succCGs = cg->allSuccCGs();
        cg_revset_t::iterator cgIt;
        for (cgIt = succCGs.begin(); cgIt != succCGs.end(); ++cgIt)
        {
            CycleGroup* succCG = *cgIt;
            CycleGroup::iterator cbIt;
            for (cbIt = succCG->begin(); cbIt != succCG->end(); ++cbIt)
            {
                ConstrainedBound* cb = *cbIt;
                Activity* succAct = (Activity*)cb->owner();
                ASSERTD(succAct != nullptr);
                JobOp* succOp = (JobOp*)succAct->owner();
                ASSERTD(succOp != nullptr);
                succOp->frozen() = false;
            }
        }
    }

    // post-conditions
    for (it = ops.begin(); it != ops.end(); ++it)
    {
        JobOp* op = *it;

        // skip summary op
        if (op->type() == op_summary)
            continue;

#ifdef DEBUG_UNIT
//         if (op->frozen() && op->status() != opstatus_complete)
//         {
//             utl::cout << "frozen_op id:" << op->id()
//                       << ",name:" << op->name().c_str()
//                       << ",status " << op->status() << utl::endlf;
//         }
#endif

        switch (op->status())
        {
        case opstatus_started:
            if (!op->frozen())
            {
                utl::cout << "WARNING: started op is not frozen: " << op->id() << " "
                          << op->name().c_str() << utl::endlf;
            }
            ASSERT(op->frozen());
            if (!op->isScheduled())
            {
                utl::cout << "WARNING: started op is not scheduled: " << op->id() << " "
                          << op->name().c_str() << utl::endlf;
            }
            ASSERT(op->isScheduled());
            break;
        case opstatus_complete:
            if (!op->frozen())
            {
                utl::cout << "WARNING: completed op is not frozen: " << op->id() << " "
                          << op->name().c_str() << utl::endlf;
            }
            ASSERT(op->frozen());
            if ((op->type() != op_precedenceLag) && (!op->isScheduled()))
            {
                utl::cout << "WARNING: completed op is not scheduled: " << op->id() << " "
                          << op->name().c_str() << utl::endlf;
            }
            ASSERT(op->isScheduled() || (op->type() == op_precedenceLag));
            break;
        case opstatus_unstarted:
            break;
        default:
            ABORT();
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
SchedulingContext::scheduleFrozenOps()
{
    FrozenOpScheduler frozenOpScheduler;
    frozenOpScheduler.run(this);
#ifdef DEBUG_UNIT
    utl::cout << "END OF SCHEDULEFROZENOPS." << utl::endlf;
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
SchedulingContext::setComplete(bool complete)
{
    _complete = complete;
    if (_complete & _afterInitialization)
        onComplete();
}
////////////////////////////////////////////////////////////////////////////////////////////////////

void
SchedulingContext::onComplete()
{
    ASSERTD(_sjobs.size() == 0);
#ifdef DEBUG_UNIT
    utl::cout << "This run is completed!" << utl::endlf;
#endif
    _hardCtScore = _dataSet->hardCtScore();

    if (!_dataSet->resourceSequenceLists().empty())
    {
        postUnaryResourceFS();
        findResourceSequenceRuleApplications();
        // Note: postResourceSequenceDelays has design fault. It can cause resource
        //       over-allocation. However, it MAY still work for datasets which ONLY
        //       use unary resources. So for datasets which have multiple capacity
        //       resources, you have to set all delays to zero in order to avoid
        //       resource over-allocation.
        // Joe, Dec 3, 2009
        postResourceSequenceDelays();
    }
    calculateMakespan();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
SchedulingContext::calculateMakespan()
{
    const job_set_id_t& jobs = _dataSet->jobs();
    job_set_id_t::const_iterator jobIt;
    bool forward = _config->forward();
    for (jobIt = jobs.begin(); jobIt != jobs.end(); ++jobIt)
    {
        Job* job = *jobIt;
        if (job->id() == 0)
            continue;
        job->calculateMakespan(forward);
        _makespan = utl::max(_makespan, job->makespan());
        _frozenMakespan = utl::max(_frozenMakespan, job->frozenMakespan());
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
SchedulingContext::postUnaryResourceFS()
{
    // iterate over unary resources
    const res_set_id_t& resources = _dataSet->resources();
    res_set_id_t::const_iterator resIt;
    for (resIt = resources.begin(); resIt != resources.end(); ++resIt)
    {
        // find resource and its sequence-list
        DiscreteResource* res = dynamic_cast<DiscreteResource*>(*resIt);
        if (res == nullptr)
            continue;
        cls::DiscreteResource* clsRes = (cls::DiscreteResource*)res->clsResource();
        if ((clsRes == nullptr) || !clsRes->isUnary())
            continue;

        // iterate over activities scheduled on unary resource
        // add F-S relationship for all activities
        Activity* lhsAct = nullptr;
        const act_set_es_t& acts = clsRes->actsByStartTime();
        act_set_es_t::const_iterator actIt;
        for (actIt = acts.begin(); actIt != acts.end(); ++actIt)
        {
            Activity* rhsAct = *actIt;

            if (lhsAct == nullptr)
            {
                lhsAct = rhsAct;
                continue;
            }

            // impose the F-S delay
            if (_config->forward())
            {
                if (lhsAct->ef() <= rhsAct->es())
                {
                    _bp->addBoundCt(lhsAct->efBound(), rhsAct->esBound(), 1, false);
                    lhsAct = rhsAct;
                }
            }
            else
            {
                if (rhsAct->ls() >= lhsAct->lf())
                {
                    _bp->addBoundCt(rhsAct->lsBound(), lhsAct->lfBound(), -1, false);
                    lhsAct = rhsAct;
                }
            }
        }
    }
    _mgr->propagate();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
SchedulingContext::findResourceSequenceRuleApplications()
{
    // iterate over unary resources
    const res_set_id_t& resources = _dataSet->resources();
    res_set_id_t::const_iterator resIt;
    for (resIt = resources.begin(); resIt != resources.end(); ++resIt)
    {
        // find resource and its sequence-list
        DiscreteResource* res = dynamic_cast<DiscreteResource*>(*resIt);
        if (res == nullptr)
            continue;
        const ResourceSequenceList* rsl = res->sequenceList();
        if (rsl == nullptr)
            continue;
        cls::DiscreteResource* clsRes = (cls::DiscreteResource*)res->clsResource();
        if ((clsRes == nullptr) || !clsRes->isUnary())
            continue;

            // uncomment to see the activities scheduled on the resource
#ifdef DEBUG_UNIT
        utl::cout << utl::endlf << "SchedulingContext::findResourceSequenceRuleApplications()"
                  << utl::endlf;
        const act_set_es_t& debug_acts = clsRes->actsByStartTime();
        act_set_es_t::const_iterator debug_it;
        for (debug_it = debug_acts.begin(); debug_it != debug_acts.end(); ++debug_it)
        {
            Activity* act = *debug_it;
            JobOp* op = (JobOp*)act->owner();
            utl::cout << "res:" << res->id() << ", act:" << act->id()
                      << ", seqId:" << op->sequenceId() << ", es=" << act->es()
                      << ", ef=" << act->ef() << utl::endl;
        }
#endif
        // iterate over activities scheduled on unary resource
        // assessing delays based on rules in sequence-list
        Activity* lhsAct = nullptr;
        const act_set_es_t& acts = clsRes->actsByStartTime();
        act_set_es_t::const_iterator actIt;
        for (actIt = acts.begin(); actIt != acts.end(); ++actIt)
        {
            Activity* rhsAct = *actIt;

            if (lhsAct == nullptr)
            {
                lhsAct = rhsAct;
                continue;
            }

            JobOp* lhsOp = (JobOp*)lhsAct->owner();
            JobOp* rhsOp = (JobOp*)rhsAct->owner();

            // do nothing if ops have same sequence-id AND same jobIds
            // Joe added the same jobIds check, because it's needed by
            // the Bioriginal dataset. this check can be removed when we
            // rewrite code for moving time/cost in the future.
            // Dec 1, 2009
            uint_t lhsosid = lhsOp->sequenceId();
            uint_t rhsosid = rhsOp->sequenceId();
            if (lhsosid == rhsosid && lhsOp->job()->id() == rhsOp->job()->id())
            {
                lhsAct = rhsAct;
                continue;
            }

            // find the most specific matching rule
            const ResourceSequenceRule* rule = rsl->findRule(lhsosid, rhsosid);
            if (rule == nullptr)
            {
                lhsAct = rhsAct;
                continue;
            }
            res->sequenceRuleApplications().push_back(
                ResourceSequenceRuleApplication(lhsOp, rhsOp, rule));

            lhsAct = rhsAct;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
SchedulingContext::postResourceSequenceDelays()
{
    // iterate over discrete resources
    bool delayImposed = false;
    std::vector<cls::DiscreteResource*> clsResources;
    const res_set_id_t& resources = _dataSet->resources();
    res_set_id_t::const_iterator resIt;
    for (resIt = resources.begin(); resIt != resources.end(); ++resIt)
    {
        // iterate over sequence-rule-applications for the resource
        DiscreteResource* res = dynamic_cast<DiscreteResource*>(*resIt);
        if (res == nullptr)
            continue;
        cls::DiscreteResource* clsRes = (cls::DiscreteResource*)res->clsResource();
        if ((clsRes == nullptr) || !clsRes->isUnary())
            continue;
        const rsra_vector_t& rsras = res->sequenceRuleApplications();

        // add an extra capcity for every relevant unary resource
        if (rsras.size() > 0)
        {
            clsResources.push_back(clsRes);
            clsRes->doubleProvidedCap();
        }

        rsra_vector_t::const_iterator raIt;
        for (raIt = rsras.begin(); raIt != rsras.end(); ++raIt)
        {
            const ResourceSequenceRuleApplication& app = *raIt;
            const ResourceSequenceRule* rule = app.rule();
            uint_t delay = _config->durationToTimeSlot(rule->delay());
            //             uint_t delay = rule->delay() / _config->timeStep();
            if (delay == 0)
                continue;
            ++delay;
            cls::Activity* lhsAct = app.lhsOp()->activity();
            cls::Activity* rhsAct = app.rhsOp()->activity();
            if ((lhsAct == nullptr) || (rhsAct == nullptr))
                continue;
#ifdef DEBUG_UNIT
            utl::cout << utl::endlf << "SchedulingContext::postResourceSequenceDelays()"
                      << utl::endlf;
            utl::cout << "res:" << clsRes->id() << ", lhsAct:" << lhsAct->id()
                      << ", rhsAct:" << rhsAct->id() << ", delay:" << delay << utl::endlf;
#endif
            // impose the F-S delay
            delayImposed = true;
            if (_config->forward())
            {
                _bp->addBoundCt(lhsAct->efBound(), rhsAct->esBound(), delay, false);
            }
            else
            {
                _bp->addBoundCt(rhsAct->lsBound(), lhsAct->lfBound(), -delay, false);
            }
        }
    }

    // if we imposed any delays, propagate them and update makespan
    if (delayImposed)
    {
        // propagate
        _mgr->propagate();
    }

    // take the added extra capacity away
    for (uint_t i = 0; i < clsResources.size(); i++)
    {
        cls::DiscreteResource* res = clsResources[i];
        res->halveProvidedCap();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
SchedulingContext::propagate()
{
    _mgr->propagate();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;
