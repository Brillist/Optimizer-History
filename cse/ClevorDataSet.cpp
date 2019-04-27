#include "libcse.h"
#include <libutl/BufferedFDstream.h>
#include <clp/BoundPropagator.h>
#include <cls/CompositeResourceRequirement.h>
#include <cls/DiscreteResourceRequirement.h>
#include <cls/EFbound.h>
#include <cls/EFboundInt.h>
#include <cls/ESbound.h>
#include <cls/ESboundInt.h>
#include <cls/ESboundCalendar.h>
#include <cls/LFbound.h>
#include <cls/LFboundCalendar.h>
#include <cls/ResourceCalendarMgr.h>
#include <cls/Schedule.h>
#include "ClevorDataSet.h"
#include "CompositeResource.h"
#include "DiscreteResource.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef DEBUG
// #define DEBUG_UNIT
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;
CLP_NS_USE;
CLS_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// JobOpIncSDordering /////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

class JobOpIncSDordering : public Ordering
{
    UTL_CLASS_DECL(JobOpIncSDordering, Ordering);
    UTL_CLASS_DEFID;

public:
    virtual int cmp(const Object* lhs, const Object* rhs) const;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

int
JobOpIncSDordering::cmp(const Object* lhs, const Object* rhs) const
{
    ASSERTD(lhs->isA(JobOp));
    ASSERTD(rhs->isA(JobOp));
    JobOp* lhsOp = (JobOp*)lhs;
    JobOp* rhsOp = (JobOp*)rhs;
    uint_t lhsSD, rhsSD;
    if (lhsOp->activity() == nullptr)
        lhsSD = 0;
    else
        lhsSD = lhsOp->esCG()->successorDepth();
    if (rhsOp->activity() == nullptr)
        rhsSD = 0;
    else
        rhsSD = rhsOp->esCG()->successorDepth();
    int res = utl::compare(lhsSD, rhsSD);
    if (res != 0)
        return res;
    return utl::compare(lhsOp->id(), rhsOp->id());
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// JobOpDecSDordering /////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

class JobOpDecSDordering : public Ordering
{
    UTL_CLASS_DECL(JobOpDecSDordering, Ordering);
    UTL_CLASS_DEFID;

public:
    virtual int cmp(const Object* lhs, const Object* rhs) const;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

int
JobOpDecSDordering::cmp(const Object* lhs, const Object* rhs) const
{
    if (!lhs->isA(JobOp) || !rhs->isA(JobOp))
    {
        return lhs->compare(*rhs);
    }
    JobOp* lhsOp = (JobOp*)lhs;
    JobOp* rhsOp = (JobOp*)rhs;
    uint_t lhsSD, rhsSD;
    if (lhsOp->activity() == nullptr)
        lhsSD = 0;
    else
        lhsSD = lhsOp->esCG()->successorDepth();
    if (rhsOp->activity() == nullptr)
        rhsSD = 0;
    else
        rhsSD = rhsOp->esCG()->successorDepth();
    int res = utl::compare(rhsSD, lhsSD);
    if (res != 0)
        return res;
    return utl::compare(lhsOp->id(), rhsOp->id());
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// ClevorDataSet //////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void
ClevorDataSet::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(ClevorDataSet));
    const ClevorDataSet& ds = (const ClevorDataSet&)rhs;
    DataSet::copy(ds);
    delete _config;
    _config = lut::clone(ds._config);
    copySet(_jobs, ds._jobs);
    copySet(_resources, ds._resources);
    copySet(_resGroups, ds._resGroups);
    copySet(_rsls, ds._rsls);
    copyVector(_pcts, ds._pcts);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ClevorDataSet::modelBuild_0(Schedule* schedule)
{
    // init _mgr, _schedule, _bp
    _schedule = schedule;
    _mgr = _schedule->manager();
    _bp = _mgr->boundPropagator();

    // init time-slot values
    _originTS = 0;
    _schedulingOriginTS = _config->schedulingOriginTimeSlot();
    _horizonTS = _config->horizonTimeSlot();

    // compute static views (phase 0-0)
    createViews_0_0();

    // create activities and precedence links
    modelBuildActivities_0();
    modelBuildPrecedenceCts_0();
    propagate();

    // compute static views (phase 0-1)
    createViews_0_1();

    // set resource-sequence-lists for all resources
    setRSLs();

#ifdef DEBUG
    // to debug a given activity's bounds...
    JobOp* debugOp = findOp(4);
    if ((debugOp != nullptr) && (debugOp->activity() != nullptr))
    {
        debugOp->activity()->setDebugFlag(true);
    }
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ClevorDataSet::modelBuild_1()
{
    modelBuildActivities_1();
    propagate();
    createViews_1();
    modelBuildPrecedenceCts_1();
    propagate();
    modelBuildUnaryCts();
    propagate();
    modelBuildResources();
    propagate();
    modelBuildResourceCts();
    propagate();
    modelBuildHeuristics();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

uint_t
ClevorDataSet::hardCtScore() const
{
    uint_t score = 0;
    Array::iterator it;
    Array::iterator lim = _hardCtOps.end();
    for (it = _hardCtOps.begin(); it != lim; ++it)
    {
        JobOp* op = (JobOp*)*it;
        Activity* act = op->activity();
        if (_config->forward())
        {
            int es = act->es();
            int ef = act->ef();

            // find ls, lf
            int ls = int_t_max;
            if (op->maxStartTime() >= 0)
            {
                ls = timeToTimeSlot(op->maxStartTime());
            }
            int lf = int_t_max;
            if (op->maxEndTime() >= 0)
            {
                lf = timeToTimeSlot(op->maxEndTime());
            }

            // no hard ct violation for this op?
            if ((es <= ls) && (ef <= lf))
            {
                continue;
            }

            score += utl::max(es - ls, ef - lf);
        }
        else
        {
        }
    }
    return score;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ClevorDataSet::set(SchedulerConfiguration* config)
{
    delete _config;
    _config = config;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ClevorDataSet::add(Job* job)
{
    _jobs.insert(job);
    job->dataSet() = this;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ClevorDataSet::add(Resource* res)
{
    _resources.insert(res);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ClevorDataSet::add(ResourceGroup* resGroup)
{
    _resGroups.insert(resGroup);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ClevorDataSet::add(ResourceSequenceList* rsl)
{
    _rsls.insert(rsl);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ClevorDataSet::add(PrecedenceCt* pct)
{
    _pcts.push_back(pct);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ClevorDataSet::add(JobGroup* jobGroup)
{
    ASSERTD(jobGroup != nullptr);
    //     ASSERTD(jobGroup->activeJob() != nullptr);

    // jobGroup's members are not dummy ones, then return.
    if (!jobGroup->jobsOwner())
    {
        JobGroup* group = lut::setFind(_jobGroups, jobGroup);
        if (group == nullptr)
        {
            _jobGroups.insert(jobGroup);
        }
        return;
    }
    // jobGroup has no members
    if (jobGroup->jobs().size() == 0)
        return;

    job_set_pref_t jobs;
    Job* selectedJob = nullptr;
    Job* aJob = jobGroup->activeJob();
    job_set_pref_t::iterator it;
    for (it = jobGroup->jobs().begin(); it != jobGroup->jobs().end(); it++)
    {
        Job* job = lut::setFind(_jobs, (*it));
        // we will not create a job group if any of its members has the status
        // of confirmed, started, or completed.
        // we also need to ignore all its siblings by setting their inactive.
        // this may affect optimization efficiency, but removing jobs from dataset
        // is hard in both FE and BE, especially in FE.
        if (job->status() == jobstatus_confirmed || job->status() == jobstatus_started ||
            job->status() == jobstatus_complete)
        {
            selectedJob = job;
        }

        ASSERTD(job != nullptr);
        // copy preference
        job->preference() = (*it)->preference();
        jobs.insert(job);
        if (aJob->id() == job->id())
        {
            job->active() = true;
            aJob = job;
        }
        else
        {
            job->active() = false;
        }
    }

    if (jobs.size() == 1)
        return;
    //     ASSERTD(jobs.size() > 1);
    ASSERTD(aJob != nullptr);
    if (selectedJob != nullptr)
    {
        for (it = jobs.begin(); it != jobs.end(); it++)
        {
            (*it)->active() = false;
        }
        selectedJob->active() = true;
        return;
    }

    _jobGroups.insert(jobGroup);
    jobGroup->setJobs(jobs);
    jobGroup->setActiveJob(aJob);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Job*
ClevorDataSet::findJob(uint_t id) const
{
    Job dummy;
    dummy.id() = id;
    job_set_id_t::const_iterator it = _jobs.find(&dummy);
    if (it == _jobs.end())
        return nullptr;
    return *it;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

JobGroup*
ClevorDataSet::findJobGroup(uint_t id) const
{
    JobGroup dummy;
    dummy.id() = id;
    jobgroup_set_id_t::const_iterator it = _jobGroups.find(&dummy);
    if (it == _jobGroups.end())
        return nullptr;
    return *it;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

JobOp*
ClevorDataSet::findOp(uint_t id) const
{
    JobOp dummy;
    dummy.id() = id;
    jobop_set_id_t::const_iterator it = _ops.find(&dummy);
    if (it == _ops.end())
        return nullptr;
    return *it;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Resource*
ClevorDataSet::findResource(uint_t id) const
{
    Resource dummy;
    dummy.id() = id;
    res_set_id_t::const_iterator it = _resources.find(&dummy);
    if (it == _resources.end())
        return nullptr;
    return *it;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

ResourceGroup*
ClevorDataSet::findResourceGroup(uint_t id) const
{
    ResourceGroup dummy(id);
    resgroup_set_t::const_iterator it = _resGroups.find(&dummy);
    if (it == _resGroups.end())
        return nullptr;
    return *it;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

ResourceSequenceList*
ClevorDataSet::findResourceSequenceList(uint_t id) const
{
    ResourceSequenceList dummy;
    dummy.id() = id;
    rsl_set_t::const_iterator it = _rsls.find(&dummy);
    if (it == _rsls.end())
        return nullptr;
    return *it;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ClevorDataSet::clearProblemData()
{
    deleteCont(_jobs);
    deleteCont(_jobGroups);
    deleteCont(_pcts);
    deleteCont(_resources);
    deleteCont(_resGroups);
    deleteCont(_rsls);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ClevorDataSet::init()
{
    _mgr = nullptr;
    _schedule = nullptr;
    _bp = nullptr;
    _originTS = -1;
    _schedulingOriginTS = -1;
    _horizonTS = -1;
    _config = nullptr;
    _hardCtOps.setOwner(false);
    _opsDecSD.setOwner(false);
    _summaryOpsIncSD.setOwner(false);
    _minCostHeuristics = nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ClevorDataSet::deInit()
{
    delete _config;
    deleteCont(_jobs);
    deleteCont(_jobGroups);
    deleteCont(_pcts);
    deleteCont(_resources);
    deleteCont(_resGroups);
    deleteCont(_rsls);
    delete _minCostHeuristics;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int
ClevorDataSet::timeToTimeSlot(time_t t) const
{
    ASSERTD(_config != nullptr);
    return _config->timeToTimeSlot(t);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ClevorDataSet::createViews_0_0()
{
    // make list of all/schedulable ops
    _ops.clear();
    job_set_id_t::iterator jobIt;
    for (jobIt = _jobs.begin(); jobIt != _jobs.end(); ++jobIt)
    {
        Job* job = *jobIt;
        Job::const_iterator opIt;
        for (opIt = job->begin(); opIt != job->end(); ++opIt)
        {
            JobOp* op = *opIt;
            _ops.insert(op);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ClevorDataSet::createViews_0_1()
{
    // perform static analysis of CycleGroup precedence graph
    _bp->staticAnalysis();

    _opsDecSD.setOrdering(new JobOpDecSDordering());
    _summaryOpsIncSD.setOrdering(new JobOpIncSDordering());

    // populate _opsDecSD
    jobop_set_id_t::iterator opIt;
    for (opIt = _ops.begin(); opIt != _ops.end(); ++opIt)
    {
        JobOp* op = *opIt;
        _opsDecSD += op;
    }

    // populate _summaryOpsIncSD
    for (opIt = _ops.begin(); opIt != _ops.end(); ++opIt)
    {
        JobOp* op = *opIt;
        if (op->type() != op_summary)
            continue;
        _summaryOpsIncSD += op;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ClevorDataSet::createViews_1()
{
    // make list of schedulable ops
    _sops.clear();
    job_set_id_t::iterator jobIt;
    for (jobIt = _jobs.begin(); jobIt != _jobs.end(); ++jobIt)
    {
        Job* job = *jobIt;
        job->sopsInit(_mgr);
        Job::const_iterator opIt;
        for (opIt = job->begin(); opIt != job->end(); ++opIt)
        {
            JobOp* op = *opIt;
            if (op->schedulable())
            {
                _sops.insert(op);
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ClevorDataSet::setRSLs()
{
    res_set_id_t::iterator resIt;
    for (resIt = _resources.begin(); resIt != _resources.end(); ++resIt)
    {
        Resource* res = *resIt;
        if (dynamic_cast<DiscreteResource*>(res) == nullptr)
        {
            continue;
        }
        DiscreteResource* dres = (DiscreteResource*)res;
        uint_t id = dres->sequenceId();
        if (id == uint_t_max)
        {
            dres->sequenceList() = nullptr;
            continue;
        }
        const ResourceSequenceList* rsl = findResourceSequenceList(id);
        dres->sequenceList() = rsl;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ClevorDataSet::propagate()
{
    _mgr->propagate();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ClevorDataSet::modelBuildActivities_0()
{
    jobop_set_id_t::const_iterator it;
    for (it = _ops.begin(); it != _ops.end(); ++it)
    {
        JobOp* op = *it;

        // get rid of unsupported resource-requirements
        op->removeImproperResReqs();

        ConstrainedBound* es;
        ConstrainedBound* ef;
        ConstrainedBound* ls;
        ConstrainedBound* lf;

        int esb = _originTS, lsb = _horizonTS - 1;
        int efb = _originTS - 1, lfb = _horizonTS - 1;

        // make the activity
        Activity* act;
        IntActivity* intact = nullptr;
        BrkActivity* brkact = nullptr;
        // Changed the following line, because removeImproperResReqs
        // guarantees all summary ops don't have any resource requirement.
        // Note: an ASSERTD is added in else{}.
        // Probably !op->schedulable() is even better. Joe Sept 6, 2006
        //         if (!op->hasRequirements() || (op->type() == op_summary))
        if (!op->hasRequirements())
        {
            act = new Activity(_schedule);
            es = new ConstrainedBound(_mgr, bound_lb, esb);
            ef = new ConstrainedBound(_mgr, bound_lb, efb);
            ls = new ConstrainedBound(_mgr, bound_ub, lsb);
            lf = new ConstrainedBound(_mgr, bound_ub, lfb);
        }
        else if (op->type() == op_interruptible)
        {
            act = intact = new IntActivity(_schedule);
            intact->setProcessingTime(_config->durationToTimeSlot(op->processingTime()));
            if (intact->processingTime() == uint_t_max)
            {
                intact->setProcessingTime(0);
            }

            if (_config->forward())
            {
                // ES
                ESboundInt* esBound = new ESboundInt(intact, esb);
                es = esBound;

                // EF
                EFboundInt* efBound = new EFboundInt(esBound, efb);
                ef = efBound;
                esBound->setEFbound(efBound);

                // LS,LF
                ls = new ConstrainedBound(_mgr, bound_ub, lsb);
                lf = new ConstrainedBound(_mgr, bound_ub, lfb);
            }
            else
            {
                ABORT();
            }
        }
        else // breakable
        {
            ASSERTD(op->type() != op_summary);
            // activity
            act = brkact = new BrkActivity(_schedule);
            Bound* calendarBound;

            if (_config->forward())
            {
                // ES
                ESbound* esBound = new ESbound(_mgr, esb);
                es = esBound;
                esBound->setActivity(brkact);

                // EF
                EFbound* efBound = new EFbound(esBound, efb);
                ef = efBound;
                esBound->setEFbound(efBound);

                // LS,LF
                ls = new ConstrainedBound(_mgr, bound_ub, lsb);
                lf = new ConstrainedBound(_mgr, bound_ub, lfb);

                // calendar bound
                calendarBound = new ESboundCalendar(brkact, 0, efBound);
                esBound->add(calendarBound);
            }
            else
            {
                // LF
                LFbound* lfBound = new LFbound(_mgr, lfb);
                lf = lfBound;

                // LS
                LSbound* lsBound = new LSbound(lfBound, lsb);
                ls = lsBound;
                lfBound->setLSbound(lsBound);

                // ES,EF
                es = new ConstrainedBound(_mgr, bound_lb, esb);
                ef = new ConstrainedBound(_mgr, bound_lb, efb);

                // calendar bound
                calendarBound = new LFboundCalendar(brkact, 0);
                lfBound->add(calendarBound);
            }

            calendarBound->setName("act-" + Uint(op->id()).toString() + " calendar");
            _mgr->revAllocate(calendarBound);
        }

        // remember to delete these bounds
        _mgr->revAllocate(es);
        _mgr->revAllocate(ef);
        _mgr->revAllocate(ls);
        _mgr->revAllocate(lf);

        // name the bounds
        String prefix = "act-" + Uint(op->id()).toString() + " ";
        es->setName(prefix + "es");
        ef->setName(prefix + "ef");
        ls->setName(prefix + "ls");
        lf->setName(prefix + "lf");

        // establish twin relationships
        // .. to detect failure if (es > ls) or (ef > lf)
        es->setTwinBound(ls);
        ls->setTwinBound(es);
        ef->setTwinBound(lf);
        lf->setTwinBound(ef);

        // make CycleGroup for ES,LF
        CycleGroup* esCG = _bp->newCycleGroup();
        CycleGroup* lfCG = _bp->newCycleGroup();
        esCG->add(es);
        lfCG->add(lf);

        CycleGroup* lsCG;
        CycleGroup* efCG;
        // summary op: make separate CGs for EF,LS
        if (op->type() == op_summary)
        {
            lsCG = _bp->newCycleGroup();
            efCG = _bp->newCycleGroup();
        }
        // non-summary: ES,EF share a CG and LS,LF share a CG
        else
        {
            efCG = esCG;
            lsCG = lfCG;
        }
        lsCG->add(ls);
        efCG->add(ef);

        // initialize start/end range-vars
        RangeVar& actStart = act->start();
        RangeVar& actEnd = act->end();
        actStart.set(es, ls);
        actEnd.set(ef, lf);

        // set id, name
        act->setId(op->id());
        act->setName(op->name());

        // link es, ef, ls, lf => act
        es->setOwner(act);
        ef->setOwner(act);
        ls->setOwner(act);
        lf->setOwner(act);

        // link schedule => act, op <=> act
        _schedule->add(act);
        op->activity() = act;
        act->setOwner(op);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ClevorDataSet::modelBuildPrecedenceCts_0()
{
    // make cycle-groups for jobs
    job_set_id_t::iterator jobIt;
    job_set_id_t::iterator jobsEnd = _jobs.end();
    for (jobIt = _jobs.begin(); jobIt != jobsEnd; ++jobIt)
    {
        Job* job = *jobIt;
        job->initPrecedenceBound(_mgr);
    }

    //DEBUG_CODE
    uint_t iter = 0;

    pct_vector_t::const_iterator it;
    for (it = _pcts.begin(); it != _pcts.end(); ++it)
    {
        //DEBUG_CODE
        ++iter;

        PrecedenceCt* pct = *it;
        JobOp* lhsOp = findOp(pct->lhsOpId());
        JobOp* rhsOp = findOp(pct->rhsOpId());

        ASSERTD(lhsOp != nullptr);
        ASSERTD(rhsOp != nullptr);
        Activity* lhsAct = lhsOp->activity();
        Activity* rhsAct = rhsOp->activity();

        // make a precedence link on job level
        // (but ignore (* -FF-> summary)) Joe:Why?
        if ((pct->type() != pct_ff) || (rhsOp->type() != op_summary))
        {
            if (_config->forward())
            {
                _bp->addPrecedenceLink(lhsOp->job()->cycleGroup(), rhsOp->job()->cycleGroup());
            }
            else
            {
                _bp->addPrecedenceLink(rhsOp->job()->cycleGroup(), lhsOp->job()->cycleGroup());
            }
        }

        CycleGroup* lhsEScg = lhsAct->esBound().cycleGroup();
        CycleGroup* lhsEFcg = lhsAct->efBound().cycleGroup();
        CycleGroup* lhsLScg = lhsAct->lsBound().cycleGroup();
        CycleGroup* lhsLFcg = lhsAct->lfBound().cycleGroup();
        CycleGroup* rhsEScg = rhsAct->esBound().cycleGroup();
        CycleGroup* rhsEFcg = rhsAct->efBound().cycleGroup();
        CycleGroup* rhsLScg = rhsAct->lsBound().cycleGroup();
        CycleGroup* rhsLFcg = rhsAct->lfBound().cycleGroup();

        switch (pct->type())
        {
        case pct_fs:
            _bp->addPrecedenceLink(lhsEFcg, rhsEScg);
            _bp->addPrecedenceLink(rhsLScg, lhsLFcg);
            break;
        case pct_ss:
            _bp->addPrecedenceLink(lhsEScg, rhsEScg);
            _bp->addPrecedenceLink(rhsLScg, lhsLScg);
            break;
        case pct_ff:
            _bp->addPrecedenceLink(lhsEFcg, rhsEFcg);
            _bp->addPrecedenceLink(rhsLFcg, lhsLFcg);
            break;
        case pct_sf:
            _bp->addPrecedenceLink(lhsEScg, rhsEFcg);
            _bp->addPrecedenceLink(rhsLFcg, lhsLScg);
            break;
        default:
            ABORT();
        }

        BREAKPOINT;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ClevorDataSet::modelBuildPrecedenceCts_1()
{
    pct_vector_t::const_iterator it;
    for (it = _pcts.begin(); it != _pcts.end(); ++it)
    {
        PrecedenceCt* pct = *it;
        JobOp* lhsOp = findOp(pct->lhsOpId());
        JobOp* rhsOp = findOp(pct->rhsOpId());

        // skip precedence-ct that involves an ignorable op
        if (lhsOp->ignorable() || rhsOp->ignorable())
        {
            continue;
        }

        ASSERTD(lhsOp != nullptr);
        ASSERTD(rhsOp != nullptr);
        Activity* lhsAct = lhsOp->activity();
        Activity* rhsAct = rhsOp->activity();

        // convert delay to time-slots
        int delay = _config->durationToTimeSlot(pct->delay());

        // reference the start/end lower/upper bounds
        ConstrainedBound& lhsES = lhsAct->start().lowerBound();
        ConstrainedBound& lhsEF = lhsAct->end().lowerBound();
        ConstrainedBound& lhsLS = lhsAct->start().upperBound();
        ConstrainedBound& lhsLF = lhsAct->end().upperBound();
        ConstrainedBound& rhsES = rhsAct->start().lowerBound();
        ConstrainedBound& rhsEF = rhsAct->end().lowerBound();
        ConstrainedBound& rhsLS = rhsAct->start().upperBound();
        ConstrainedBound& rhsLF = rhsAct->end().upperBound();

        switch (pct->type())
        {
        case pct_fs:
            ++delay;
            _bp->addBoundCt(lhsEF, rhsES, delay);
            _bp->addBoundCt(rhsLS, lhsLF, -delay);
            break;
        case pct_ss:
            _bp->addBoundCt(lhsES, rhsES, delay);
            _bp->addBoundCt(rhsLS, lhsLS, -delay);
            break;
        case pct_ff:
            _bp->addBoundCt(lhsEF, rhsEF, delay);
            _bp->addBoundCt(rhsLF, lhsLF, -delay);
            break;
        case pct_sf:
            --delay;
            _bp->addBoundCt(lhsES, rhsEF, delay);
            _bp->addBoundCt(rhsLF, lhsLS, -delay);
            break;
        default:
            ABORT();
        }
    }

    // set release-bounds for each job
    job_set_id_t::iterator jobIt;
    job_set_id_t::iterator jobsEnd = _jobs.end();
    for (jobIt = _jobs.begin(); jobIt != jobsEnd; ++jobIt)
    {
        Job* job = *jobIt;
        job->initReleaseBound(_mgr);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ClevorDataSet::modelBuildActivities_1()
{
    jobop_set_id_t::const_iterator it;
    for (it = _ops.begin(); it != _ops.end(); ++it)
    {
        JobOp* op = *it;

        // handle non-schedulable
        if (!op->schedulable())
        {
            // not schedulable => set bounds on start/end times
            if (op->isScheduled() && !op->ignorable() && (op->type() != op_summary))
            {
                Activity* act = op->activity();
                int startTS = timeToTimeSlot(op->scheduledStartTime());
                int endTS = timeToTimeSlot(op->scheduledEndTime()) - 1;
                if (_config->forward())
                {
                    act->start().setLB(startTS);
                    act->end().setLB(endTS);
                }
                else
                {
                    act->end().setUB(endTS);
                    act->start().setUB(startTS);
                }
            }
            continue;
        }

        // skip op that is not (frozen with res-reqs)
        if (!(op->frozen() && (op->breakable() || op->interruptible())))
        {
            continue;
        }

        // determine processing-time
        int pt;
        if ((op->status() == opstatus_started) && (op->scheduledRemainingPt() != uint_t_max) &&
            (op->scheduledRemainingPt() > 0))
        {
            pt = (int)op->scheduledRemainingPt();
        }
        else if (op->type() == op_precedenceLag)
        {
            ASSERTD(op->processingTime() != uint_t_max);
            pt = (int)op->processingTime();
        }
        else
        {
            pt = (int)op->scheduledProcessingTime();
        }
        pt = _config->durationToTimeSlot(pt);

        // set activity's processing-time
        if (op->interruptible())
        {
            IntActivity* act = op->intact();
            act->setProcessingTime(pt);
        }
        else
        {
            // Joe: how about ops whose hasResRequirements()=False,
            // and contains only an Activity (see modelBuildActivity_0)?
            // Oct 10, 2006
            PtActivity* act = op->brkact();
            act->setFrozenPt(pt);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ClevorDataSet::modelBuildUnaryCts()
{
    _hardCtOps.clear();
    jobop_set_id_t::const_iterator it;
    for (it = _ops.begin(); it != _ops.end(); ++it)
    {
        JobOp* op = *it;

        // skip ignorable
        if (op->ignorable())
            continue;

        Activity* act = op->activity();
        if (act == nullptr)
            continue;

        // default values for es,ls,ef,lf
        int es = _originTS, ls = _horizonTS - 1;
        int ef = _originTS - 1, lf = _horizonTS - 1;

        if (op->frozen())
        {
            ASSERTD(_config->forward());
            if (op->status() == opstatus_started)
            {
                es = utl::max(es, timeToTimeSlot(op->scheduledResumeTime()));
            }
            else
            {
                es = timeToTimeSlot(op->scheduledStartTime());
            }
        }
        else // not frozen
        {
            // impose limit on ES or LF
            if (op->type() != op_summary)
            {
                if (_config->forward())
                {
                    es = _schedulingOriginTS;
                }
                else
                {
                    lf = utl::min(_horizonTS - 1, _schedulingOriginTS);
                    Job* job = op->job();
                    if (job->dueTime() != -1)
                    {
                        int dueTS = timeToTimeSlot(job->dueTime());
                        lf = utl::min(lf, dueTS - 1);
                    }
                }
            }

            // unary-cts
            op->processUnaryCts();

            if (op->minStartTime() >= 0)
            {
                es = utl::max(es, timeToTimeSlot(op->minStartTime()));
            }
            if (op->maxStartTime() >= 0)
            {
                ls = timeToTimeSlot(op->maxStartTime());
            }
            if (op->minEndTime() >= 0)
            {
                ef = timeToTimeSlot(op->minEndTime()) - 1;
            }
            if (op->maxEndTime() >= 0)
            {
                lf = timeToTimeSlot(op->maxEndTime()) - 1;
            }

            // detect a hard unary constraint
            if (_config->forward())
            {
                if ((ls < (_horizonTS - 1)) || (lf < (_horizonTS - 1)))
                {
                    ls = (_horizonTS - 1);
                    lf = (_horizonTS - 1);
                    _hardCtOps += op;
                    op->hasHardCt() = true;
                }
            }
            else
            {
                if ((es > _originTS) || (ef > (_originTS - 1)))
                {
                    es = _originTS;
                    ef = (_originTS - 1);
                    _hardCtOps += op;
                    op->hasHardCt() = true;
                }
            }
        }

        // make sure we have sane ES,EF and LS,LF relationships
        ef = utl::max(ef, es - 1);
        ls = utl::min(ls, lf + 1);

        ConstrainedBound& esb = act->start().lowerBound();
        ConstrainedBound& efb = act->end().lowerBound();
        ConstrainedBound& lsb = act->start().upperBound();
        ConstrainedBound& lfb = act->end().upperBound();

        // impose bounds
        esb.setLB(es);
        efb.setLB(ef);
        lsb.setUB(ls);
        lfb.setUB(lf);

        // for ops with no requirements and defined processing-time
        // post simple constraints between es,ls,ef,lf

        if (op->hasRequirements() || (op->processingTime() == uint_t_max))
        {
            continue;
        }

        // convert processing-time to time-slots
        int pt = _config->durationToTimeSlot((int)op->processingTime());
        //         int pt = op->processingTime() / (int)_config->timeStep();
        --pt;

        // post constraints
        // es => ef
        _bp->addBoundCt(esb, efb, pt);
        // ef => es
        _bp->addBoundCt(efb, esb, -pt);
        // ls => lf
        _bp->addBoundCt(lsb, lfb, pt);
        // lf => ls
        _bp->addBoundCt(lfb, lsb, -pt);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ClevorDataSet::modelBuildResources()
{
    modelBuildDiscreteResources();
    modelBuildCompositeResources();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ClevorDataSet::modelBuildDiscreteResources()
{
    res_set_id_t::const_iterator it;
    for (it = _resources.begin(); it != _resources.end(); ++it)
    {
        cse::Resource* cseRes = *it;
        if (dynamic_cast<DiscreteResource*>(cseRes) == nullptr)
        {
            continue;
        }
        cse::DiscreteResource* cseDres = (cse::DiscreteResource*)cseRes;

        // make cls::DiscreteResource
        cls::DiscreteResource* clsDres = new cls::DiscreteResource(_schedule);
        clsDres->setId(cseDres->id());
        clsDres->setName(cseDres->name());
        clsDres->setObject(cseDres->cost());
        cseDres->setCLSresource(clsDres);
        _schedule->add(clsDres);
    }
    modelBuildDiscreteResourceCalendars();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ClevorDataSet::modelBuildDiscreteResourceCalendars()
{
    ResourceCalendarMgr* calMgr = _schedule->calendarMgr();
    calMgr->setHorizonTS(_horizonTS);

    res_set_id_t::const_iterator it;
    for (it = _resources.begin(); it != _resources.end(); ++it)
    {
        cse::Resource* cseRes = *it;
        if (dynamic_cast<DiscreteResource*>(cseRes) == nullptr)
        {
            continue;
        }
        const cse::DiscreteResource* cseDres = (DiscreteResource*)cseRes;
        cls::DiscreteResource* clsDres = cseDres->clsResource();
        ResourceCalendar* resCal = cseDres->makeCurrentCalendar(_config);

        // add provided capacity
        ResourceCalendar::iterator spanIt;
        for (spanIt = resCal->begin(); spanIt != resCal->end(); ++spanIt)
        {
            ResourceCalendarSpan* rcs = (ResourceCalendarSpan*)*spanIt;
            clsDres->addProvidedCapacity(rcs->begin(), rcs->end(), rcs->capacity());
        }

        // add the calendar to the calendar mgr
        resCal = calMgr->add(resCal);
        clsDres->setCalendar(resCal);
        clsDres->initialize();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ClevorDataSet::modelBuildCompositeResources()
{
    // create CompositeResource objects
    // and link discrete => composite
    res_set_id_t::const_iterator it;
    for (it = _resources.begin(); it != _resources.end(); ++it)
    {
        cse::Resource* cseRes = *it;
        if (!cseRes->isA(cse::CompositeResource))
            continue;
        cse::CompositeResource* cseCres = (cse::CompositeResource*)cseRes;
        uint_t resGroupId = cseCres->resourceGroupId();
        ASSERTD(resGroupId != uint_t_max);

        // make cls::CompositeResource
        cls::CompositeResource* clsCres = new cls::CompositeResource(_schedule);
        clsCres->setId(cseCres->id());
        clsCres->setName(cseCres->name());

        // add resources to cls::CompositeResource
        const ResourceGroup* resGroup = findResourceGroup(resGroupId);
        ASSERTD(resGroup != nullptr);
        const uint_set_t& resIds = resGroup->resIds();
        uint_set_t::const_iterator resIdIt;
        for (resIdIt = resIds.begin(); resIdIt != resIds.end(); ++resIdIt)
        {
            uint_t resId = *resIdIt;
            cse::Resource* res = findResource(resId);
            ASSERTD(res->isA(cse::DiscreteResource));
            cls::Resource* resCLSres = res->clsResource();
            ASSERTD(resCLSres->isA(cls::DiscreteResource));
            cls::DiscreteResource* resCLSdres = (cls::DiscreteResource*)resCLSres;
            resCLSdres->addCRid(cseCres->id());
        }

        // add clsCres to Schedule
        cseCres->clsResource() = clsCres;
        _schedule->add(clsCres);
    }

    // serialize resources in schedule
    _schedule->serializeResources();

    // now translate cls::DiscreteResource::resIds to serial-id
    // and add capacity to composite timetables
    for (it = _resources.begin(); it != _resources.end(); ++it)
    {
        cse::Resource* cseRes = *it;
        if (!cseRes->isA(cse::DiscreteResource))
            continue;
        cls::DiscreteResource* clsDres = (cls::DiscreteResource*)cseRes->clsResource();
        uint_vector_t& crIds = clsDres->crIds();
        uint_vector_t::iterator it;
        for (it = crIds.begin(); it != crIds.end(); ++it)
        {
            uint_t id = *it;
            cse::Resource* cseRes = findResource(id);
            cls::Resource* clsRes = cseRes->clsResource();
            ASSERTD(clsRes->isA(cls::CompositeResource));
            cls::CompositeResource* clsCres = (cls::CompositeResource*)clsRes;
            *it = cseRes->clsResource()->serialId();
            clsCres->add(clsDres);
        }
    }

    // initialize composite resource timetables
    for (it = _resources.begin(); it != _resources.end(); ++it)
    {
        cse::Resource* cseRes = *it;
        if (!cseRes->isA(cse::CompositeResource))
            continue;
        cls::CompositeResource* clsCres = (cls::CompositeResource*)cseRes->clsResource();
        clsCres->initialize();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ClevorDataSet::modelBuildResourceCts()
{
    modelBuildDiscreteResourceCts();
    modelBuildResourceGroupCts();
    modelBuildCompositeResourceCts();

    // init requirements
    jobop_set_id_t::iterator it;
    for (it = _ops.begin(); it != _ops.end(); ++it)
    {
        JobOp* op = *it;

        if (!op->schedulable())
        {
            continue;
        }

        // breakable
        if (op->breakable())
        {
            BrkActivity* act = op->brkact();
            act->initRequirements();
            continue;
        }

        // interruptible
        if (op->interruptible())
        {
            IntActivity* act = op->intact();
            act->initRequirements();
            continue;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ClevorDataSet::modelBuildDiscreteResourceCts()
{
    jobop_set_id_t::const_iterator it;
    for (it = _ops.begin(); it != _ops.end(); ++it)
    {
        JobOp* op = *it;

        // skip non-breakable op
        if (!op->breakable())
        {
            continue;
        }

        BrkActivity* act = op->brkact();
        ASSERTD(act != nullptr);

        // spin through resource requirements
        uint_t numResReqs = op->numResReqs();
        for (uint_t i = 0; i < numResReqs; ++i)
        {
            cse::ResourceRequirement* cseResReq = op->getResReq(i);
            uint_t resId = cseResReq->resourceId();
            cse::Resource* cseRes = findResource(resId);
            ASSERTD(cseRes != nullptr);

            // skip non-discrete resource
            if (!cseRes->isA(DiscreteResource))
            {
                continue;
            }

            DiscreteResource* cseDres = (DiscreteResource*)cseRes;
            cls::DiscreteResource* clsDres = (cls::DiscreteResource*)cseDres->clsResource();

            // determine cap, pt (may not be defined)
            uint_t cap, pt;
            cap = cseResReq->capacity();
            if (cap == uint_t_max)
                cap = cseResReq->scheduledCapacity();
            if (act->frozenPt() == uint_t_max)
            {
                pt = op->processingTime();
                if (pt != uint_t_max)
                    pt = _config->durationToTimeSlot(pt);
            }
            else // frozen
            {
                op->removeResCapPtsAdj(resId);
                pt = act->frozenPt();
            }

            // define ResourceCapPts for the resource
            ResourceCapPts* resCapPts = op->resCapPtsAdj(resId);
            if (resCapPts == nullptr)
            {
                ASSERTD(pt != uint_t_max);
                resCapPts = new ResourceCapPts();
                resCapPts->resourceId() = resId;
                op->resCapPtsAdj() += resCapPts;
            }
            resCapPts->setResource(clsDres);
            if ((cap != uint_t_max) && (pt != uint_t_max))
            {
                resCapPts->addCapPt(cap, pt);
            }

            // add resource requirement
            cls::DiscreteResourceRequirement* drr =
                new cls::DiscreteResourceRequirement(act, resCapPts);
            cseResReq->clsResReq() = drr;
            act->add(drr);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ClevorDataSet::modelBuildResourceGroupCts()
{
    jobop_set_id_t::const_iterator it;
    for (it = _ops.begin(); it != _ops.end(); ++it)
    {
        JobOp* op = *it;

        // skip non-breakable or frozen op
        if (!op->breakable() || op->frozen())
        {
            continue;
        }

        BrkActivity* act = op->brkact();
        ASSERTD(act != nullptr);

        // spin through resource group requirements
        uint_t numResGroupReqs = op->numResGroupReqs();
        for (uint_t i = 0; i < numResGroupReqs; ++i)
        {
            cse::ResourceGroupRequirement* cseResGroupReq = op->getResGroupReq(i);
            ResourceGroup* resGroup = findResourceGroup(cseResGroupReq->resourceGroupId());
            ASSERTD(resGroup != nullptr);

            // build res-cap-pts set
            utl::RBtree rcps(false);

            // iterate over all resources in the group
            const uint_set_t& resIds = resGroup->resIds();
            uint_set_t::const_iterator residIt;
            for (residIt = resIds.begin(); residIt != resIds.end(); ++residIt)
            {
                uint_t resId = *residIt;
                cse::Resource* res = findResource(resId);
                ASSERTD(res != nullptr);

                // skip past non-discrete resource
                const cse::DiscreteResource* dres = dynamic_cast<cse::DiscreteResource*>(res);
                if (dres == nullptr)
                    continue;

                // get res-cap-pts for this resource
                ResourceCapPts* resCapPts = op->resCapPtsAdj(resId);

                // ensure ResourceCapPts defined for the resource
                if (resCapPts == nullptr)
                {
                    ASSERTD(op->processingTime() != uint_t_max);
                    ASSERTD(cseResGroupReq->capacity() != uint_t_max);
                    resCapPts = new ResourceCapPts();
                    resCapPts->resourceId() = resId;
                    resCapPts->addCapPt(cseResGroupReq->capacity(),
                                        _config->durationToTimeSlot(op->processingTime()));
                    op->resCapPtsAdj() += resCapPts;
                }
                resCapPts->setResource(dres->clsResource());
                rcps += resCapPts;
            }

            // post resource constraint
            cls::DiscreteResourceRequirement* drr = new cls::DiscreteResourceRequirement(act, rcps);
            cseResGroupReq->clsResReq() = drr;
            act->add(drr);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ClevorDataSet::modelBuildCompositeResourceCts()
{
    jobop_set_id_t::const_iterator it;
    for (it = _ops.begin(); it != _ops.end(); ++it)
    {
        JobOp* op = *it;

        // skip non-interruptible
        if (!op->interruptible())
        {
            continue;
        }

        IntActivity* act = op->intact();

        // spin through resource requirements
        uint_t numResReqs = op->numResReqs();
        for (uint_t i = 0; i < numResReqs; ++i)
        {
            cse::ResourceRequirement* cseResReq = op->getResReq(i);
            uint_t resId = cseResReq->resourceId();
            cse::Resource* cseRes = findResource(resId);
            ASSERTD(cseRes != nullptr);

            // skip non-composite resource
            if (!cseRes->isA(CompositeResource))
            {
                continue;
            }

            CompositeResource* cseCres = (CompositeResource*)cseRes;
            cls::CompositeResource* clsCres = (cls::CompositeResource*)cseCres->clsResource();

            // determine cap, pt (may not be defined)
            uint_t cap = cseResReq->capacity();

            // min capacity must be defined - else requirement is ignored
            if (cap == uint_t_max)
            {
                continue;
            }

            // preferred-resources
            //     convert resource-ids to serial-ids
            PreferredResources* pr = utl::clone(cseResReq->preferredResources());
            if (pr != nullptr)
            {
                uint_vector_t& resIds = pr->resIds();
                uint_vector_t::iterator it;
                for (it = resIds.begin(); it != resIds.end(); ++it)
                {
                    uint_t id = *it;
                    cse::Resource* cseRes = findResource(id);
                    if (cseRes == nullptr)
                    {
                        *it = uint_t_max;
                        continue;
                    }
                    cls::Resource* clsRes = cseRes->clsResource();
                    ASSERTD(clsRes != nullptr);
                    *it = clsRes->serialId();
                }
            }

            // add resource requirement
            cls::CompositeResourceRequirement* crr = new cls::CompositeResourceRequirement(
                act, clsCres, cap / 100, cseResReq->maxCapacity() / 100);
            crr->setPreferredResources(pr);
            cseResReq->clsResReq() = crr;
            act->add(crr);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ClevorDataSet::modelBuildHeuristics()
{
    _minCostHeuristics = new MinCostHeuristics();
    _minCostHeuristics->initialize(this);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(cse::JobOpIncSDordering);
UTL_CLASS_IMPL(cse::JobOpDecSDordering);
UTL_CLASS_IMPL(cse::ClevorDataSet);
