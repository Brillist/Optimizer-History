#include "libcse.h"
#include <clp/BoundPropagator.h>
#include <cse/Job.h>
#include <cse/JobOpOrdering.h>
#include <libutl/BufferedFDstream.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef DEBUG
#define DEBUG_UNIT
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;
CLP_NS_USE;
CLS_NS_USE;
GOP_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL_ABC(cse::JobOrdering);
UTL_CLASS_IMPL(cse::JobOrderingDecSuccessorDepth);
UTL_CLASS_IMPL(cse::JobOrderingDecLatenessCost);
UTL_CLASS_IMPL(cse::JobOrderingIncDueTime);
UTL_CLASS_IMPL(cse::JobOrderingIncSID);
UTL_CLASS_IMPL_ABC(cse::OpOrdering);
UTL_CLASS_IMPL(cse::OpOrderingFrozenFirst);
UTL_CLASS_IMPL(cse::OpOrderingIncES);
UTL_CLASS_IMPL(cse::OpOrderingIncSID);
UTL_CLASS_IMPL(cse::JobOpOrderingIncSID);

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////
// JobOrdering //////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void
JobOrdering::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(JobOrdering));
    const JobOrdering& jOrdering = (const JobOrdering&)rhs;
    _nextOrdering = utl::clone(jOrdering._nextOrdering);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// JobOrderingDecSuccessorDepth ////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

