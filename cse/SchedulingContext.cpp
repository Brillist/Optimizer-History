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
//#define DEBUG_UNIT
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
    {
        delete _dataSet;
    }
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

    // unsuspend CGs with no predecessors, propagate
    _bp->unsuspendInitial();
    propagate();

    // schedule frozen ops
    scheduleFrozenOps();

    _initialized = true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
SchedulingContext::clear()
{
    super::clear();

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
    auto& jobs = _dataSet->jobs();
    for (auto job : jobs)
    {
        job->scheduleClear();
        _mgr->revSet(job->schedulableJobsIdx());
        for (auto op : *job)
        {
            _mgr->revSet(op->schedulableOpsIdx());
#ifdef DEBUG
            auto act = op->activity();
            if (act != nullptr)
            {
                auto& cb = act->esBound();
                ASSERT(!cb.queued());
            }
#endif
        }
    }

    // iterate over DiscreteResources
    auto& resources = _dataSet->resources();
    for (auto res_ : resources)
    {
        // not a DiscreteResource -> skip
        auto res = dynamic_cast<DiscreteResource*>(res_);
        if (res == nullptr)
        {
            continue;
        }

        // clear res's ResourceSequenceRuleApplications
        res->sequenceRuleApplications().clear();

        // res has no scheduled activities -> skip
        auto clsRes = res->clsResource();
        if ((clsRes == nullptr) || (clsRes->actsByStartTime().size() == 0))
        {
            continue;
        }

        // remove non-frozen activities from res's activities-by-start-time
        auto it = clsRes->actsByStartTime().begin();
        while (it != clsRes->actsByStartTime().end())
        {
            auto act = *it;
            auto op = utl::cast<JobOp>(act->owner());
            if (op->frozen())
            {
                ++it;
            }
            else
            {
                it = clsRes->actsByStartTime().erase(it);
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
SchedulingContext::schedule(JobOp* op)
{
    ASSERTD(op->schedulable());

    // act = op's activity (it MUST exist)
    auto act = op->activity();
    ASSERTD(act != nullptr);

    // finalize the ES bound
    if (_config->forward())
    {
        _bp->finalize(act->esBound());
        _mgr->propagate();
    }
    else
    {
        _bp->finalize(act->lfBound());
        _mgr->propagate();
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
SchedulingContext::store()
{
    ASSERT(complete());

    // set job status
    auto& jobs = _dataSet->jobs();
    for (auto job : jobs)
    {
        // job is active?
        if (job->active())
        {
            // status is inactive or undefined -> set to planned
            if ((job->status() == jobstatus_inactive) || (job->status() == jobstatus_undefined))
            {
                job->status() = jobstatus_planned;
            }
        }
        else // inactive
        {
            // set job status to inactive
            job->status() = jobstatus_inactive;
        }
    }

    // iterate over ops
    auto& ops = _dataSet->ops();
    for (auto op : ops)
    {
        // op's job is not active -> skip
        if (!op->job()->active())
        {
            continue;
        }

        // op is summary -> skip
        if (op->type() == op_summary)
        {
            continue;
        }

        // op is ignorable -> skip
        if (op->ignorable())
        {
            continue;
        }

        // op has no activity -> skip
        auto act = op->activity();
        if (act == nullptr)
        {
            continue;
        }

        ASSERT(act->ef() >= (act->es() - 1));

        // reference activity subtype
        auto brkact = op->breakable() ? op->brkact() : nullptr;
        auto intact = op->interruptible() ? op->intact() : nullptr;

        // set scheduling agent, scheduled-start-time, scheduled-resume-time
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

        // pt = activity's processing-time
        uint_t pt = uint_t_max;
        if (brkact != nullptr)
        {
            // breakable activity
            if (&brkact->possiblePts() == nullptr)
            {
                throw FailEx("scheduling error detected for op " + Uint(op->id()).toString());
            }
            pt = brkact->possiblePts().value();
        }
        else if (intact != nullptr)
        {
            // interruptible activity
            pt = intact->processingTime();
        }

        // set op's scheduled-processing-time or scheduled-remaining-pt
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

        // non-frozen and non-interruptible op -> remove resource requirements created by system
        if (!op->frozen() || (intact != nullptr))
        {
            op->removeSystemResReqs();
        }

        // record scheduled-capacity for discrete-res-reqs
        uint_t numResReqs = op->numResReqs();
        for (uint_t resReqIdx = 0; resReqIdx != numResReqs; ++resReqIdx)
        {
            auto cseResReq = op->getResReq(resReqIdx);

            // not DiscreteResourceRequirement -> skip
            auto clsDRR = dynamic_cast<cls::DiscreteResourceRequirement*>(cseResReq->clsResReq());
            if (clsDRR == nullptr)
            {
                continue;
            }

            // select the processing time in ResourceCapPts
            auto resCapPts = clsDRR->resCapPts();
            ASSERT(pt != uint_t_max);
            resCapPts->selectPt(pt);

            // record the scheduled capacity in cseResReq
            cseResReq->scheduledCapacity() = resCapPts->selectedCap();
        }

        // record resource requirements for interruptible activity
        auto resourcesArray = _schedule->resourcesArray();
        if (intact != nullptr)
        {
            uint_t numAllocations;
            IntActivity::revarray_uint_t** allocations;
            intact->getAllocations(allocations, numAllocations);
            for (uint_t allocIdx = 0; allocIdx != numAllocations; ++allocIdx)
            {
                if (allocations[allocIdx] == nullptr)
                {
                    continue;
                }
                auto& allocs = *allocations[allocIdx];
                uint_t resId = resourcesArray[allocIdx]->id();
                uint_t numSpans = allocs.size() / 2;
                uint_t idx = 0;
                for (uint_t spanIdx = 0; spanIdx != numSpans; ++spanIdx)
                {
                    time_t beginTime = timeSlotToTime(allocs[idx++]);
                    time_t endTime = timeSlotToTime(allocs[idx++] + 1);
                    auto resReq = new ResourceRequirement();
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
        {
            continue;
        }

        // resource-group-requirements: record resource selections
        uint_t numResGroupReqs = op->numResGroupReqs();
        for (uint_t resGroupReqIdx = 0; resGroupReqIdx != numResGroupReqs; ++resGroupReqIdx)
        {
            // reference this ResourceGroupRequirement's cls::ResourceRequirement
            auto cseResGroupReq = op->getResGroupReq(resGroupReqIdx);
            auto clsResReq = cseResGroupReq->clsResReq();
            if (clsResReq == nullptr)
            {
                continue;
            }

            // 
            auto& selectedResources = clsResReq->selectedResources();
            uint_t resId = selectedResources.value();
            auto resCapPts = clsResReq->resCapPts(resId);
            resCapPts->selectPt(pt);
            cseResGroupReq->scheduledResourceId() = resCapPts->resourceId();
            cseResGroupReq->scheduledCapacity() = resCapPts->selectedCap();

            // add res-req
            auto resReq = new ResourceRequirement();
            resReq->resourceId() = cseResGroupReq->scheduledResourceId();
            resReq->capacity() = cseResGroupReq->scheduledCapacity();
            resReq->scheduledCapacity() = cseResGroupReq->scheduledCapacity();
            resReq->isSystem() = true;
            op->addResReq(resReq);
        }
    }

    // handle summary ops separately
    auto& summaryOpsIncSD = _dataSet->summaryOpsIncSD();
    for (auto op_ : summaryOpsIncSD)
    {
        auto op = utl::cast<SummaryOp>(op_);

        // unschedule the summary op
        op->unschedule();

        // set start-time and end-time based on child ops
        for (auto succOp : op->childOps())
        {
            // ignore unscheduled child
            if (!succOp->isScheduled())
            {
                continue;
            }

            // set scheduling agent
            op->scheduledBy() = sa_clevor;

            // summary.scheduledStart = min(summary.start, child.start)
            if ((op->scheduledStartTime() == -1) ||
                (succOp->scheduledStartTime() < op->scheduledStartTime()))
            {
                op->scheduledStartTime() = succOp->scheduledStartTime();
            }

            // summary.scheduledEnd = max(summary.end, child.end)
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
SchedulingContext::setComplete(bool complete)
{
    _complete = complete;
    if (_complete & _initialized)
    {
        onComplete();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
SchedulingContext::sjobsAdd(Job* job)
{
    ASSERTD(job->schedulableJobsIdx() == uint_t_max);
    job->schedulableJobsIdx() = _sjobs.size();
    _sjobs.add(job);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
SchedulingContext::sjobsRemove(Job* job)
{
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
SchedulingContext::init()
{
    _dataSet = nullptr;
    _mgr = nullptr;
    _bp = nullptr;
    _schedule = nullptr;
    _dataSetOwner = true;
    _initialized = false;
    _complete = false;
    _config = nullptr;
    _makespan = 0;
    _frozenMakespan = 0;
    _hardCtScore = 0;
    _numScheduledOps = 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
SchedulingContext::deInit()
{
    if (_dataSetOwner)
    {
        delete _dataSet;
    }
    delete _schedule;
    delete _mgr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
SchedulingContext::setResCapPtsAdj()
{
    auto& ops = _dataSet->ops();
    for (auto op : ops)
    {
        op->setResCapPtsAdj(_config->timeStep());
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
SchedulingContext::initSummaryOps()
{
    auto& pcts = _dataSet->precedenceCts();
    for (auto pct : pcts)
    {
        auto lhsOp = _dataSet->findOp(pct->lhsOpId());
        auto rhsOp = _dataSet->findOp(pct->rhsOpId());
        ASSERTD(lhsOp != nullptr);
        ASSERTD(rhsOp != nullptr);

        // lhsOp is not summary -> skip
        if (lhsOp->type() != op_summary)
        {
            continue;
        }
        auto lhsSummary = utl::cast<SummaryOp>(lhsOp);

        // not S-S constraint -> skip
        if (pct->type() != pct_ss)
        {
            continue;
        }

        // lhsOp (summary) is parent of rhsOp
        lhsSummary->addChildOp(rhsOp);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
SchedulingContext::setCompletionStatus()
{
    const jobop_set_id_t& ops = _dataSet->ops();
    const TRBtree<JobOp>& opsDecSD = _dataSet->opsDecSD();

    // initially:
    //   mark precedence-lag as complete
    //   mark summary as unstarted
    for (auto op : ops)
    {
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
    for (auto op : opsDecSD)
    {
        // summary or complete op -> skip
        if ((op->type() == op_summary) || (op->status() == opstatus_complete))
        {
            continue;
        }

        // mark successors as unstarted
        auto cg = op->esCG();
        auto& succCGs = cg->allSuccCGs();
        for (auto succCG : succCGs)
        {
            for (auto cb : *succCG)
            {
                // reference successor activity and op
                auto succAct = utl::cast<Activity>(cb->owner());
                auto succOp = utl::cast<JobOp>(succAct->owner());
                if ((succAct == nullptr) || (succOp == nullptr))
                {
                    continue;
                }

                // mark non-precedenceLag succOp as unstarted
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

    // for each summary op:
    //   mark it as complete if all its children are complete
    //   mark it as started if at least one of its children has started
    auto& summaryOpsIncSD = _dataSet->summaryOpsIncSD();
    for (auto op_ : summaryOpsIncSD)
    {
        auto op = utl::cast<SummaryOp>(op_);

        // iterate over successors
        bool allComplete = true;
        bool anyStarted = false;
        for (auto childOp : op->childOps())
        {
            allComplete = allComplete && (childOp->status() == opstatus_complete);
            anyStarted = anyStarted || (childOp->status() == opstatus_started);
        }

        // all child ops complete -> mark summary op as complete
        if (allComplete)
        {
            op->status() = opstatus_complete;
        }
        else if (anyStarted) // at least one child op started
        {
            // mark summary op as started
            op->status() = opstatus_started;
        }
    }

    // for each completed precedenceLag op:
    //   if its successor is started, post a unary-ct for it
    for (auto op : ops)
    {
        // skip op that is not precedenceLag type
        if ((op->type() != op_precedenceLag) || (op->status() != opstatus_complete))
        {
            continue;
        }

        // find the precedence-ct linking to successor
        JobOp* succOp = nullptr;
        auto& precedenceCts = _dataSet->precedenceCts();
        for (auto pct : precedenceCts)
        {
            if (pct->lhsOpId() != op->id())
            {
                continue;
            }
            succOp = _dataSet->findOp(pct->rhsOpId());
            break;
        }

        // couldn't find successor op => do nothing else
        if (succOp == nullptr)
        {
            continue;
        }

        // successor is started -> post unary-ct
        if (succOp->status() == opstatus_started)
        {
            ASSERTD(succOp->scheduledStartTime() != -1);
            succOp->add(new UnaryCt(uct_startNoSoonerThan, succOp->scheduledStartTime()));
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
SchedulingContext::setFrozenStatus()
{
    auto& ops = _dataSet->ops();
    auto& opsDecSD = _dataSet->opsDecSD();
    auto opsDecSDend = opsDecSD.end();
    time_t originTime = _config->originTime();
    time_t autoFreezeTime = originTime + _config->autoFreezeDuration();

    // predecessors of frozen ops are frozen (initially)
    for (auto op : opsDecSD)
    {
        // manually frozen?
        op->frozen() = op->frozen() || op->manuallyFrozen();

        // skip summary or non-frozen op
        if ((op->type() == op_summary) || !op->frozen())
        {
            continue;
        }

        // non-frozen ops can't have frozen successors
        // .. so mark predecessors of frozen op as frozen
        auto cg = op->esCG();
        auto& predCGs = cg->allPredCGs();
        for (auto predCG : predCGs)
        {
            for (auto cb : *predCG)
            {
                auto predAct = utl::cast<Activity>(cb->owner());
                auto predOp = utl::cast<JobOp>(predAct->owner());
                ASSERTD(predAct != nullptr);
                ASSERTD(predOp != nullptr);
                predOp->frozen() = true;
            }
        }
    }

    // set frozen status of ops
    for (auto op : ops)
    {
        time_t sst = op->scheduledStartTime();

        // backward scheduling -> don't freeze
        if (_config->backward())
        {
            goto nofreeze;
        }

        // summary op -> don't freeze
        if (op->type() == op_summary)
        {
            goto nofreeze;
        }

        // useInitialAsSeed -> freeze
        if (_config->useInitialAsSeed())
        {
            goto freeze;
        }

        // unscheduled (and not precedence-lag op) -> don't freeze
        if (!op->isScheduled() && (op->type() != op_precedenceLag))
        {
            goto nofreeze;
        }

        // frozen -> freeze
        if (op->frozen())
        {
            goto freeze;
        }

        // started or completed -> freeze
        if ((op->status() == opstatus_started) || (op->status() == opstatus_complete))
        {
            goto freeze;
        }

        // unstarted -> don't freeze
        if (op->status() == opstatus_unstarted)
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

    // successors of non-frozen ops are non-frozen
    for (auto op : opsDecSD)
    {
        // summary op frozen op -> skip
        if ((op->type() == op_summary) || op->frozen())
        {
            continue;
        }

        // mark successors of non-frozen op non-frozen
        auto cg = op->esCG();
        auto& succCGs = cg->allSuccCGs();
        for (auto succCG : succCGs)
        {
            for (auto cb : *succCG)
            {
                auto succAct = utl::cast<Activity>(cb->owner());
                auto succOp = utl::cast<JobOp>(succAct->owner());
                ASSERTD(succAct != nullptr);
                ASSERTD(succOp != nullptr);
                succOp->frozen() = false;
            }
        }
    }

    // post-conditions
    for (auto op : ops)
    {
        // summary op -> skip
        if (op->type() == op_summary)
        {
            continue;
        }

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
SchedulingContext::onComplete()
{
    ASSERTD(_sjobs.size() == 0);
#ifdef DEBUG_UNIT
    utl::cout << "This run is completed!" << utl::endlf;
#endif
    _hardCtScore = _dataSet->hardCtScore();

    // data-set has ResourceSequenceLists?
    if (!_dataSet->resourceSequenceLists().empty())
    {
        // add BoundCts for activities scheduled on unary resources
        postUnaryResourceFS();

        // find ResourceSequenceRule applications, post delays for them
        findResourceSequenceRuleApplications();
        postResourceSequenceDelays();
    }

    // calculate makespan and frozen-makespan
    calculateMakespan();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
SchedulingContext::calculateMakespan()
{
    bool forward = _config->forward();
    auto& jobs = _dataSet->jobs();
    for (auto job : jobs)
    {
        // skip job with id 0
        if (job->id() == 0)
        {
            continue;
        }

        // calculate job's makespan
        job->calculateMakespan(forward);

        // adjust overall makespan & frozen-makespan
        _makespan = utl::max(_makespan, job->makespan());
        _frozenMakespan = utl::max(_frozenMakespan, job->frozenMakespan());
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
SchedulingContext::postUnaryResourceFS()
{
    // iterate over unary resources
    auto& resources = _dataSet->resources();
    for (auto res_ : resources)
    {
        // not a DiscreteResource -> skip
        auto res = dynamic_cast<DiscreteResource*>(res_);
        if (res == nullptr)
        {
            continue;
        }

        // not a unary resource -> skip
        auto clsRes = res->clsResource();
        if ((clsRes == nullptr) || !clsRes->isUnary())
        {
            continue;
        }

        // add F-S relationship for activities scheduled on the resource
        Activity* lhsAct = nullptr;
        const act_set_es_t& acts = clsRes->actsByStartTime();
        for (auto rhsAct : acts)
        {
            // first iteration -> only set lhsAct
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

    // propagate
    _mgr->propagate();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
SchedulingContext::findResourceSequenceRuleApplications()
{
    // iterate over unary resources
    auto& resources = _dataSet->resources();
    for (auto res_ : resources)
    {
        // not a DiscreteResource -> skip
        auto res = dynamic_cast<DiscreteResource*>(res_);
        if (res == nullptr)
        {
            continue;
        }

        // res has no ResourceSequenceList -> skip
        auto rsl = res->sequenceList();
        if (rsl == nullptr)
        {
            continue;
        }

        // not a unary resource -> skip
        auto clsRes = res->clsResource();
        if ((clsRes == nullptr) || !clsRes->isUnary())
        {
            continue;
        }

#ifdef DEBUG_UNIT
        // show activities scheduled on the resource
        utl::cout << utl::endlf << "SchedulingContext::findResourceSequenceRuleApplications()"
                  << utl::endlf;
        auto& debug_acts = clsRes->actsByStartTime();
        for (auto act : debug_acts)
        {
            auto op = utl::cast<JobOp>(act->owner());
            utl::cout << "res:" << res->id() << ", act:" << act->id()
                      << ", seqId:" << op->sequenceId() << ", es=" << act->es()
                      << ", ef=" << act->ef() << utl::endl;
        }
#endif

        // impose delays based on rules in sequence-list
        Activity* lhsAct = nullptr;
        auto& acts = clsRes->actsByStartTime();
        for (auto rhsAct : acts)
        {
            // first iteration -> only set lhsAct
            if (lhsAct == nullptr)
            {
                lhsAct = rhsAct;
                continue;
            }

            auto lhsOp = utl::cast<JobOp>(lhsAct->owner());
            auto rhsOp = utl::cast<JobOp>(rhsAct->owner());

            // do nothing if ops have same sequenceId AND same jobId
            // (this check can be removed when we rewrite code for moving time/cost)
            uint_t lhsOpSeqId = lhsOp->sequenceId();
            uint_t rhsOpSeqId = rhsOp->sequenceId();
            if ((lhsOpSeqId == rhsOpSeqId) && (lhsOp->job()->id() == rhsOp->job()->id()))
            {
                lhsAct = rhsAct;
                continue;
            }

            // find the most specific matching rule
            auto rule = rsl->findRule(lhsOpSeqId, rhsOpSeqId);
            if (rule != nullptr)
            {
                res->sequenceRuleApplications().push_back(
                    ResourceSequenceRuleApplication(lhsOp, rhsOp, rule));
            }

            // rhsAct becomes lhsAct for next iteration
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
    auto& resources = _dataSet->resources();
    for (auto res_ : resources)
    {
        // not a DiscreteResource -> skip
        auto res = dynamic_cast<DiscreteResource*>(res_);
        if (res == nullptr)
        {
            continue;
        }

        // not a unary resource -> skip
        auto clsRes = res->clsResource();
        if ((clsRes == nullptr) || !clsRes->isUnary())
        {
            continue;
        }

        // reference res's ResourceSequenceRuleApplications
        auto& rsras = res->sequenceRuleApplications();

        // double res's provided capacity
        if (rsras.size() > 0)
        {
            clsResources.push_back(clsRes);
            clsRes->doubleProvidedCap();
        }

        // iterate over this resource's ResourceSequenceRuleApplications
        for (auto& app : rsras)
        {
            auto rule = app.rule();
            uint_t delay = _config->durationToTimeSlot(rule->delay());
            if (delay == 0)
            {
                continue;
            }
            ++delay;
            auto lhsAct = app.lhsOp()->activity();
            auto rhsAct = app.rhsOp()->activity();
            if ((lhsAct == nullptr) || (rhsAct == nullptr))
            {
                continue;
            }
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

    // restore res's original capacity
    for (uint_t i = 0; i != clsResources.size(); ++i)
    {
        clsResources[i]->halveProvidedCap();
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
