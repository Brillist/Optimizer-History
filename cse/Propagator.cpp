#include "libcse.h"
#include <cls/SchedulableBound.h>
#include "Job.h"
#include "Propagator.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;
CLP_NS_USE;
CLS_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(cse::Propagator);

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Propagator::unsuspend(ConstrainedBound* cb)
{
    // not schedulable => just finalize it
    if (!cb->isA(SchedulableBound))
    {
        finalize(cb);
        return;
    }

    // reference sb/job/op/act
    SchedulableBound* sb = (SchedulableBound*)cb;
    Activity* act = (Activity*)sb->owner();
    JobOp* op = (JobOp*)act->owner();
    Job* job = op->job();

    // SchedulableBound but non-schedulable op => just finalize it
    if (!op->schedulable())
    {
        finalize(cb);
        return;
    }

    // update schedulable jobs/ops
    if (job->sopsEmpty())
    {
        _context->sjobsAdd(job);
    }
    job->sopsAdd(op);

    // register for timetable events
    sb->invalidate();
    sb->registerEvents();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Propagator::finalize(ConstrainedBound* cb)
{
    // not schedulable => just let superclass do its finalize()
    if (!cb->isA(SchedulableBound))
    {
        BoundPropagator::finalize(cb);
        return;
    }

    // reference sb/job/op/act
    SchedulableBound* sb = (SchedulableBound*)cb;
    Activity* act = (Activity*)sb->owner();
    JobOp* op = (JobOp*)act->owner();
    Job* job = op->job();

    // let superclass do its finalize()
    BoundPropagator::finalize(cb);

    // propagate to calculate bound
    _mgr->propagate();

    // SchedulableBound but non-schedulable op => do nothing else
    if (!op->schedulable())
        return;

    ASSERTD(act->ef() >= (act->es() - 1));

    // update schedulable jobs/ops
    job->sopsRemove(op);
    if (job->sopsEmpty())
    {
        _context->sjobsRemove(job);
    }

    // allocate capacity
    sb->allocateCapacity();

    // remember that we allocated capacity for the activity
    ASSERTD(!act->allocated());
    _mgr->revToggle(act->allocated());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;
