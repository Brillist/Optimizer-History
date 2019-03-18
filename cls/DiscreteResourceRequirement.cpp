#include "libcls.h"
#include <libutl/AutoPtr.h>
#include <clp/FailEx.h>
#include <clp/IntExpDomainAR.h>
#include "BrkActivity.h"
#include "DiscreteResource.h"
#include "ESbound.h"
#include "ESboundTimetable.h"
#include "LFbound.h"
#include "LFboundTimetable.h"
#include "DiscreteResourceRequirement.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;
CLP_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(cls::DiscreteResourceRequirement);

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

DiscreteResourceRequirement::DiscreteResourceRequirement(BrkActivity* act,
                                                         ResourceCapPts* resCapPts)
{
    utl::RBtree rcps(false);
    rcps += resCapPts;
    uint_set_t numRequiredDomain;
    numRequiredDomain.insert(1);
    init(act, rcps, numRequiredDomain);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

DiscreteResourceRequirement::DiscreteResourceRequirement(BrkActivity* act,
                                                         const Collection& rcps,
                                                         uint_t numRequired)
{
    uint_set_t numRequiredDomain;
    numRequiredDomain.insert(numRequired);
    init(act, rcps, numRequiredDomain);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

DiscreteResourceRequirement::DiscreteResourceRequirement(BrkActivity* act,
                                                         const Collection& rcps,
                                                         const uint_set_t& numRequiredDomain)
{
    init(act, rcps, numRequiredDomain);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int
DiscreteResourceRequirement::compare(const Object& rhs) const
{
    auto& drr = utl::cast<DiscreteResourceRequirement>(rhs);
    int res = utl::compare(_act->id(), drr._act->id());
    if (res != 0)
        return res;
    res = _rcpsArray.compare(drr._rcpsArray);
    return res;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
DiscreteResourceRequirement::initialize(BrkActivity* act)
{
    // initialize all ResourceCapPts
    for (auto rcp_ : _rcps)
    {
        auto rcp = utl::cast<ResourceCapPts>(rcp_);
        rcp->initialize(utl::cast<Activity>(act));
        rcp->setDRR(this);
    }

    checkAllSelected();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Manager*
DiscreteResourceRequirement::manager() const
{
    return _act->manager();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

const ResourceCapPts*
DiscreteResourceRequirement::resCapPts() const
{
    ASSERTD(_rcps.size() == 1);
    return utl::cast<ResourceCapPts>(_rcps.first());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
DiscreteResourceRequirement::selectNumRequired(uint_t numRequired)
{
    _numRequired->setValue(numRequired);
    checkAllSelected();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool
DiscreteResourceRequirement::selectResource(uint_t resId)
{
    if (!_selectResource(resId))
    {
        return false;
    }
    checkAllSelected();
    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
DiscreteResourceRequirement::excludeResource(uint_t resId)
{
    if (_possibleResources->remove(resId))
    {
        checkAllSelected();
    }
    else if (_selectedResources->has(resId))
    {
        throw FailEx("act-" + Uint(_act->id()).toString() + ": " +
                     "excluding selected resource: " + Uint(resId).toString());
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
DiscreteResourceRequirement::init(BrkActivity* act,
                                  const Collection& rcps,
                                  const uint_set_t& numRequiredDomain)
{
    _act = act;
    _rcps.setOwner(false);
    _rcps = rcps;
    _rcpsArray.setOwner(false);
    _rcpsArray = rcps;
    _rcpsArray.sort();
    _allSelected = false;

    auto mgr = manager();
    uint_set_t allResIds;
    getAllResIds(allResIds);

    _numRequired = new IntVar(mgr, new IntExpDomainAR(mgr, numRequiredDomain));
    _possibleResources = new IntVar(mgr, new IntExpDomainAR(mgr, allResIds));
    _possibleResources->setFailOnEmpty(false);
    _selectedResources = new IntVar(mgr, new IntExpDomainAR(mgr, allResIds, true));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
DiscreteResourceRequirement::deInit()
{
    delete _numRequired;
    delete _possibleResources;
    delete _selectedResources;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
DiscreteResourceRequirement::getSelectedCalendarIds(uint_set_t& calendarIds)
{
    IntExpDomainIt* it;
    AutoPtr<IntExpDomainIt> itPtr;
    for (itPtr = it = _selectedResources->begin(); !it->atEnd(); it->next())
    {
        int resId = **it;
        auto resCapPts = this->resCapPts(resId);
        auto res = utl::cast<DiscreteResource>(resCapPts->resource());
        ASSERTD(res != nullptr);
        ASSERTD(res->calendar() != nullptr);
        calendarIds.insert(res->calendar()->id());
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
DiscreteResourceRequirement::selectPt(uint_t pt)
{
    int resId;

    // remove resources for which the pt is not workable
    IntExpDomainIt* it;
    AutoPtr<IntExpDomainIt> itPtr;
    for (itPtr = it = _possibleResources->begin(); !it->atEnd() && !_allSelected; it->next())
    {
        resId = **it;
        auto resCapPts = this->resCapPts(resId);
        if (resCapPts->findPt(pt) == nullptr)
        {
            excludeResource(resId);
        }
    }

    // select the pt for all selected RCPs
    for (itPtr = it = _selectedResources->begin(); !it->atEnd(); it->next())
    {
        resId = **it;
        auto resCapPts = this->resCapPts(resId);
        resCapPts->selectPt(pt);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
DiscreteResourceRequirement::checkAllSelected()
{
    // we already did this or numRequired
    if (_allSelected || !_numRequired->isBound())
    {
        return;
    }

    uint_t numRequired = _numRequired->value();
    uint_t numSelected = _selectedResources->size();
    uint_t numPossible = _possibleResources->size();
    ASSERTD(numSelected <= numRequired);
    numRequired -= numSelected;

    // still require additional resource(s)?
    if (numRequired > 0)
    {
        // unable to select resources?
        if (numPossible > numRequired)
        {
            return;
        }

        // not possible to satisfy requirement?
        if (numPossible < numRequired)
        {
            throw FailEx("act-" + Uint(_act->id()).toString() + ": " +
                         Uint(numRequired).toString() +
                         " additional resource(s) needed, but only " +
                         Uint(numPossible).toString() + " possible");
        }

        // required resources are known
        ASSERTD(numPossible == numRequired);
        IntExpDomainIt* it;
        AutoPtr<IntExpDomainIt> itPtr;
        for (itPtr = it = _possibleResources->begin(); !it->atEnd(); it->next())
        {
            int resId = **it;
            _selectResource(resId);
        }
    }

    // all required resources known
    ASSERTD(_selectedResources->size() == (uint_t)_numRequired->value());
    manager()->revToggle(_allSelected);
    _possibleResources->remove(_possibleResources->min(), _possibleResources->max());
    _act->decrementUnknownReqs();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool
DiscreteResourceRequirement::_selectResource(uint_t resId)
{
    // try to remove resId from _possibleResources
    if (!_possibleResources->remove(resId))
    {
        // resId was already present -> do nothing
        return false;
    }

    // add resId to _selectedResources
    _selectedResources->add(resId);

    // notify resCapPts(resId) that it has been selected
    auto resCapPts = this->resCapPts(resId);
    ASSERTD(resCapPts != nullptr);
    resCapPts->select();

    // if the processing time is known, select it
    auto& possiblePts = _act->possiblePts();
    if (possiblePts.isBound())
    {
        uint_t pt = possiblePts.value();
        ASSERTD(resCapPts != nullptr);
        resCapPts->selectPt(pt);
    }
    _act->selectResource(resId, this);
    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
DiscreteResourceRequirement::addTimetableBounds()
{
    uint_t pt = _act->possiblePts().value();
    bool forward = _act->forward();
    IntExpDomainIt* it;
    AutoPtr<IntExpDomainIt> itPtr;
    for (itPtr = it = _selectedResources->begin(); !it->atEnd(); it->next())
    {
        int resId = **it;
        auto rcp = resCapPts(resId);
        auto capPt = rcp->findPt(pt);
        if (capPt == nullptr)
        {
            throw FailEx("act-" + Uint(_act->id()).toString() + ": " +
                         "could not find CapPt for pt = " + Uint(pt).toString());
        }
        if (forward)
        {
            auto& esb = utl::cast<ESbound>(_act->esBound());
            auto ttBound = utl::cast<ESboundTimetable>(capPt->object());
            ASSERTD(ttBound != nullptr);
            esb.add(ttBound);
            if (!esb.suspended())
            {
                ttBound->registerEvents(esb);
            }
        }
        else
        {
            auto& lfb = utl::cast<LFbound>(_act->lfBound());
            auto ttBound = utl::cast<LFboundTimetable>(capPt->object());
            ASSERTD(ttBound != nullptr);
            lfb.add(ttBound);
            if (!lfb.suspended())
            {
                ttBound->registerEvents(lfb);
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
DiscreteResourceRequirement::getAllPts(uint_set_t& pts)
{
    for (auto rcp_ : _rcps)
    {
        auto rcp = utl::cast<ResourceCapPts>(rcp_);
        for (auto capPt_ : *rcp)
        {
            auto capPt = utl::cast<CapPt>(capPt_);
            uint_t pt = capPt->processingTime();
            pts.insert(pt);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
DiscreteResourceRequirement::getAllResIds(uint_set_t& resIds)
{
    for (auto rcp_ : _rcps)
    {
        auto rcp = utl::cast<ResourceCapPts>(rcp_);
        resIds.insert(rcp->resourceId());
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_END;
