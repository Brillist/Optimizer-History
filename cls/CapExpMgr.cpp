#include "libcls.h"
#include <clp/IntVar.h>
#include "CapExpMgr.h"
#include "CompositeResource.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;
CLP_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(cls::CapExp);
UTL_CLASS_IMPL(cls::CapExpMgr);

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// CapExp /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

int
CapExp::compare(const Object& rhs) const
{
    if (!rhs.isA(CapExp))
    {
        return Object::compare(rhs);
    }
    auto& capExp = utl::cast<CapExp>(rhs);
    int res = utl::compare(_resCaps.size(), capExp._resCaps.size());
    if (res != 0)
        return res;
    uint_t size = _resCaps.size();
    for (uint_t i = 0; i != size; ++i)
    {
        res = utl::compare(_resCaps[i], capExp._resCaps[i]);
        if (res != 0)
            return res;
    }
    return res;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
CapExp::initExp(const Schedule* schedule)
{
    ASSERTD(_capExp == nullptr);
    auto mgr = schedule->manager();
    _capExp = new IntVar(mgr);
    auto resourcesArray = schedule->resourcesArray();
    for (uint_t idx = 0; idx != _resCaps.size(); idx += 2)
    {
        uint_t resSid = _resCaps[idx];
        uint_t cap = _resCaps[idx + 1];
        auto res = resourcesArray[resSid];
        auto cres = utl::cast<CompositeResource>(res);
        auto& timetable = cres->timetable();
        auto resCapExp = timetable.addCapExp(cap);
        _capExp->intersect(resCapExp);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

IntExp*
CapExp::get(const Schedule* schedule) const
{
    ASSERTD(_capExp != nullptr);

    // first use: set up propagation
    if (_useCount == 0)
    {
        auto mgr = schedule->manager();
        mgr->revSet(_useCount);
        auto resourcesArray = schedule->resourcesArray();
        for (uint_t idx = 0; idx != _resCaps.size(); idx += 2)
        {
            uint_t resSid = _resCaps[idx];
            uint_t cap = _resCaps[idx + 1];
            auto res = resourcesArray[resSid];
            auto cres = utl::cast<CompositeResource>(res);
            auto& timetable = cres->timetable();
            auto resCapExp = timetable.addCapExp(cap);
            resCapExp->addIntersectExp(_capExp);
        }
    }
    ++_useCount;
    return _capExp;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
CapExp::deInit()
{
    delete _capExp;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// CapExpMgr //////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void
CapExpMgr::add(const uint_vector_t& resCaps)
{
    auto capExp = new CapExp(resCaps);

    // already have it?
    if (_capExps.has(*capExp))
    {
        delete capExp;
        return;
    }

    // add it
    capExp->initExp(_schedule);
    _capExps += capExp;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

IntExp*
CapExpMgr::find(const uint_vector_t& resCaps) const
{
    auto capExp = utl::cast<CapExp>(_capExps.find(CapExp(resCaps)));
    auto intExp = capExp->get(_schedule);
    return intExp;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_END;
