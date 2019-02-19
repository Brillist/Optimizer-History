#include "libcse.h"
#include <libutl/Array.h>
#include <libutl/Time.h>
#include <libutl/Bool.h>
#include <clp/BoundPropagator.h>
#include <cls/ESbound.h>
#include <cls/EFbound.h>
#include <cls/LSbound.h>
#include <cls/LFbound.h>
#include <cse/ClevorDataSet.h>
#include "Job.h"
#include "SummaryOp.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;
CLP_NS_USE;
CLS_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// JobReleaseCG ///////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

class JobReleaseCG : public CycleGroup
{
    UTL_CLASS_DECL(JobReleaseCG, CycleGroup);
    UTL_CLASS_DEFID;

public:
    /** Constructor. */
    JobReleaseCG(Manager* mgr)
        : CycleGroup(mgr)
    {
    }

    virtual void unsuspend();
};

////////////////////////////////////////////////////////////////////////////////////////////////////

void
JobReleaseCG::unsuspend()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Job  ///////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

int
Job::compare(const utl::Object& rhs) const
{
    if (!rhs.isA(Job))
    {
        return Object::compare(rhs);
    }
    const Job& j = (const Job&)rhs;
    int res = utl::compare(_id, j._id);
    return res;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Job::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(Job));
    const Job& job = (const Job&)rhs;
    clear();
    _id = job._id;
    _serialId = job._serialId;
    _preference = job._preference;
    _active = job._active;
    _workOrderIds = job._workOrderIds;
    _name = job._name;
    _groupId = job._groupId;
    _rootSummaryOp = job._rootSummaryOp;
    for (Job::const_iterator it = job.begin(); it != job.end(); ++it)
    {
        JobOp& op = **it;
        add(op);
    }
    _itemId = job._itemId;
    _itemQuantity = job._itemQuantity;
    _makespan = job._makespan;
    _frozenMakespan = 0;
    _releaseTime = job._releaseTime;
    _dueTime = job._dueTime;
    _status = job._status;
    _opportunityCost = job._opportunityCost;
    _opportunityCostPeriod = job._opportunityCostPeriod;
    _latenessCost = job._latenessCost;
    _latenessCostPeriod = job._latenessCostPeriod;
    /* November 21, 2013 (Elisa) */
    /* copy lateness cost increment */
    _latenessIncrement = job._latenessIncrement;
    /* January 2, 2014 (Elisa) */
    /* copy the overhead cost and period */
    _overheadCost = job._overheadCost;
    _overheadCostPeriod = job._overheadCostPeriod;
    _inventoryCost = job._inventoryCost;
    _inventoryCostPeriod = job._inventoryCostPeriod;
    _schedulableJobsIdx = uint_t_max;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Job::serialize(Stream& stream, uint_t io, uint_t)
{
    utl::serialize(_id, stream, io);
    lut::serialize(_name, stream, io);
    utl::serialize(_preference, stream, io);
    utl::serialize(_groupId, stream, io);
    lut::serialize(_workOrderIds, stream, io);
    utl::serialize(_itemId, stream, io);
    utl::serialize(_itemQuantity, stream, io);
    utl::serialize(_releaseTime, stream, io);
    utl::serialize(_dueTime, stream, io);

    // set _active based on _status when read
    // change _status based on _active when write
    if (io == io_rd)
    {
        utl::serialize((uint_t&)_status, stream, io);
        if ((_status == jobstatus_inactive) || (_status == jobstatus_undefined))
        {
            _active = false;
        }
        else
        {
            _active = true;
        }
    }
    else
    {
        if (_active == false)
            _status = jobstatus_inactive;
        utl::serialize((uint_t&)_status, stream, io);
    }

    utl::serialize(_opportunityCost, stream, io);
    utl::serialize((uint_t&)_opportunityCostPeriod, stream, io);
    utl::serialize(_latenessCost, stream, io);
    utl::serialize((uint_t&)_latenessCostPeriod, stream, io);
    /* November 21, 2013 (Elisa) */
    /* add serialization for lateness cost increment */
    utl::serialize(_latenessIncrement, stream, io);
    /* January 2, 2014 (Elisa) */
    /* add serialization for overhead cost and period */
    utl::serialize(_overheadCost, stream, io);
    utl::serialize((uint_t&)_overheadCostPeriod, stream, io);
    utl::serialize(_inventoryCost, stream, io);
    utl::serialize((uint_t&)_inventoryCostPeriod, stream, io);

    if (io == io_rd)
    {
        deleteCont(_ops);
        Array array;
        array.serializeIn(stream);
        for (auto op_ : array)
        {
            auto op = utl::cast<JobOp>(op_);
            add(op);
        }
        array.setOwner(false);
    }
    else
    {
        Array array(false);
        for (auto op : _ops)
        {
            array += op;
        }
        array.serializeOut(stream);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Job::clear()
{
    deleteCont(_ops);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Job::sopsInit(Manager* mgr)
{
    _sops.initialize(mgr);
    _allSops.clear();
    iterator it;
    for (it = begin(); it != end(); ++it)
    {
        JobOp* op = *it;
        if (op->schedulable())
        {
            _allSops.insert(op);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Job::sopsAdd(JobOp* op)
{
    ASSERTD(op->schedulableOpsIdx() == uint_t_max);
    op->schedulableOpsIdx() = _sops.size();
    _sops.add(op);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Job::sopsRemove(JobOp* op)
{
    ASSERTD(op->schedulableOpsIdx() != uint_t_max);
    uint_t idx = op->schedulableOpsIdx();
    uint_t endIdx = _sops.size() - 1;
    _sops.remove(idx);
    op->schedulableOpsIdx() = uint_t_max;
    if (idx < endIdx)
    {
        _sops[idx]->schedulableOpsIdx() = idx;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Job::scheduleClear()
{
    _makespan = _frozenMakespan;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

// void
// Job::schedule(JobOp* op, bool forward)
// {
//     // get activity reference
//     Activity* act = op->activity();
//     ASSERTD(act != nullptr);

//     // update makespan
//     if (forward)
//     {
//         _makespan = utl::max(_makespan, act->ef() + 1);
//     }
//     else
//     {
//         _makespan = utl::max(_makespan, act->lf() + 1);
//     }

//     // frozen op : update frozen makespan
//     if (op->frozen())
//     {
//         _frozenMakespan = _makespan;
//     }
// }

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Job::calculateMakespan(bool forward)
{
    jobop_set_id_t::iterator it, lim;
    lim = _ops.end();
    for (it = _ops.begin(); it != lim; ++it)
    {
        JobOp* op = *it;
        Activity* act = op->activity();
        ASSERT(act != nullptr);
        if (forward)
        {
            _makespan = utl::max(_makespan, act->ef() + 1);
        }
        else
        {
            _makespan = utl::max(_makespan, act->lf() + 1);
        }
        if (op->frozen())
        {
            if (forward)
            {
                _frozenMakespan = utl::max(_frozenMakespan, act->ef() + 1);
            }
            else
            {
                _frozenMakespan = utl::max(_frozenMakespan, act->lf() + 1);
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

uint_t
Job::tardiness() const
{
    if ((_dueTime == -1) || (_makespan < _dueTime))
    {
        return 0;
    }

    return (_makespan - _dueTime);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Job::add(JobOp* op)
{
    op->job() = this;
    _ops.insert(op);
    if (op->id() == _id)
    {
        ASSERTD(dynamic_cast<SummaryOp*>(op) != nullptr);
        SummaryOp* summaryOp = (SummaryOp*)op;
        _rootSummaryOp = summaryOp;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Job::initPrecedenceBound(Manager* mgr)
{
    // make _precedenceBound
    BoundPropagator* bp = mgr->boundPropagator();
    _precedenceBound = new ConstrainedBound(mgr, bound_lb, 0);

    //add name
    _precedenceBound->name() = "job-" + Uint(id()).toString() + " predBound";

    _precedenceBound->owner() = this;
    mgr->revAllocate(_precedenceBound);
    CycleGroup* cg = bp->newCycleGroup();
    cg->add(_precedenceBound);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Job::initReleaseBound(Manager* mgr)
{
    BoundPropagator* bp = mgr->boundPropagator();
    bool forward = _dataSet->schedulerConfig()->forward();

    // make _releaseBound
    _releaseBound =
        new ConstrainedBound(mgr, forward ? bound_lb : bound_ub, forward ? int_t_min : int_t_max);

    // add name
    _releaseBound->name() = "job-" + Uint(id()).toString() + " relsBound";

    _releaseBound->owner() = this;
    mgr->revAllocate(_releaseBound);
    CycleGroup* cg = bp->newCycleGroup(new JobReleaseCG(mgr));
    cg->add(_releaseBound);

    // make precedence links from _releaseBound to CGs of all schedulable ops
    CycleGroup* releaseCG = _releaseBound->cycleGroup();
    jobop_set_id_t::iterator it, lim;
    lim = _allSops.end();
    for (it = _allSops.begin(); it != lim; ++it)
    {
        JobOp* op = *it;
        if (op->frozen())
        {
            continue;
        }
        Activity* opAct = op->activity();
        ASSERTD(opAct != nullptr);
        CycleGroup* opCG = forward ? opAct->esBound().cycleGroup() : opAct->lfBound().cycleGroup();
        bp->addPrecedenceLink(releaseCG, opCG, false);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Job::release(Manager* mgr)
{
    BoundPropagator* bp = mgr->boundPropagator();
    bp->finalize(_releaseBound);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool
Job::released() const
{
    return _releaseBound->finalized();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Job::finalize(Manager* mgr)
{
    bool forward = _dataSet->schedulerConfig()->forward();
    BoundPropagator* bp = mgr->boundPropagator();
    ASSERTD(_rootSummaryOp != nullptr);
    Activity* act = _rootSummaryOp->activity();
    if (forward)
    {
        ConstrainedBound* cb = act->efBound();
        ASSERTD(cb->cycleGroup()->size() == 1);
        bp->BoundPropagator::finalize(cb);
    }
    else
    {
        ConstrainedBound* cb = act->lsBound();
        ASSERTD(cb->cycleGroup()->size() == 1);
        bp->BoundPropagator::finalize(cb);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

String
Job::toString() const
{
    MemStream str;
    str << "job: " << _id << ", active:" << Bool(_active).toString() << ", status:" << _status
        << ", rlsT:";
    if (_releaseTime == -1)
    {
        str << "null";
    }
    else
    {
        str << _releaseTime;
    }
    str << ", dueT:";
    if (_dueTime == -1)
    {
        str << "null";
    }
    else
    {
        str << _dueTime;
        //         time_t t = _dataSet->schedulerConfig()->timeSlotToTime(_dueTime);
        //         str << Time(t).toString("$yyyy/$m/$d $h/$nn");
    }
    str << ", sid:";
    if (_serialId == uint_t_max)
    {
        str << "null";
    }
    else
    {
        str << _serialId;
    }
    str << ", itemId:";
    if (_itemId != uint_t_max)
    {
        str << _itemId;
    }
    else
    {
        str << "null";
    }
    str << ", itemQnty:" << _itemQuantity;
    str << ", rootOp:";
    if (_rootSummaryOp != nullptr)
    {
        str << _rootSummaryOp->id();
    }
    else
    {
        str << "null";
    }
    str << ", ops:";
    for (jobop_set_id_t::const_iterator it = _ops.begin(); it != _ops.end(); it++)
    {
        if (it != _ops.begin())
            str << ",";
        str << (*it)->id();
    }
    str << ", sops:";
    for (jobop_set_id_t::const_iterator it = _allSops.begin(); it != _allSops.end(); it++)
    {
        if (it != _allSops.begin())
            str << ",";
        str << (*it)->id();
    }
    str << '\0';
    return String((char*)str.get());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Job::init()
{
    _dataSet = nullptr;
    _id = _serialId = _preference = _groupId = uint_t_max;
    _active = true;
    _precedenceBound = nullptr;
    _releaseBound = nullptr;
    _rootSummaryOp = nullptr;

    _itemId = uint_t_max;
    _itemQuantity = 0;
    _makespan = 0;
    _frozenMakespan = 0;
    _releaseTime = -1;
    _dueTime = -1;
    _status = jobstatus_undefined;
    _opportunityCost = 0.0;
    _opportunityCostPeriod = period_undefined;
    _latenessCost = 0.0;
    _latenessCostPeriod = period_undefined;
    /* November 21, 2013 (Elisa) */
    /* initialize lateness cost increment */
    _latenessIncrement = 0.0;
    _inventoryCost = 0.0;
    _inventoryCostPeriod = period_undefined;
    /* Janaury 2, 2014 (Elisa) */
    /* initialize overhead cost and period */
    _overheadCost = 0.0;
    _overheadCostPeriod = period_undefined;
    _schedulableJobsIdx = uint_t_max;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Job::deInit()
{
    clear();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(cse::JobReleaseCG);
UTL_CLASS_IMPL(cse::Job);
