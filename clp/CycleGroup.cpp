#include "libclp.h"
#include "BoundPropagator.h"
#include "BoundCt.h"
#include "FailEx.h"
#include "CycleGroup.h"
#include <libutl/MemStream.h>

//////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;

//////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(clp::CycleGroup, utl::Object);

//////////////////////////////////////////////////////////////////////////////

CLP_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////

bool
CycleGroupIdOrdering::operator()(
    const CycleGroup* lhs,
    const CycleGroup* rhs) const
{
    return (lhs->id() < rhs->id());
}

//////////////////////////////////////////////////////////////////////////////

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

//////////////////////////////////////////////////////////////////////////////

int
CycleGroup::compare(const Object& rhs) const
{
    if (!rhs.isA(CycleGroup))
    {
        return Object::compare(rhs);
    }
    const CycleGroup& cg = (const CycleGroup&)rhs;
    int res = utl::compare(_id, cg._id);
    return res;
}

//////////////////////////////////////////////////////////////////////////////

void
CycleGroup::add(ConstrainedBound* cb)
{
    // what cycle-group does cb belong to?
    CycleGroup* cbCG = cb->cycleGroup();

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

//////////////////////////////////////////////////////////////////////////////

void
CycleGroup::eclipse(CycleGroup* cg)
{
    // assume ownership of all the given CG's bounds
    cb_revset_t::iterator cbIt, cbLim = cg->_cbs.end();
    for (cbIt = cg->_cbs.begin(); cbIt != cbLim; ++cbIt)
    {
        ConstrainedBound* cb = *cbIt;
        add(cb);
    }

    // for each predecessor of cg
    cg_revset_t::iterator cgIt, cgNext, cgLim = cg->_predCGs.end();
    for (cgIt = cg->_predCGs.begin(); cgIt != cgLim; cgIt = cgNext)
    {
        cgNext = cgIt; ++cgNext;

        CycleGroup* predCG = *cgIt;

        // skip predCG if it's part of the new cycle
        if (predCG->visited()) continue;

        // remove the (predCG => cg) link
        cg->removePred(predCG);

        // add (predCG <=> this) link
        addPred(predCG);
    }

    // for each successor of cg
    cgLim = cg->_succCGs.end();
    for (cgIt = cg->_succCGs.begin(); cgIt != cgLim; cgIt = cgNext)
    {
        cgNext = cgIt; ++cgNext;

        CycleGroup* succCG = *cgIt;

        // skip succCG if it's part of the new cycle
        if (succCG->visitedIdx() != uint_t_max) continue;

        // remove the (succCG => cg) link
        succCG->removePred(cg);

        // add (predCG <=> this) link
        succCG->addPred(this);
    }
}

//////////////////////////////////////////////////////////////////////////////

bool
CycleGroup::addPred(CycleGroup* cg, bool updateIndirect)
{
    // new link is: (cg => self)
    bool cycleFound = false;

    // no need to add it?
//     if ((cg == this)
//         || (_allPredCGs.find(cg) != _allPredCGs.end()))
//     {
//         return cycleFound;
//     }
    if ((cg == this) || (_predCGs.find(cg) != _predCGs.end()))
        return cycleFound;
    if (_allPredCGs.find(cg) != _allPredCGs.end())
    {
        removePred(cg);
    }

    // save state and add the relationship
    saveState();
    ASSERTFNP(_predCGs.add(cg));
    ASSERTFNP(_predCGs.has(cg));
    ASSERTFNP(cg->_succCGs.add(this));
    ASSERTFNP(cg->_succCGs.has(this));
    if (!cg->finalized())
    {
        ++_numUnfinalizedPredCGs;
    }
    _allPredCGs.add(cg);
    cg->_allSuccCGs.add(this);

    // update indirect relationships?
    if (!updateIndirect)
    {
        return cycleFound;
    }

    // for each successor of self
    cg_revset_t::iterator succIt, succLim = _allSuccCGs.end();
    for (succIt = _allSuccCGs.begin(); succIt != succLim; ++succIt)
    {
        CycleGroup* succCG = *succIt;
        if (succCG == cg)
        {
            cycleFound = true;
            continue;
        }
        succCG->_allPredCGs.add(cg);
        cg->_allSuccCGs.add(succCG);
    }

    // for each predecessor of cg
    cg_revset_t::iterator predIt, predLim = cg->allPredCGs().end();
    for (predIt = cg->allPredCGs().begin(); predIt != predLim; ++predIt)
    {
        CycleGroup* predCG = *predIt;

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
        succLim = _allSuccCGs.end();
        for (succIt = _allSuccCGs.begin(); succIt != succLim; ++succIt)
        {
            CycleGroup* succCG = *succIt;
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

//////////////////////////////////////////////////////////////////////////////

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

//////////////////////////////////////////////////////////////////////////////

void
CycleGroup::clearIndirectLists()
{
    // indirect predecessors
    cg_revset_t::iterator it = _allPredCGs.begin();
    cg_revset_t::iterator lim = _allPredCGs.end();
    while (it != lim)
    {
        CycleGroup* predCG = *it;
        ++it;
        _allPredCGs.remove(predCG);
        predCG->_allSuccCGs.remove(this);
    }

    // indirect successors
    it = _allSuccCGs.begin();
    lim = _allSuccCGs.end();
    while (it != lim)
    {
        CycleGroup* succCG = *it;
        ++it;
        _allSuccCGs.remove(succCG);
        succCG->_allPredCGs.remove(this);
    }
}

//////////////////////////////////////////////////////////////////////////////

void
CycleGroup::initIndirectLists()
{
    _allPredCGs.clear();
    _allSuccCGs.clear();
    cg_revset_t::iterator it;
    for (it = _predCGs.begin(); it != _predCGs.end(); ++it)
    {
        CycleGroup* predCG = *it;
        _allPredCGs.add(predCG);
    }
    for (it = _succCGs.begin(); it != _succCGs.end(); ++it)
    {
        CycleGroup* succCG = *it;
        _allSuccCGs.add(succCG);
    }
}

//////////////////////////////////////////////////////////////////////////////

void
CycleGroup::unsuspend()
{
    // unsuspend all member bounds
    cb_revset_t::iterator it, itLim = _cbs.end();
    for (it = _cbs.begin(); it != itLim; ++it)
    {
        ConstrainedBound* cb = *it;
        _bp->unsuspend(cb);
    }
}

//////////////////////////////////////////////////////////////////////////////

void
CycleGroup::finalize()
{
    cg_revset_t::iterator it, endIt = _succCGs.end();
    for (it = _succCGs.begin(); it != endIt; ++it)
    {
        CycleGroup* cg = *it;
        cg->finalizePred();
    }
}

//////////////////////////////////////////////////////////////////////////////

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

//////////////////////////////////////////////////////////////////////////////

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

//////////////////////////////////////////////////////////////////////////////

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

//////////////////////////////////////////////////////////////////////////////

bool
CycleGroup::precedes(const CycleGroup* cg) const
{
    return (cg->_allPredCGs.find((CycleGroup*)this) != cg->_allPredCGs.end());
}

//////////////////////////////////////////////////////////////////////////////

bool
CycleGroup::succeeds(const CycleGroup* cg) const
{
    return (_allPredCGs.find((CycleGroup*)cg) != _allPredCGs.end());
}

//////////////////////////////////////////////////////////////////////////////

void
CycleGroup::_saveState()
{
    _mgr->revSet(&_stateDepth, 3);
    _stateDepth = _mgr->depth();
}

//////////////////////////////////////////////////////////////////////////////

utl::String
CycleGroup::toString() const
{
    utl::MemStream str;
    str << "CG:" << this->id()
        << "(";
    cb_revset_t::iterator cbIt, cbLim = _cbs.end();
    for (cbIt = _cbs.begin(); cbIt != cbLim; ++cbIt)
    {
        if (cbIt != _cbs.begin()) str << ", ";
        ConstrainedBound* cb = *cbIt;
        ASSERTD(cb->owner() != nullptr);
        str << cb->name();

    }
    str << ")" << '\0';
    return utl::String((char*)str.get());
}

//////////////////////////////////////////////////////////////////////////////

utl::String
CycleGroup::predCGsString() const
{
    utl::MemStream str;
    str << "PredCGs[";
    cg_revset_t::iterator cgIt, cgLim = _predCGs.end();
    for (cgIt = _predCGs.begin(); cgIt != cgLim; ++cgIt)
    {
        if (cgIt != _predCGs.begin()) str << ", ";
        CycleGroup* cg = *cgIt;
        str << cg->toString();
    }
    str << "]" << '\0';
    return utl::String((char*)str.get());
}

//////////////////////////////////////////////////////////////////////////////

utl::String
CycleGroup::allPredCGsString() const
{
    utl::MemStream str;
    str << "AllPredCGs[";
    cg_revset_t::iterator cgIt, cgLim = _allPredCGs.end();
    for (cgIt = _allPredCGs.begin(); cgIt != cgLim; ++cgIt)
    {
        if (cgIt != _allPredCGs.begin()) str << ", ";
        CycleGroup* cg = *cgIt;
        str << cg->toString();
    }
    str << "]" << '\0';
    return utl::String((char*)str.get());
}

//////////////////////////////////////////////////////////////////////////////

utl::String
CycleGroup::succCGsString() const
{
    utl::MemStream str;
    str << "SuccCGs[";
    cg_revset_t::iterator cgIt, cgLim = _succCGs.end();
    for (cgIt = _succCGs.begin(); cgIt != cgLim; ++cgIt)
    {
        if (cgIt != _succCGs.begin()) str << ", ";
        CycleGroup* cg = *cgIt;
        str << cg->toString();
    }
    str << "]" << '\0';
    return utl::String((char*)str.get());
}

//////////////////////////////////////////////////////////////////////////////

utl::String
CycleGroup::allSuccCGsString() const
{
    utl::MemStream str;
    str << "AllSuccCGs[";
    cg_revset_t::iterator cgIt, cgLim = _allSuccCGs.end();
    for (cgIt = _allSuccCGs.begin(); cgIt != cgLim; ++cgIt)
    {
        if (cgIt != _allSuccCGs.begin()) str << ", ";
        CycleGroup* cg = *cgIt;
        str << cg->toString();
    }
    str << "]" << '\0';
    return utl::String((char*)str.get());
}

//////////////////////////////////////////////////////////////////////////////

CLP_NS_END;
