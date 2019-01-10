#include "libclp.h"
#include "BoundCt.h"
#include "ConstrainedBound.h"
#include "BoundPropagator.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef DEBUG
//#define DEBUG_UNIT
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(clp::BoundPropagator, utl::Object);

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

void
BoundPropagator::addBoundCt(ConstrainedBound* src, ConstrainedBound* dst, int d, bool cycleCheck)
{
    // initially constrain dst bound
    dst->set(src->get() + d);
    BoundCt* bct = new BoundCt(src->last() + d, src, dst);
    _mgr->revAllocate(bct);

    // link src => dst
    ASSERTD(dst != src);
    if (dst->type() == bound_lb)
    {
        src->_lbCts.add(bct);
    }
    else
    {
        src->_ubCts.add(bct);
    }

    // add a precedence-link between the two CGs (and handle cycles)
    addPrecedenceLink(src->cycleGroup(), dst->cycleGroup(), cycleCheck);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
BoundPropagator::addPrecedenceLink(CycleGroup* src, CycleGroup* dst, bool cycleCheck)
{
    // do nothing else if src and dst are the same
    if (src == dst)
    {
        return;
    }

    // add (src,dst) precedence link
    bool cycleFound = dst->addPred(src, cycleCheck);
    if (cycleFound)
    {
        dfs(src, dst);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
BoundPropagator::staticAnalysis()
{
    setSuccessorDepth();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
BoundPropagator::unsuspendInitial()
{
    cg_set_id_t initialCGs;
    cg_revset_t::iterator rit;
    for (rit = _cgs.begin(); rit != _cgs.end(); ++rit)
    {
        CycleGroup* cg = *rit;
        if (!cg->suspended())
        {
            initialCGs.insert(cg);
        }
    }

    cg_set_id_t::iterator it;
    for (it = initialCGs.begin(); it != initialCGs.end(); ++it)
    {
        CycleGroup* initialCG = *it;
        initialCG->unsuspend();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
BoundPropagator::unsuspend(ConstrainedBound* cb)
{
    cb->invalidate();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
BoundPropagator::finalize(ConstrainedBound* cb)
{
    CycleGroup* cg = cb->cycleGroup();
    // added the if-condition line, and commented out the ASSERTD(..)
    // because this finalize() function is called by Job::xxx to
    // release an inactive job's successor jobs for scheduling.
    // the if-condition is used to stop to finalize the cg again
    // during normal scheduling. Joe, Nov 1, 2006
    if (cg->finalized())
        return;
    //     ASSERTD(!cg->suspended());
    cb->queueFind();
    cg->finalizeMember();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CycleGroup*
BoundPropagator::newCycleGroup(CycleGroup* cg)
{
    //     static uint_t cgId = 0;
    if (cg == nullptr)
    {
        cg = new CycleGroup(_mgr);
    }
    cg->id() = _initCgId++;
    //     cg->id() = cgId++;
    _mgr->revAllocate(cg);
    _cgs.add(cg);
    return cg;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
BoundPropagator::propagate()
{
    while (!_propQ.empty())
    {
        // pick up a bound and propagate locally
        ConstrainedBound* bound = _propQ.deQ();
        _boundInProcess = bound;
        bound->propagate();
        bound->queued() = false;
    }
    _boundInProcess = nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
BoundPropagator::clearPropQ()
{
    if (_boundInProcess != nullptr)
    {
        _boundInProcess->queued() = false;
        _boundInProcess = nullptr;
    }
    while (!_propQ.empty())
    {
        ConstrainedBound* bound = _propQ.deQ();
        bound->queued() = false;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef DEBUG
void
BoundPropagator::enQ(ConstrainedBound* bound)
{
    if (bound->debugFlag())
    {
        BREAKPOINT;
    }
    _propQ.enQ(bound);
}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////

void
BoundPropagator::init(Manager* mgr)
{
    _mgr = mgr;
    _boundInProcess = nullptr;
    _restartCG = nullptr;
    _cgs.initialize(mgr);

    _cgArray = _cgArrayPtr = _cgArrayLim = nullptr;
    _cgArraySize = 0;

    _initCgId = 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
BoundPropagator::deInit()
{
    delete[] _cgArray;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
BoundPropagator::dfs(CycleGroup* src, CycleGroup* dst)
{
    ASSERTD(src != dst);

    // first perform DFS starting with (src,dst)
    dfs_push(src);
    dfs(dst);
    dfs_pop();

    // keep re-starting the search until no more cycles are found
    while (_restartCG != nullptr)
    {
        CycleGroup* restartCG = _restartCG;
        _restartCG = nullptr;
        dfs(restartCG);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
BoundPropagator::dfs(CycleGroup* cg)
{
    // already visited cg ==> make cycle group
    if (cg->visited())
    {
        ASSERTD(cg->visitedIdx() == 0);
        makeCycleGroup();
        return;
    }

    // does cg lead toward a cycle?
    CycleGroup* firstCG = _cgArray[0];
    const cg_revset_t& cgAllSucc = cg->allSuccCGs();
    if (!cgAllSucc.has(firstCG))
    {
        return;
    }

    // put cg onto the stack
    dfs_push(cg);

    // iterate thru cg's successors
    CycleGroup::cg_iterator it, lim = cg->succCGs().end();
    for (it = cg->succCGs().begin(); it != lim; ++it)
    {
        CycleGroup* succCG = *it;
        dfs(succCG);
        if (_restartCG != nullptr)
        {
            break;
        }
    }

    // pop cb off the stack
    dfs_pop();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
BoundPropagator::dfs_push(CycleGroup* cg)
{
    ASSERTD(!cg->visited());

    // grow stack if necessary
    if (_cgArrayPtr == _cgArrayLim)
    {
        utl::arrayGrow(_cgArray, _cgArrayPtr, _cgArrayLim);
        _cgArraySize = _cgArrayLim - _cgArray;
    }

    // record cg's position in the array
    cg->visitedIdx() = (_cgArrayPtr - _cgArray);

    // push cg onto stack
    *_cgArrayPtr++ = cg;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
BoundPropagator::dfs_pop()
{
    ASSERTD(_cgArrayPtr > _cgArray);

    // remove cg from stack
    CycleGroup* cg = *(--_cgArrayPtr);

    // clear cg's visited flag
    cg->visitedIdx() = uint_t_max;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
BoundPropagator::makeCycleGroup()
{
    ASSERTD((_cgArrayPtr - _cgArray) >= 2);

    // find largest cycle group
    CycleGroup* motherCG = _cgArray[0];
    //uint_t motherCGidx = 0;
    uint_t motherCGsize = motherCG->size();
    CycleGroup** cgIt;
    CycleGroup** cgLim = _cgArrayPtr;
    for (cgIt = (_cgArray + 1); cgIt != cgLim; ++cgIt)
    {
        CycleGroup* cg = *cgIt;
        if (cg->size() > motherCGsize)
        {
            motherCG = cg;
            //motherCGidx = (cgIt - _cgArray);
            motherCGsize = motherCG->size();
        }
    }

    // remove links between motherCG and other members of new cycle

    // for each predecessor of motherCG
    cg_revset_t::iterator predIt, predNext;
    cg_revset_t::iterator predLim = motherCG->allPredCGs().end();
    for (predIt = motherCG->allPredCGs().begin(); predIt != predLim; predIt = predNext)
    {
        predNext = predIt;
        ++predNext;

        CycleGroup* predCG = *predIt;

        // skip predCG if it's not part of the new cycle
        if (predCG->visitedIdx() == uint_t_max)
            continue;

        // remove the (predCG ==> motherCG) link
        motherCG->removePred(predCG);
    }

    // for each successor of motherCG
    cg_revset_t::iterator succIt, succNext;
    cg_revset_t::iterator succLim = motherCG->allSuccCGs().end();
    for (succIt = motherCG->allSuccCGs().begin(); succIt != succLim; succIt = succNext)
    {
        succNext = succIt;
        ++succNext;

        CycleGroup* succCG = *succIt;

        // skip succCG if it's not part of the new cycle
        if (succCG->visitedIdx() == uint_t_max)
            continue;

        // remove the (succCG => cg) link
        succCG->removePred(motherCG);
    }

    // motherCG eclipses all others
    for (cgIt = _cgArray; cgIt != cgLim; ++cgIt)
    {
        // if cg is not motherCG, then clear its indirect lists
        CycleGroup* cg = *cgIt;
        if (cg == motherCG)
            continue;
        cg->clearIndirectLists();
    }
    for (cgIt = _cgArray; cgIt != cgLim; ++cgIt)
    {
        // if cg is not motherCG, then motherCG swallows it
        CycleGroup* cg = *cgIt;
        if (cg == motherCG)
            continue;
        motherCG->eclipse(cg);
        _cgs.remove(cg);
    }

    // restart DFS at motherCG
    _restartCG = motherCG;

#ifdef DEBUG_UNIT
    sanityCheckCGs();
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
BoundPropagator::setSuccessorDepth()
{
    cg_revset_t::iterator it;

    // all CGs have successor-depth = 0 initially
    for (it = _cgs.begin(); it != _cgs.end(); ++it)
    {
        CycleGroup* cg = *it;
        cg->successorCount() = cg->succCGs().size();
        cg->successorDepth() = 0;
    }

    // recursively set successor-depth
    // (starting at CGs with no successors)
    cg_set_id_t zsdCGs;
    for (it = _cgs.begin(); it != _cgs.end(); ++it)
    {
        CycleGroup* cg = *it;
        if (cg->successorCount() == 0)
        {
            zsdCGs.insert(cg);
        }
    }
    cg_set_id_t::iterator zsdIt;
    for (zsdIt = zsdCGs.begin(); zsdIt != zsdCGs.end(); ++zsdIt)
    {
        CycleGroup* zsdCG = *zsdIt;
        setSuccessorDepth(zsdCG);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
BoundPropagator::setSuccessorDepth(CycleGroup* cg)
{
    uint_t sd = cg->successorDepth();

    cg_revset_t::iterator it, endIt = cg->predCGs().end();
    for (it = cg->predCGs().begin(); it != endIt; ++it)
    {
        CycleGroup* predCG = *it;
        uint_t& predSC = predCG->successorCount();
        uint_t& predSD = predCG->successorDepth();
        predSD = utl::max(predSD, sd + 1);
        ASSERTD(predSC > 0);
        if (--predSC == 0)
        {
            setSuccessorDepth(predCG);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef DEBUG
void
BoundPropagator::sanityCheckCGs()
{
    cg_revset_t::iterator it;
    for (it = _cgs.begin(); it != _cgs.end(); ++it)
    {
        CycleGroup* cg = *it;
        sanityCheckCG(cg);
    }
}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef DEBUG
void
BoundPropagator::sanityCheckCG(CycleGroup* cg)
{
    // check predecessors
    uint_t num = 0;
    CycleGroup* last = nullptr;
    cg_revset_t::iterator predIt;
    for (predIt = cg->predCGs().begin(); predIt != cg->predCGs().end(); ++predIt, ++num)
    {
        CycleGroup* predCG = *predIt;
        ASSERT(_cgs.has(predCG));
        ASSERT(predCG > last);
        last = predCG;
        ASSERT(predCG != cg);
        ASSERT(predCG->succCGs().has(cg));
    }
    ASSERT(num == cg->predCGs().size());

    // check successors
    num = 0;
    last = nullptr;
    cg_revset_t::iterator succIt;
    for (succIt = cg->succCGs().begin(); succIt != cg->succCGs().end(); ++succIt, ++num)
    {
        CycleGroup* succCG = *succIt;
        ASSERT(_cgs.has(succCG));
        ASSERT(succCG > last);
        last = succCG;
        ASSERT(succCG != cg);
        ASSERT(succCG->predCGs().has(cg));
    }
    ASSERT(num == cg->succCGs().size());
}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_END;
