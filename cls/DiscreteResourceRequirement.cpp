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
    ASSERTD(rhs.isA(DiscreteResourceRequirement));
    const DiscreteResourceRequirement& dr = (const DiscreteResourceRequirement&)rhs;
    int res = utl::compare(_act->id(), dr._act->id());
    if (res != 0)
        return res;
    res = _rcpsArray.compare(dr._rcpsArray);
    return res;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
DiscreteResourceRequirement::initialize(BrkActivity* act)
{
    // initialize all ResourceCapPts
    Hashtable::iterator it;
    for (it = _rcps.begin(); it != _rcps.end(); ++it)
    {
        ResourceCapPts* rcp = (ResourceCapPts*)*it;
        rcp->initialize((Activity*)act);
        rcp->drr() = this;
    }

    checkAllSelected();
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
        String& str = *new String();
        str = "act-" + Uint(_act->id()).toString() + ": ";
        str += "excluding selected resource: " + Uint(resId).toString();
        throw FailEx(str);
    }
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
    return (const ResourceCapPts*)_rcps.first();
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

    Manager* mgr = manager();
    uint_set_t allResIds;
    getAllResIds(allResIds);

    _numRequired = new IntVar(mgr, new IntExpDomainAR(mgr, numRequiredDomain));

    _possibleResources = new IntVar(mgr, new IntExpDomainAR(mgr, allResIds));
    _possibleResources->failOnEmpty() = false;

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
        ResourceCapPts* resCapPts = this->resCapPts(resId);
        DiscreteResource* res = (DiscreteResource*)resCapPts->resource();
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
        ResourceCapPts* resCapPts = this->resCapPts(resId);
        if (resCapPts->findPt(pt) == nullptr)
        {
            excludeResource(resId);
        }
    }

    // select the pt for all selected RCPs
    for (itPtr = it = _selectedResources->begin(); !it->atEnd(); it->next())
    {
        resId = **it;
        ResourceCapPts* resCapPts = this->resCapPts(resId);
        resCapPts->selectPt(pt);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
DiscreteResourceRequirement::checkAllSelected()
{
    if (_allSelected || !_numRequired->isBound())
    {
        return;
    }

    uint_t numRequired = _numRequired->getValue();
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
            String& str = *new String();
            str = "act-" + Uint(_act->id()).toString() + ": " + Uint(numRequired).toString() +
                  " additional resource(s) needed, but only " + Uint(numPossible).toString() +
                  " possible";
            throw FailEx(str);
        }

        // requirements are known
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
    ASSERTD(_selectedResources->size() == (uint_t)_numRequired->getValue());
    manager()->revToggle(_allSelected);
    _possibleResources->remove(_possibleResources->min(), _possibleResources->max());
    _act->decrementUnknownReqs();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool
DiscreteResourceRequirement::_selectResource(uint_t resId)
{
    if (!_possibleResources->remove(resId))
    {
        return false;
    }
    ASSERTD(resCapPts(resId) != nullptr);
    _selectedResources->add(resId);
    const IntVar& possiblePts = _act->possiblePts();
    ResourceCapPts* resCapPts = this->resCapPts(resId);
    resCapPts->select();
    if (possiblePts.isBound())
    {
        uint_t pt = possiblePts.getValue();
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
    uint_t pt = _act->possiblePts().getValue();
    bool forward = _act->forward();
    IntExpDomainIt* it;
    AutoPtr<IntExpDomainIt> itPtr;
    for (itPtr = it = _selectedResources->begin(); !it->atEnd(); it->next())
    {
        int resId = **it;
        ResourceCapPts* rcp = resCapPts(resId);
        const CapPt* capPt = rcp->findPt(pt);
        if (capPt == nullptr)
        {
            String& str = *new String();
            str = "act-" + Uint(_act->id()).toString() + ": " +
                  "could not find CapPt for pt = " + Uint(pt).toString();
            throw FailEx(str);
        }
        if (forward)
        {
            ESbound* esb = (ESbound&)_act->esBound();
            ESboundTimetable* ttBound = (ESboundTimetable*)capPt->object();
            ASSERTD(ttBound != nullptr);
            esb->add(ttBound);
            if (!esb->suspended())
            {
                ttBound->registerEvents(esb);
            }
        }
        else
        {
            LFbound* lfb = (LFbound&)_act->lfBound();
            LFboundTimetable* ttBound = (LFboundTimetable*)capPt->object();
            ASSERTD(ttBound != nullptr);
            lfb->add(ttBound);
            if (!lfb->suspended())
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
    forEachIt(Hashtable, _rcps, ResourceCapPts, rcp) forEachIt(ResourceCapPts, rcp, CapPt, capPt)
        uint_t pt = capPt.processingTime();
    pts.insert(pt);
    endForEach;
    endForEach;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
DiscreteResourceRequirement::getAllResIds(uint_set_t& resIds)
{
    forEachIt(Hashtable, _rcps, ResourceCapPts, rcp) resIds.insert(rcp.resourceId());
    endForEach;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_END;
