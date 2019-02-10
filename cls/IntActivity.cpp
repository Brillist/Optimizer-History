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
        // _breakList is already nullptr (nothing to do)
    }
    else if (_compositeReqs.size() == 1)
    {
        CompositeResourceRequirement* crr = (CompositeResourceRequirement*)_compositeReqs.first();
        CompositeResource* cres = crr->resource();
        ASSERTD(cres != nullptr);
        _breakList = cres->timetable().addCapExp(crr->minCapacity());
    }
    else
    {
        CapExpMgr* capExpMgr = schedule()->capExpMgr();
        uint_vector_t resCaps;
        uint_set_t::iterator it;
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
            dres->maxReqCap() += 100;
            allResIds().insert(dres->id());
        }
    }

    // initialize ES-bound (or LF-bound)
    if (forward())
    {
        ((ESboundInt&)esBound()).initialize();
    }
    else
    {
        ABORT();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

IntExp*
IntActivity::breakList() const
{
    return _breakList;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool
IntActivity::selectResource(uint_t resId)
{
    // since current IntActivity allows only CompositeResReq
    // and the selection of unary resource in a composite resource
    // is decided by a heuristic, nothing is needed to be done here
    // for now until DiscreteResReq is allowed for IntActivity.
    // Joe, Sept 14, 2005
    return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
IntActivity::addAllocation(uint_t resId, uint_t min, uint_t max)
{
    // grow array if necessary
    if (resId >= _allocationsSize)
    {
        revarray_t* nullPtr = nullptr;
        utl::arrayGrow(_allocations, _allocationsSize, resId + 1, 16, &nullPtr);
    }

    // reference RevArray for resId
    revarray_t* allocs;
    if (_allocations[resId] == nullptr)
    {
        _allocations[resId] = new revarray_t(manager());
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
    Resource** resourcesArray = schedule()->resourcesArray();
    for (uint_t i = 0; i < _allocationsSize; ++i)
    {
        revarray_t* allocs = _allocations[i];
        if (allocs == nullptr)
            continue;
        Resource* res = resourcesArray[i];
        ASSERTD(res->isA(DiscreteResource));
        DiscreteResource* dres = (DiscreteResource*)res;

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

void
IntActivity::init()
{
    _processingTime = uint_t_max;
    _breakList = nullptr;
    _allocationsSize = 0;
    _allocations = nullptr;
    revarray_t* nullPtr = nullptr;
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
