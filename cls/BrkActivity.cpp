#include "libcls.h"
#include <clp/FailEx.h>
#include <clp/IntExpDomainAR.h>
#include "BrkActivity.h"
#include "CompositeResource.h"
#include "DiscreteResource.h"
#include "ResourceCalendarMgr.h"
#include "Schedule.h"
#include "SchedulableBound.h"
#include "TimetableBound.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;
CLP_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(cls::BrkActivity);

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

void
BrkActivity::add(DiscreteResourceRequirement* drr)
{
    _discreteReqs += drr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
BrkActivity::initRequirements()
{
    Manager* mgr = manager();
    uint_set_t possiblePtsDomain;
    uint_set_t selectedResourcesDomain;

    // frozen processing-time?
    if (_frozenPt != uint_t_max)
    {
        possiblePtsDomain.insert(_frozenPt);
    }

    // get processing-times and resource-ids from requirements
    for (auto rr_ : _discreteReqs)
    {
        auto rr = utl::cast<DiscreteResourceRequirement>(rr_);
        if (possiblePtsDomain.empty())
        {
            rr->getAllPts(possiblePtsDomain);
        }
        else
        {
            uint_set_t curPts, isect;
            rr->getAllPts(curPts);
            std::set_intersection(possiblePtsDomain.begin(), possiblePtsDomain.end(),
                                  curPts.begin(), curPts.end(),
                                  std::insert_iterator<uint_set_t>(isect, isect.begin()));
            possiblePtsDomain = isect;
        }
        rr->getAllResIds(this->allResIds());
        rr->getAllResIds(selectedResourcesDomain);
    }

    // no possible pts?
    if (possiblePtsDomain.empty())
    {
        throw FailEx("act-" + Uint(id()).toString() + ": no possible pts");
    }

    // make possible-pts var
    _possiblePts = new IntVar(mgr, new IntExpDomainAR(mgr, possiblePtsDomain));

    // make selected-resources var
    _selectedResources = new IntVar(mgr, new IntExpDomainAR(mgr, selectedResourcesDomain, true));

    // initialize requirements
    _numUnknownReqs = _discreteReqs.size();
    for (auto rr_ : _discreteReqs)
    {
        auto rr = utl::cast<DiscreteResourceRequirement>(rr_);
        rr->initialize(this);
    }

    // pt bound?
    if (_possiblePts->isBound())
    {
        selectPt(_possiblePts->value());
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool
BrkActivity::selectResource(uint_t resId)
{
    for (auto rr_ : _discreteReqs)
    {
        auto rr = utl::cast<DiscreteResourceRequirement>(rr_);
        if (rr->selectResource(resId))
            return true;
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

IntExp*
BrkActivity::breakList() const
{
    if (_calendar == nullptr)
        return nullptr;
    return _calendar->breakList();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
BrkActivity::selectPt(uint_t pt)
{
    _possiblePts->setValue(pt);

    // notify res-reqs of the pt selection
    for (auto rr_ : _discreteReqs)
    {
        auto rr = utl::cast<DiscreteResourceRequirement>(rr_);
        rr->selectPt(pt);
    }

    addTimetableBounds();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool
BrkActivity::forward() const
{
    if (esBound().isA(SchedulableBound))
        return true;
    ASSERTD(lfBound().isA(SchedulableBound));
    return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
BrkActivity::init()
{
    _calendar = nullptr;
    _selectedResources = nullptr;
    _numUnknownReqs = 0;
    _addedTimetableBounds = false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
BrkActivity::deInit()
{
    delete _selectedResources;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
BrkActivity::decrementUnknownReqs()
{
    ASSERTD(_numUnknownReqs > 0);
    manager()->revDecrement(_numUnknownReqs);
    addTimetableBounds();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
BrkActivity::selectResource(uint_t resId, DiscreteResourceRequirement* p_rr)
{
    bool ptsWasBound = _possiblePts->isBound();

    // mark the resource as selected
    _selectedResources->add(resId);

    // processing-time must be appropriate for the selected resource
    ResourceCapPts* resCapPts = p_rr->resCapPts(resId);
    ASSERTD(resCapPts != nullptr);
    const Array& capPtsArray = resCapPts->capPtsArray();
    uint_t numCapPts = capPtsArray.size();
    int last = int_t_min;
    for (uint_t i = 0; i < numCapPts; ++i)
    {
        CapPt* capPt = (CapPt*)capPtsArray[i];
        TimetableBound* ttb = (TimetableBound*)capPt->object();
        if ((ttb == nullptr) || !ttb->possible())
            continue;
        int pt = capPt->processingTime();
        if ((last + 1) < pt)
        {
            _possiblePts->remove(last + 1, pt - 1);
        }
        last = pt;
    }
    ASSERTD(last < int_t_max);
    _possiblePts->remove(last + 1, int_t_max);

    // resource is not possible for any other requirement
    for (auto rr_ : _discreteReqs)
    {
        auto rr = utl::cast<DiscreteResourceRequirement>(rr_);
        if (rr != p_rr)
        {
            rr->excludeResource(resId);
        }
    }

    // notify requirements if pt was bound
    if (!ptsWasBound && _possiblePts->isBound())
    {
        selectPt(_possiblePts->value());
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
BrkActivity::addTimetableBounds()
{
    // OK to proceed?
    if (_addedTimetableBounds || (_numUnknownReqs > 0) || (!_possiblePts->isBound()))
    {
        return;
    }

    // set up calendar
    ResourceCalendarMgr* calendarMgr = schedule()->calendarMgr();
    std::set<uint_t> selectedCalendarIds;
    for (auto rr_ : _discreteReqs)
    {
        auto rr = utl::cast<DiscreteResourceRequirement>(rr_);
        rr->getSelectedCalendarIds(selectedCalendarIds);
    }
    ResourceCalendarSpec calendarSpec(rc_allAvailable, selectedCalendarIds);
    Manager* mgr = manager();
    mgr->revSet(_calendar);
    _calendar = calendarMgr->add(calendarSpec);

    // add timetable bounds to ES or LF bound
    for (auto rr_ : _discreteReqs)
    {
        auto rr = utl::cast<DiscreteResourceRequirement>(rr_);
        rr->addTimetableBounds();
    }

    // remember that we did this
    manager()->revToggle(_addedTimetableBounds);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_END;
