#include "libcls.h"
#include <clp/FailEx.h>
#include <clp/IntExpDomainAR.h>
#include "CapExpMgr.h"
#include "CompositeResource.h"
#include "CompositeResourceRequirement.h"
#include "ESboundInt.h"
#include "SchedulableBound.h"
#include "IntActivity.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;
CLP_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(cls::IntActivity);

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

bool
IntActivity::selectResource(uint_t resId)
{
    // no action required here until IntActivity can have a DiscreteResourceRequirement
    return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
IntActivity::add(CompositeResourceRequirement* crr)
{
    _compositeReqs += crr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
IntActivity::initRequirements()
{
    // set _breakList
    if (_compositeReqs.size() == 0)
    {
        // _breakList is already null (nothing to do)
    }
    else if (_compositeReqs.size() == 1)
    {
        auto crr = utl::cast<CompositeResourceRequirement>(_compositeReqs.first());
        auto cres = crr->resource();
        ASSERTD(cres != nullptr);
        _breakList = cres->timetable().addCapExp(crr->minCapacity());
    }
    else
    {
        auto capExpMgr = schedule()->capExpMgr();
        uint_vector_t resCaps;
        for (auto crr_ : _compositeReqs)
        {
            auto crr = utl::cast<CompositeResourceRequirement>(crr_);
            auto cres = crr->resource();
            uint_t cap = crr->minCapacity();
            resCaps.push_back(cres->serialId());
            resCaps.push_back(cap);
        }
        capExpMgr->add(resCaps);
        _breakList = capExpMgr->find(resCaps);
    }

    // update _allResIds, and update discrete resources' maxCap
    for (auto crr_ : _compositeReqs)
    {
        auto crr = utl::cast<CompositeResourceRequirement>(crr_);
        auto cres = crr->resource();
        for (auto dres : *cres)
        {
            dres->setMaxReqCap(dres->maxReqCap() + 100);
            allResIds().insert(dres->id());
        }
    }

    // initialize ES-bound (or LF-bound)
    if (forward())
    {
        utl::cast<ESboundInt>(esBound()).initialize();
    }
    else
    {
        ABORT();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
IntActivity::addAllocation(uint_t resId, uint_t min, uint_t max)
{
    // grow array if necessary
    if (resId >= _allocationsSize)
    {
        revarray_uint_t* nullPtr = nullptr;
        utl::arrayGrow(_allocations, _allocationsSize, resId + 1, 16, &nullPtr);
    }

    // reference RevArray for resId
    revarray_uint_t* allocs;
    if (_allocations[resId] == nullptr)
    {
        _allocations[resId] = new revarray_uint_t(manager());
    }
    allocs = _allocations[resId];

    // extend existing span or add new span
    uint_t size = allocs->size();
    if ((size > 0) && (min == (allocs->get(size - 1) + 1)))
    {
        allocs->set(size - 1, max);
    }
    else
    {
        allocs->add(min);
        allocs->add(max);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
IntActivity::deallocate()
{
    auto resourcesArray = schedule()->resourcesArray();
    for (uint_t i = 0; i != _allocationsSize; ++i)
    {
        auto allocs = _allocations[i];
        if (allocs == nullptr)
            continue;
        auto res = resourcesArray[i];
        auto dres = utl::cast<DiscreteResource>(res);

        // deallocate
        uint_t lim = allocs->size();
        for (uint_t j = 0; j < lim; j += 2)
        {
            int st = allocs->get(j);
            int et = allocs->get(j + 1);

            // deallocate capacity
            dres->deallocate(st, et, 100);
        }

        // finally, clear allocations
        allocs->clear();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool
IntActivity::forward() const
{
    if (esBound().isA(SchedulableBound))
        return true;
    ASSERTD(lfBound().isA(SchedulableBound));
    return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

IntExp*
IntActivity::breakList() const
{
    return _breakList;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
IntActivity::init()
{
    _processingTime = uint_t_max;
    _breakList = nullptr;
    _allocationsSize = 0;
    _allocations = nullptr;
    revarray_uint_t* nullPtr = nullptr;
    utl::arrayGrow(_allocations, _allocationsSize, 256, 256, &nullPtr);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
IntActivity::deInit()
{
    for (uint_t i = 0; i < _allocationsSize; ++i)
    {
        delete _allocations[i];
    }
    delete[] _allocations;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_END;
