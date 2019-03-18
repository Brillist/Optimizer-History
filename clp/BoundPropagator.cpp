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

UTL_CLASS_IMPL(clp::BoundPropagator);

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

void
BoundPropagator::addBoundCt(ConstrainedBound* src, ConstrainedBound* dst, int d, bool cycleCheck)
{
    // initially constrain dst bound
    dst->set(src->get() + d);
    auto bct = new BoundCt(src->last() + d, src, dst);
    _mgr->revAllocate(bct);

    // link src -> dst
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
    // do nothing if src and dst are the same
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

CycleGroup*
BoundPropagator::newCycleGroup(CycleGroup* cg)
{
    if (cg == nullptr)
    {
        cg = new CycleGroup(_mgr);
    }
    cg->setId(_initCgId++);
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
        auto bound = _propQ.front();
        _propQ.pop_front();
        _boundInProcess = bound;
        bound->propagate();
        bound->setQueued(false);
    }
    _boundInProcess = nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
BoundPropagator::clearPropQ()
{
    if (_boundInProcess != nullptr)
    {
        _boundInProcess->setQueued(false);
        _boundInProcess = nullptr;
    }
    for (auto bound : _propQ)
    {
        bound->setQueued(false);
    }
    _propQ.clear();
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
    _propQ.push_back(bound);
}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////

void
BoundPropagator::unsuspendInitial()
{
    // initialCGS (cycle groups with no unfinalized predecessors)
    cg_set_id_t initialCGs;
    for (auto cg : _cgs)
    {
        if (!cg->suspended())
        {
            initialCGs.insert(cg);
        }
    }

    // unsuspend member bounds for initialCGs
    for (auto initialCG : initialCGs)
    {
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
    // all bounds in this cb's cycle-group are finalized?
    auto cg = cb->cycleGroup();
    if (cg->finalized())
    {
        // do nothing
        return;
    }
    // 
    cb->queueFind();
    cg->finalizeMember();
}

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
        auto restartCG = _restartCG;
        _restartCG = nullptr;
        dfs(restartCG);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
BoundPropagator::dfs(CycleGroup* cg)
{
    // already visited cg -> make cycle group
    if (cg->visited())
    {
        ASSERTD(cg->visitedIdx() == 0);
        makeCycleGroup();
        return;
    }

    // cg doesn't lead toward a cycle -> ignore it
    auto firstCG = _cgArray[0];
    if (!cg->allSuccCGs().has(firstCG))
    {
        return;
    }

    // put cg (leading to cycle) onto the stack
    dfs_push(cg);

    // recursive invocation for cg's successors
    for (auto succCG : cg->succCGs())
    {
        dfs(succCG);
        // stop when restart is needed
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
    cg->setVisitedIdx(_cgArrayPtr - _cgArray);
    ASSERTD(cg->visited());

    // push cg onto stack
    *_cgArrayPtr++ = cg;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
BoundPropagator::dfs_pop()
{
    ASSERTD(_cgArrayPtr > _cgArray);

    // remove cg from stack
    auto cg = *(--_cgArrayPtr);

    // clear cg's visited flag
    cg->setVisitedIdx(uint_t_max);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
BoundPropagator::makeCycleGroup()
{
    ASSERTD((_cgArrayPtr - _cgArray) >= 2);

    // find largest cycle group
    auto motherCG = _cgArray[0];
    uint_t motherCGsize = motherCG->size();
    CycleGroup** cgIt;
    CycleGroup** cgLim = _cgArrayPtr;
    for (cgIt = (_cgArray + 1); cgIt != cgLim; ++cgIt)
    {
        CycleGroup* cg = *cgIt;
        if (cg->size() > motherCGsize)
        {
            motherCG = cg;
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

        // preCG is part of new cycle -> remove (predCG->motherCG) link
        if (predCG->visited())
        {
            motherCG->removePred(predCG);
        }
    }

    // for each successor of motherCG
    cg_revset_t::iterator succIt, succNext;
    cg_revset_t::iterator succLim = motherCG->allSuccCGs().end();
    for (succIt = motherCG->allSuccCGs().begin(); succIt != succLim; succIt = succNext)
    {
        succNext = succIt;
        ++succNext;

        // succCG is part of new cycle -> remove (motherCG->succCG) link
        auto succCG = *succIt;
        if (succCG->visited())
        {
            succCG->removePred(motherCG);
        }
    }

    // motherCG eclipses all others
    for (cgIt = _cgArray; cgIt != cgLim; ++cgIt)
    {
        // cg is motherCG -> skip it
        auto cg = *cgIt;
        if (cg == motherCG)
            continue;

        // clear cg's indirect lists
        cg->clearIndirectLists();
    }
    for (cgIt = _cgArray; cgIt != cgLim; ++cgIt)
    {
        // cg is motherCG -> skip it
        auto cg = *cgIt;
        if (cg == motherCG)
            continue;

        // motherCG gobbles up cg
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
    // all CGs have successor-depth = 0 initially
    for (auto cg : _cgs)
    {
        cg->setSuccessorCount(cg->succCGs().size());
        cg->setSuccessorDepth(0);
    }

    // make a list of terminal CGs (those with no successors)
    cg_set_id_t terminalCGs;
    for (auto cg : _cgs)
    {
        if (cg->successorCount() == 0)
        {
            terminalCGs.insert(cg);
        }
    }

    // set successor depth for terminal CGs
    for (auto terminalCG : terminalCGs)
    {
        setSuccessorDepth(terminalCG);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
BoundPropagator::setSuccessorDepth(CycleGroup* cg)
{
    uint_t sd = cg->successorDepth();
    for (auto predCG : cg->predCGs())
    {
        ASSERTD(predCG->successorCount() > 0);
        predCG->setSuccessorDepth(utl::max(predCG->successorDepth(), sd + 1));
        if (predCG->setSuccessorCount(predCG->successorCount() - 1) == 0)
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
    for (auto cg : _cgs)
    {
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
    for (auto predCG : cg->predCGs())
    {
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
    for (auto succCG : cg->succCGs())
    {
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