int
JobOrderingDecSuccessorDepth::cmp(const Object* lhs, const Object* rhs) const
{
    ASSERTD(lhs->isA(Job));
    ASSERTD(rhs->isA(Job));
    Job* lhsJob = (Job*)lhs;
    Job* rhsJob = (Job*)rhs;
    uint_t lhsSD = lhsJob->successorDepth();
    uint_t rhsSD = rhsJob->successorDepth();
    ASSERTD(lhsSD >= 0);
    ASSERTD(rhsSD >= 0);
    int cmpResult = utl::compare(rhsSD, lhsSD); //Dec: rhs first
                                                //     utl::cout << "lhsJob:" << lhsJob->id()
    //               << "(" << lhsJob->successorDepth() << ")"
    //               << ", rhsJob:" << rhsJob->id()
    //               << "(" << rhsJob->successorDepth() << ")"
    //               << ", cmpResult:" << cmpResult
    //               << utl::endlf;
    if (cmpResult == 0 && nextOrdering())
        return nextOrdering()->cmp(lhs, rhs);
    return cmpResult;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// JobOrderingDecLatenessCost //////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

int
JobOrderingDecLatenessCost::cmp(const Object* lhs, const Object* rhs) const
{
    ASSERTD(lhs->isA(Job));
    ASSERTD(rhs->isA(Job));
    Job* lhsJob = (Job*)lhs;
    Job* rhsJob = (Job*)rhs;
    double lhsCost = lhsJob->latenessCost();
    double rhsCost = rhsJob->latenessCost();
    ASSERTD(lhsCost >= 0);
    ASSERTD(rhsCost >= 0);
    int cmpResult = utl::compare(rhsCost, lhsCost); //Dec: rhs first
                                                    //     utl::cout << "lhsJob:" << lhsJob->id()
    //               << "(" << lhsJob->latenessCost() << ")"
    //               << ", rhsJob:" << rhsJob->id()
    //               << "(" << rhsJob->latenessCost() << ")"
    //               << ", cmpResult:" << cmpResult
    //               << utl::endlf;
    if (cmpResult == 0 && nextOrdering())
        return nextOrdering()->cmp(lhs, rhs);
    return cmpResult;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// JobOrderingDecDueTime ///////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

int
JobOrderingIncDueTime::cmp(const Object* lhs, const Object* rhs) const
{
    ASSERTD(lhs->isA(Job));
    ASSERTD(rhs->isA(Job));
    Job* lhsJob = (Job*)lhs;
    Job* rhsJob = (Job*)rhs;
    time_t lhsDueTime = lhsJob->dueTime();
    time_t rhsDueTime = rhsJob->dueTime();
    ASSERTD(lhsDueTime >= -1);
    ASSERTD(rhsDueTime >= -1);
    int cmpResult = 0;
    if ((lhsDueTime > -1) && (rhsDueTime > -1))
    {
        cmpResult = utl::compare(lhsDueTime, rhsDueTime);
    }
    else
    // treat as OrderingDec, because:
    //       case 1: (lhs=-1, rhs=-1) = 0;
    //       case 2: (lhs=-1, rhs=>-1) = 1;
    //       case 3: (lhs>-1, rhs=-1) = -1
    {
        cmpResult = utl::compare(rhsDueTime, lhsDueTime);
    }
    if (cmpResult == 0 && nextOrdering())
        return nextOrdering()->cmp(lhs, rhs);
    return cmpResult;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// JobOrderingIncSID ///////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

int
JobOrderingIncSID::cmp(const Object* lhs, const Object* rhs) const
{
    ASSERTD(lhs->isA(Job));
    ASSERTD(rhs->isA(Job));
    Job* lhsJob = (Job*)lhs;
    Job* rhsJob = (Job*)rhs;
    int cmpResult = utl::compare(lhsJob->serialId(), rhsJob->serialId());
    if (cmpResult == 0 && nextOrdering())
        return nextOrdering()->cmp(lhs, rhs);
    return cmpResult;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// OpOrdering //////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

bool
OpOrdering::isSchedulable(const JobOp*) const
{
    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// OpOrderingFrozenFirst ///////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

int
OpOrderingFrozenFirst::cmp(const Object* lhs, const Object* rhs) const
{
    ASSERTD(lhs->isA(JobOp));
    ASSERTD(rhs->isA(JobOp));
    JobOp* lhsOp = (JobOp*)lhs;
    JobOp* rhsOp = (JobOp*)rhs;
    ASSERTD(lhsOp->activity() != nullptr);
    ASSERTD(rhsOp->activity() != nullptr);
    if (lhsOp->frozen() == rhsOp->frozen())
    {
        // unfrozen ops are equal
        if (!lhsOp->frozen())
            return 0;

        // note: both ops are frozen
        int lhsES = lhsOp->activity()->es();
        int rhsES = rhsOp->activity()->es();

        int res = utl::compare(lhsES, rhsES);
        if (res != 0)
            return res;
        return utl::compare(lhsOp->id(), rhsOp->id());
    }
    else
    {
        return lhsOp->frozen() ? 1 : -1;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool
OpOrderingFrozenFirst::isSchedulable(const JobOp* op) const
{
    return op->frozen();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// OpOrderingIncES /////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

int
OpOrderingIncES::cmp(const Object* lhs, const Object* rhs) const
{
    ASSERTD(lhs->isA(JobOp));
    ASSERTD(rhs->isA(JobOp));
    JobOp* lhsOp = (JobOp*)lhs;
    JobOp* rhsOp = (JobOp*)rhs;
    Activity* lhsAct = lhsOp->activity();
    Activity* rhsAct = rhsOp->activity();
    ASSERTD(lhsAct != nullptr);
    ASSERTD(rhsAct != nullptr);

    // op with hard unary constraint takes precedence
    int res = utl::compare((int)rhsOp->hasHardCt(), (int)lhsOp->hasHardCt());
    if (res != 0)
        return res;

    // make sure ES bounds are valid
    ConstrainedBound& lhsES = lhsAct->esBound();
    ConstrainedBound& rhsES = rhsAct->esBound();
    BoundPropagator* bp = lhsES.boundPropagator();
    ASSERTD(bp != nullptr);
    if (!lhsES.valid() || !rhsES.valid())
    {
        if (!lhsES.valid())
        {
            bp->enQ(lhsES);
        }
        if (!rhsES.valid())
        {
            bp->enQ(rhsES);
        }
        bp->propagate();
    }

    // op with minimum ES takes precedence
    res = utl::compare(lhsES.get(), rhsES.get());
    //     utl::cout << "lhsOp:" << lhsOp->id()
    //               << "(" << lhsES.get() << ")"
    //               << ", rhsOp:" << rhsOp->id()
    //               << "(" << rhsES.get() << ")"
    //               << ", cmpResult:" << res
    //               << utl::endlf;
    if (res != 0)
        return res;

    // failing any other difference, compare id's
    return utl::compare(lhsOp->id(), rhsOp->id());
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// OpOrderingIncSID ////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

int
OpOrderingIncSID::cmp(const Object* lhs, const Object* rhs) const
{
    ASSERTD(lhs->isA(JobOp));
    ASSERTD(rhs->isA(JobOp));
    JobOp* lhsOp = (JobOp*)lhs;
    JobOp* rhsOp = (JobOp*)rhs;
    ASSERTD(lhsOp->activity() != nullptr);
    ASSERTD(rhsOp->activity() != nullptr);

    return utl::compare(lhsOp->serialId(), rhsOp->serialId());
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// JobOpOrderingIncSID /////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

int
JobOpOrderingIncSID::cmp(const Object* lhs, const Object* rhs) const
{
    ASSERTD(lhs->isA(JobOp));
    ASSERTD(rhs->isA(JobOp));
    JobOp* lhsOp = (JobOp*)lhs;
    JobOp* rhsOp = (JobOp*)rhs;
    ASSERTD(lhsOp->activity() != nullptr);
    ASSERTD(rhsOp->activity() != nullptr);
    Job* lhsJob = lhsOp->job();
    Job* rhsJob = rhsOp->job();

    int cmpResult = utl::compare(lhsJob->serialId(), rhsJob->serialId());
    if (cmpResult == 0)
    {
        return utl::compare(lhsOp->serialId(), rhsOp->serialId());
    }
    else
    {
        return cmpResult;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;
