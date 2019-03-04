#include "libclp.h"
#include "BoundPropagator.h"
#include "BoundCt.h"
#include "FailEx.h"
#include "CycleGroup.h"
#include <libutl/MemStream.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(clp::CycleGroup);

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////
// CycleGroupIdOrdering ////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

bool
CycleGroupIdOrdering::operator()(const CycleGroup* lhs, const CycleGroup* rhs) const
{
    return (lhs->id() < rhs->id());
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// CycleGroup //////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

CycleGroup::CycleGroup(Manager* mgr)
{
    ASSERTD(mgr != nullptr);
    _mgr = mgr;
    _bp = _mgr->boundPropagator();
    _id = uint_t_max;
    _visitedIdx = uint_t_max;
    _cbs.initialize(_mgr, rsd_add);
    _predCGs.initialize(_mgr, rsd_both);
    _succCGs.initialize(_mgr, rsd_both);
    _allPredCGs.initialize(_mgr, rsd_both);
    _allSuccCGs.initialize(_mgr, rsd_both);
    _stateDepth = _mgr->depth();
    _numUnfinalizedCBs = 0;
    _numUnfinalizedPredCGs = 0;
    _successorCount = 0;
    _successorDepth = 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int
CycleGroup::compare(const Object& rhs) const
{
    if (!rhs.isA(CycleGroup))
    {
        return Object::compare(rhs);
    }
    auto& cg = utl::cast<CycleGroup>(rhs);
    int res = utl::compare(_id, cg._id);
    return res;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

String
CycleGroup::toString() const
{
    utl::MemStream str;
    str << "CG:" << this->id() << "(";
    for (auto cb : _cbs)
    {
        if (cb != *_cbs.begin())
            str << ", ";
        ASSERTD(cb->owner() != nullptr);
        str << cb->name();
    }
    str << ")" << '\0';
    return String(str.takeString(), true, false);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
CycleGroup::add(ConstrainedBound* cb)
{
    // what cycle-group does cb belong to?
    auto cbCG = cb->cycleGroup();

    // cb already belongs to self => do nothing else
    if (cbCG == this)
    {
        ASSERTD(_cbs.find(*cb) != _cbs.end());
        return;
    }

    // save state and add cb to member list
    saveState();
    ASSERTFNP(_cbs.add(cb));
    cb->setCycleGroup(this);
    ++_numUnfinalizedCBs;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
CycleGroup::eclipse(CycleGroup* cg)
{
    // assume ownership of all the given CG's bounds
    for (auto cb : cg->_cbs)
    {
        add(cb);
    }

    // cg's predecessors become ours
    cg_revset_t::iterator cgIt, cgNext, cgLim = cg->_predCGs.end();
    for (cgIt = cg->_predCGs.begin(); cgIt != cgLim; cgIt = cgNext)
    {
        cgNext = cgIt;
        ++cgNext;

        auto predCG = *cgIt;

        // skip predCG if it's part of the new cycle
        if (predCG->visited())
            continue;

        // remove the (predCG => cg) link
        cg->removePred(predCG);

        // add (predCG <=> this) link
        addPred(predCG);
    }

    // cg's successors become ours
    cgLim = cg->_succCGs.end();
    for (cgIt = cg->_succCGs.begin(); cgIt != cgLim; cgIt = cgNext)
    {
        cgNext = cgIt;
        ++cgNext;

        auto succCG = *cgIt;

        // skip succCG if it's part of the new cycle
        if (succCG->visited())
            continue;

        // remove the (succCG => cg) link
        succCG->removePred(cg);

        // add (predCG <=> this) link
        succCG->addPred(this);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

cg_precedence_rel_t
CycleGroup::relationship(const CycleGroup* cg) const
{
    if (precedes(cg))
    {
        return pr_precedes;
    }
    else if (succeeds(cg))
    {
        return pr_succeeds;
    }
    else
    {
        return pr_none;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool
CycleGroup::precedes(const CycleGroup* cg) const
{
    return (cg->_allPredCGs.find(const_cast<CycleGroup*>(this)) != cg->_allPredCGs.end());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool
CycleGroup::succeeds(const CycleGroup* cg) const
{
    return (_allPredCGs.find(const_cast<CycleGroup*>(cg)) != _allPredCGs.end());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool
CycleGroup::addPred(CycleGroup* cg, bool updateIndirect)
{
    // new link is: (cg => self)
    bool cycleFound = false;

    // no need to add it?
    if ((cg == this) || (_predCGs.find(cg) != _predCGs.end()))
    {
        return cycleFound;
    }

    // cg is already in all-predecessors?
    if (_allPredCGs.find(cg) != _allPredCGs.end())
    {
        // unlink cg as a predecessor
        removePred(cg);
    }

    // save state and add the relationship
    saveState();
    ASSERTFNP(_predCGs.add(cg));
    ASSERTD(_predCGs.has(cg));
    ASSERTFNP(cg->_succCGs.add(this));
    ASSERTD(cg->_succCGs.has(this));
    if (!cg->finalized())
    {
        ++_numUnfinalizedPredCGs;
    }
    _allPredCGs.add(cg);
    cg->_allSuccCGs.add(this);

    // don't update indirect relationships?
    if (!updateIndirect)
    {
        return cycleFound;
    }

    // update indirect relationships
    //   NOTE: 
    //     1. when a cycle is detected we record that
    //     2. we don't create additional cyclical indirect links here

    // cg <--indirect--> self.indirectSuccessors
    for (auto succCG : _allSuccCGs)
    {
        if (succCG == cg)
        {
            cycleFound = true;
            continue;
        }
        succCG->_allPredCGs.add(cg);
        cg->_allSuccCGs.add(succCG);
    }

    // 1. cg.indirectPredecessors <--indirect--> self
    // 2. cg.indirectPredecessors <--indirect--> self.indirectSuccessors
    for (auto predCG : cg->_allPredCGs)
    {
        // predCG => self
        if (predCG == this)
        {
            cycleFound = true;
        }
        else
        {
            _allPredCGs.add(predCG);
            predCG->_allSuccCGs.add(this);
        }

        // for each successor of self
        for (auto succCG : _allSuccCGs)
        {
            if (predCG == succCG)
            {
                cycleFound = true;
                continue;
            }
            succCG->_allPredCGs.add(predCG);
            predCG->_allSuccCGs.add(succCG);
        }
    }
    return cycleFound;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
CycleGroup::removePred(CycleGroup* cg)
{
    // remove any indirect relationship
    saveState();
    _allPredCGs.remove(*cg);
    cg->_allSuccCGs.remove(*this);

    // cg not a direct predecessor of self => do nothing else
    if (!_predCGs.remove(*cg))
    {
        return;
    }
    ASSERTFNP(cg->_succCGs.remove(*this));

    // cg not finalized => one less unfinalized predecessor
    if (!cg->finalized())
    {
        --_numUnfinalizedPredCGs;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
CycleGroup::clearIndirectLists()
{
    // indirect predecessors
    auto it = _allPredCGs.begin();
    auto lim = _allPredCGs.end();
    while (it != lim)
    {
        auto predCG = *it;
        ++it;
        _allPredCGs.remove(predCG);
        predCG->_allSuccCGs.remove(this);
    }

    // indirect successors
    it = _allSuccCGs.begin();
    lim = _allSuccCGs.end();
    while (it != lim)
    {
        auto succCG = *it;
        ++it;
        _allSuccCGs.remove(succCG);
        succCG->_allPredCGs.remove(this);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
CycleGroup::initIndirectLists()
{
    _allPredCGs.clear();
    _allSuccCGs.clear();
    for (auto predCG : _allPredCGs)
    {
        _allPredCGs.add(predCG);
    }
    for (auto succCG : _allSuccCGs)
    {
        _allSuccCGs.add(succCG);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
CycleGroup::unsuspend()
{
    // unsuspend all member bounds
    for (auto cb : _cbs)
    {
        _bp->unsuspend(cb);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
CycleGroup::finalize()
{
    for (auto succCG : _succCGs)
    {
        succCG->finalizePred();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
CycleGroup::finalizeMember()
{
    ASSERTD(_numUnfinalizedCBs > 0);

    // all bounds finalized => inform successors that self is finalized
    saveState();
    if (--_numUnfinalizedCBs == 0)
    {
        finalize();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
CycleGroup::finalizePred()
{
    ASSERTD(_numUnfinalizedPredCGs > 0);

    // all predecessor CGs finalized => unsuspend members
    saveState();
    if (--_numUnfinalizedPredCGs == 0)
    {
        unsuspend();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
CycleGroup::_saveState()
{
    _mgr->revSet(&_stateDepth, 3);
    _stateDepth = _mgr->depth();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef DEBUG

////////////////////////////////////////////////////////////////////////////////////////////////////

String
CycleGroup::predCGsString() const
{
    utl::MemStream str;
    str << "PredCGs[";
    for (auto cg : _predCGs)
    {
        if (cg != *_predCGs.begin())
            str << ", ";
        str << cg->toString();
    }
    str << "]" << '\0';
    return String(str.takeString(), true, false);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

String
CycleGroup::allPredCGsString() const
{
    utl::MemStream str;
    str << "AllPredCGs[";
    for (auto cg : _allPredCGs)
    { 
        if (cg != *_allPredCGs.begin())
            str << ", ";
        str << cg->toString();
    }
    str << "]" << '\0';
    return String(str.takeString(), true, false);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

String
CycleGroup::succCGsString() const
{
    utl::MemStream str;
    str << "SuccCGs[";
    for (auto cg : _succCGs)
    {
        if (cg != *_succCGs.begin())
            str << ", ";
        str << cg->toString();
    }
    str << "]" << '\0';
    return String(str.takeString(), true, false);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

String
CycleGroup::allSuccCGsString() const
{
    utl::MemStream str;
    str << "AllSuccCGs[";
    for (auto cg : _allSuccCGs)
    {
        if (cg != *_allSuccCGs.begin())
            str << ", ";
        str << cg->toString();
    }
    str << "]" << '\0';
    return String(str.takeString(), true, false);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_END;
