#include "libcse.h"
#include <clp/BoundPropagator.h>
#include <clp/FailEx.h>
#include <cse/SchedulingContext.h>
#include "RuleBasedScheduler.h"
#include <libutl/BufferedFDstream.h>

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

UTL_CLASS_IMPL_ABC(cse::RuleBasedScheduler);
UTL_CLASS_IMPL(cse::JobLevelScheduler);
UTL_CLASS_IMPL(cse::FrozenOpScheduler);
UTL_CLASS_IMPL(cse::JobSequenceScheduler);
UTL_CLASS_IMPL(cse::OpSequenceScheduler);
UTL_CLASS_IMPL(cse::FwdScheduler);

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////
// RuleBasedScheduler //////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void
RuleBasedScheduler::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(RuleBasedScheduler));
    const RuleBasedScheduler& rbs = (const RuleBasedScheduler&)rhs;
    Scheduler::copy(rbs);
    _jobSid = 0;
    _opSid = 0;
    //     _context = rbs._context;
    setOpOrdering(rbs._opOrdering);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
RuleBasedScheduler::run(SchedulingContext* context) const
{
    // initialize the run
    ASSERTD(context != nullptr);
    initRun(context);

    // schedule using selectOp()
    JobOp* selectedOp;
    while (!context->complete())
    {
        selectedOp = selectOp(context);
        if (selectedOp == nullptr)
        {
            continue;
        }
        if (_setSid)
        {
            selectedOp->serialId() = _opSid++;
        }
#ifdef DEBUG_UNIT
        utl::cout << "   jobId:" << selectedOp->job()->id() << ", opId:" << selectedOp->id()
                  << ", active:" << selectedOp->job()->active()
                  << ", frozen:" << selectedOp->frozen() << ", opSid:" << selectedOp->serialId()
                  << ", opES:" << selectedOp->activity()->es() << utl::endlf;
        CycleGroup* cg = selectedOp->esCG();
        utl::cout << "        " << cg->toString() << ", " << cg->predCGsString() << ", "
                  << cg->succCGsString() << utl::endlf;
#endif
        context->schedule(selectedOp);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
RuleBasedScheduler::initRun(SchedulingContext* context) const
{
    _jobSid = 0;
    _opSid = 0;
    context->setComplete(false); //
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
RuleBasedScheduler::setOpOrdering(OpOrdering* opOrdering)
{
    delete _opOrdering;
    _opOrdering = opOrdering;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
RuleBasedScheduler::init()
{
    _jobSid = 0;
    _opSid = 0;
    _opOrdering = nullptr;
    _setSid = false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
RuleBasedScheduler::deInit()
{
    setOpOrdering(nullptr);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// JobLevelScheduler ///////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void
JobLevelScheduler::copy(const Object& rhs)
{
    RuleBasedScheduler::copy(rhs);

    ASSERTD(rhs.isA(JobLevelScheduler));
    const JobLevelScheduler& jls = (const JobLevelScheduler&)rhs;
    setJobOrdering(jls._jobOrdering); //note: not cloned?
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
JobLevelScheduler::setJobOrdering(JobOrdering* jobOrdering)
{
    delete _jobOrdering;
    _jobOrdering = jobOrdering;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
JobLevelScheduler::initRun(SchedulingContext* context) const
{
    RuleBasedScheduler::initRun(context);

    _sjobsPtr = _sjobs;
    _unreleasedJobs.clear();
    const job_set_id_t& jobs = context->clevorDataSet()->jobs();
    job_set_id_t::iterator jobIt, jobLim = jobs.end();
    for (jobIt = jobs.begin(); jobIt != jobLim; ++jobIt)
    {
        Job* job = *jobIt;
        if (job->active())
        {
            _unreleasedJobs += job;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

JobOp*
JobLevelScheduler::selectOp(SchedulingContext* context) const
{
    //     ASSERTD(_context != nullptr);
    ASSERTD(_jobOrdering != nullptr);
    ASSERTD(_opOrdering != nullptr);

    // select an op from among the schedulable jobs
    JobOp* selectedOp = nullptr;
    Object** jobIt;
    for (jobIt = _sjobs; jobIt != _sjobsPtr; ++jobIt)
    {
        Job* job = (Job*)*jobIt;
        JobOp** opIt;
        JobOp** opLim = job->sopsEnd();
        for (opIt = job->sopsBegin(); opIt != opLim; ++opIt)
        {
            JobOp* op = *opIt;
            // skip op that we can't schedule
            if (!_opOrdering->isSchedulable(op))
            {
                continue;
            }

            // no selectedOp yet => select op
            if (selectedOp == nullptr)
            {
                selectedOp = op;
                continue;
            }

            // note: selectedOp != nullptr

            // select op if it beats selectedOp
            if (_opOrdering->cmp(op, selectedOp) < 0)
            {
                selectedOp = op;
            }
        }
    }

    if (selectedOp == nullptr)
    {
#ifdef DEBUG_UNIT
        utl::cout
            << "\nJobLevelScheduler::selectOp cannot select an op from JobLevelScheduler._sjobs={";
        for (Object** jobIt = _sjobs; jobIt != _sjobsPtr; ++jobIt)
            utl::cout << ((Job*)(*jobIt))->id() << ",";
        utl::cout << "}" << utl::endl;
#endif
        setSchedulableJobs(context);
    }
    return selectedOp;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
JobLevelScheduler::setSchedulableJobs(SchedulingContext* context) const
{
    // clear schedulable-jobs list
    _sjobsPtr = _sjobs;

    // do nothing else if there is no job ordering
    if (_jobOrdering == nullptr)
    {
        return;
    }

    // look at schedulable jobs
    Job* selectedJob = nullptr;
    Job** sjobsIt;
    Job** sjobsLim = context->sjobsEnd();
#ifdef DEBUG_UNIT
    utl::cout << "Select a job from SchedulingContext._sjobs={";
    for (sjobsIt = context->sjobsBegin(); sjobsIt != sjobsLim; ++sjobsIt)
    {
        Job* job = *sjobsIt;
        utl::cout << job->id() << ",";
    }
    utl::cout << "}" << utl::endlf;
#endif
    for (sjobsIt = context->sjobsBegin(); sjobsIt != sjobsLim; ++sjobsIt)
    {
        Job* job = *sjobsIt;

        // skip this job if it has no suitable ops
        JobOp** opIt;
        JobOp** opLim = job->sopsEnd();
        bool anySchedulable = false;
        for (opIt = job->sopsBegin(); opIt != opLim; ++opIt)
        {
            JobOp* op = *opIt;
            if (_opOrdering->isSchedulable(op))
            {
                anySchedulable = true;
                break;
            }
        }
        if (!anySchedulable)
            continue;

        // use _jobOrdering to decide whether to select this job
        // note: OrderingInc is accepted by default
        if ((selectedJob == nullptr) || (_jobOrdering->cmp(job, selectedJob) < 0))
        {
            selectedJob = job;
        }
    }

    // look at unreleased jobs
    if (_releaseJobs && (selectedJob == nullptr))
    {
        utl::RBtree::iterator urIt, urLim = _unreleasedJobs.end();
#ifdef DEBUG_UNIT
        utl::cout << "No job in SchedulingContext._sjobs can be selected, select a job from "
                     "JobLevelScheduler._unreleasedjobs={";
        for (urIt = _unreleasedJobs.begin(); urIt != urLim; ++urIt)
        {
            Job* job = (Job*)*urIt;
            utl::cout << job->id() << ",";
        }
        utl::cout << "}" << utl::endlf;
#endif
        for (urIt = _unreleasedJobs.begin(); urIt != urLim; ++urIt)
        {
            Job* job = (Job*)*urIt;
            ASSERTD(!job->released());

            // use _jobOrdering to decide whether to select this job
            // note: OrderingInc is accepted by default
            if ((selectedJob == nullptr) || (_jobOrdering->cmp(job, selectedJob) < 0))
            {
                selectedJob = job;
            }
        }
    }
#ifdef DEBUG_UNIT
    utl::cout << "selectedJob:";
    if (selectedJob)
    {
        utl::cout << selectedJob->id() << utl::endlf;
    }
    else
    {
        utl::cout << "nullptr" << utl::endlf;
    }
#endif
    // nothing left
    if (selectedJob == nullptr)
    {
        if (!_releaseJobs || _unreleasedJobs.empty())
        {
            context->setComplete(true);
            //             _terminate = true;
        }
        return;
    }

    // all jobs, including itself, in selectedJob's cycle-group
    // are schedulable
    CycleGroup* selectedJobCG = selectedJob->cycleGroup();
    //         utl::cout << "        "
    //                   << selectedJobCG->toString() << ", "
    //                   << selectedJobCG->predCGsString() << ", "
    //                   << selectedJobCG->succCGsString()
    //                   << utl::endlf;

    CycleGroup::iterator cbIt;
    CycleGroup::iterator cbLim = selectedJobCG->end();
    for (cbIt = selectedJobCG->begin(); cbIt != cbLim; ++cbIt)
    {
        ConstrainedBound* cb = *cbIt;
        ASSERTD(cb->owner() != nullptr);
        Job* cbJob = (Job*)cb->owner();
        releaseJob(cbJob, context);
        //         if (!cbJob->active()) continue;
        //         sjobsAdd(cbJob);
        //         if (_releaseJobs && !cbJob->released())
        //         {
        //             if (_setSid)
        //             {
        //                 cbJob->serialId() = _jobSid++;
        //             }
        //             releaseJob(cbJob, context);
        //         }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
JobLevelScheduler::sjobsAdd(utl::Object* obj) const
{
    if (_sjobsPtr == _sjobsLim)
    {
        utl::arrayGrow(_sjobs, _sjobsPtr, _sjobsLim, utl::max((size_t)256, _sjobsSize * 2));
        _sjobsSize = _sjobsLim - _sjobs;
    }
    *_sjobsPtr++ = obj;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
JobLevelScheduler::releaseJob(Job* job, SchedulingContext* context) const
{
    if (!job->active())
        return;
    sjobsAdd(job);
    if (_releaseJobs && !job->released())
    {
        if (_setSid)
        {
            job->serialId() = _jobSid++;
        }
        ASSERTFNP(_unreleasedJobs.remove(*job));
        job->release(context->manager());
#ifdef DEBUG_UNIT
        clp::CycleGroup* cg = job->cycleGroup();
        utl::cout << "Release job:" << job->id() << " in " << cg->toString() << ", "
                  << cg->predCGsString() << ", " << cg->succCGsString() << utl::endlf;
        utl::cout << "   jobId:" << job->id() << ", jobSid:" << job->serialId()
                  << ", dueTime:" << job->dueTime() << ", successorDepth:" << job->successorDepth()
                  << ", active:" << job->active() << ", #unreleasedJobs:" << _unreleasedJobs.size()
                  << ", latenessCost:" << job->latenessCost() << utl::endl;
#endif
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
JobLevelScheduler::init()
{
    _jobOrdering = nullptr;
    _unreleasedJobs.setOwner(false);
    _releaseJobs = true;

    _sjobs = _sjobsPtr = _sjobsLim = nullptr;
    _sjobsSize = 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
JobLevelScheduler::deInit()
{
    setJobOrdering(nullptr);
    delete[] _sjobs;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// FrozenOpScheduler ///////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void
FrozenOpScheduler::init()
{
    JobOrderingDecLatenessCost* jobOrdering1 = new JobOrderingDecLatenessCost();
    JobOrderingIncDueTime* jobOrdering2 = new JobOrderingIncDueTime();
    jobOrdering1->setNextOrdering(jobOrdering2);
    setJobOrdering(jobOrdering1);
    setOpOrdering(new OpOrderingFrozenFirst());
    _releaseJobs = false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// JobSequenceScheduler ////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void
JobSequenceScheduler::init()
{
    setJobOrdering(new JobOrderingIncSID());
    setOpOrdering(new JobOpOrderingIncSID());
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// OpSequenceScheduler /////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void
OpSequenceScheduler::initRun(SchedulingContext* context) const
{
    RuleBasedScheduler::initRun(context);

    // init _idx
    _idx = 0;

    // init _ops
    _ops.clear();
    ClevorDataSet* dataSet = (ClevorDataSet*)context->dataSet();
    const jobop_set_id_t& ops = dataSet->sops();
    jobop_set_id_t::const_iterator opIt;
    for (opIt = ops.begin(); opIt != ops.end(); ++opIt)
    {
        JobOp* op = *opIt;
        if (op->frozen() || !op->job()->active())
            continue;
        _ops += op;
    }
    // sort _ops by SID
    _ops.setOrdering(*_opOrdering);

    // release all jobs
    const job_set_id_t& jobs = dataSet->jobs();
    job_set_id_t::const_iterator jobIt;
    for (jobIt = jobs.begin(); jobIt != jobs.end(); ++jobIt)
    {
        Job* job = *jobIt;
        if (job->active())
        {
            job->release(context->manager());
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

JobOp*
OpSequenceScheduler::selectOp(SchedulingContext* context) const
{
    ASSERTD(_idx <= _ops.items());

    // all done?
    if (_idx == _ops.items())
    {
        context->setComplete(true);
        //         _terminate = true;
        return nullptr;
    }

    ASSERTD(_ops[_idx]->isA(JobOp));
    JobOp* op = (JobOp*)_ops[_idx++];
    return op;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
OpSequenceScheduler::init()
{
    _ops.setOwner(false);
    setOpOrdering(new OpOrderingIncSID());
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// FwdScheduler ////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void
FwdScheduler::init()
{
    JobOrderingDecSuccessorDepth* jobOrdering1 = new JobOrderingDecSuccessorDepth();
    JobOrderingDecLatenessCost* jobOrdering2 = new JobOrderingDecLatenessCost();
    JobOrderingIncDueTime* jobOrdering3 = new JobOrderingIncDueTime();
    jobOrdering1->setNextOrdering(jobOrdering2);
    jobOrdering2->setNextOrdering(jobOrdering3);
    setJobOrdering(jobOrdering1);
    setOpOrdering(new OpOrderingIncES());
    _setSid = true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
FwdScheduler::initRun(SchedulingContext* context) const
{
    RuleBasedScheduler::initRun(context);

    _sjobsPtr = _sjobs;
    _unreleasedJobs.clear();
    const job_set_id_t& jobs = context->clevorDataSet()->jobs();
    job_set_id_t::iterator jobIt, jobLim = jobs.end();
    for (jobIt = jobs.begin(); jobIt != jobLim; ++jobIt)
    {
        Job* job = *jobIt;
        // allow inactive jobs in _unreleasedJobs
        _unreleasedJobs += job;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
FwdScheduler::releaseJob(Job* job, SchedulingContext* context) const
{
    sjobsAdd(job);
    if (_releaseJobs && !job->released())
    {
        if (_setSid && job->active())
        {
            job->serialId() = _jobSid++;
        }
        ASSERTFNP(_unreleasedJobs.remove(*job));
        if (!job->active())
        {
            if (_setSid)
            {
                //                 if (job->id() == 6)
                //                 {
                //                     utl::cout << "job:" << job->id()
                //                               << ", numAllSops:" << Uint(job->allSops().size())
                //                               << utl::endlf;
                //                 }
                jobop_vector_t opVect;
                const jobop_set_id_t& ops = job->allSops();
                jobop_set_id_t::const_iterator opIt;
                for (opIt = ops.begin(); opIt != ops.end(); ++opIt)
                {
                    JobOp* op = *opIt;
                    opVect.push_back(op);
                }
                std::stable_sort(opVect.begin(), opVect.end(), JobOpFDsuccessorDepthDecOrdering());
                jobop_vector_t::iterator it;
                for (it = opVect.begin(); it != opVect.end(); it++)
                {
                    JobOp* op = *it;
                    op->serialId() = _opSid++;
                }
            }
        }
        else
        {
            job->release(context->manager());
        }
#ifdef DEBUG_UNIT
        clp::CycleGroup* cg = job->cycleGroup();
        utl::cout << "Release job:" << job->id() << " in " << cg->toString() << ", "
                  << cg->predCGsString() << ", " << cg->succCGsString() << utl::endlf;
        utl::cout << "   jobId:" << job->id() << ", jobSid:" << job->serialId()
                  << ", dueTime:" << job->dueTime() << ", successorDepth:" << job->successorDepth()
                  << ", active:" << job->active() << ", #unreleasedJobs:" << _unreleasedJobs.size()
                  << ", latenessCost:" << job->latenessCost() << utl::endl;
#endif
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;
