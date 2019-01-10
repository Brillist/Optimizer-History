#include "libcse.h"
#include <libutl/BufferedFDstream.h>
#include <clp/FailEx.h>
#include "ResCapMutate.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef DEBUG
#define DEBUG_UNIT
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;
GOP_NS_USE;
CLP_NS_USE;
CLS_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(cse::ResCapMutate, gop::RevOperator);

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ResCapMutate::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(ResCapMutate));
    const ResCapMutate& rcm = (const ResCapMutate&)rhs;
    RevOperator::copy(rcm);
    _resources = rcm._resources;
    _moveSchedule = rcm._moveSchedule;
    _moveResIdx = rcm._moveResIdx;
    _moveResCap = rcm._moveResCap;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ResCapMutate::initialize(const gop::DataSet* p_dataSet)
{
    ASSERTD(dynamic_cast<const ClevorDataSet*>(p_dataSet) != nullptr);
    const ClevorDataSet* dataSet = (const ClevorDataSet*)p_dataSet;

    RevOperator::initialize();
    setResources(dataSet);
    uint_t numCaps = 0;
    uint_t numRes = _resources.size();
    for (uint_t i = 0; i < numRes; i++)
    {
        DiscreteResource* res = _resources[i];
        uint_t size = ((_maxCaps[i] - _minCaps[i]) / res->stepCap()) + 1;
        if (size == 0)
        {
            addOperatorVar(i, 0, 2);
        }
        else
        {
            addOperatorVar(i, 1, 2);
        }
        numCaps += size;
    }
    setNumChoices(numCaps - numRes);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool
ResCapMutate::execute(gop::Ind* p_ind, gop::IndBuilderContext* p_context, bool singleStep)
{
    if (_resources.size() == 0)
    {
        return false;
    }

    ASSERTD(dynamic_cast<StringInd<uint_t>*>(p_ind) != nullptr);
    ASSERTD(dynamic_cast<SchedulingContext*>(p_context) != nullptr);
    StringInd<uint_t>* ind = (StringInd<uint_t>*)p_ind;
    gop::String<uint_t>& string = ind->string();
    _moveSchedule = ind;
#ifdef DEBUG_UNIT
//      utl::cout << string.toString() << utl::endl;
#endif

    // select a resource (resIdx)
    //     uint_t resIdx = selectOperatorVarIdx();
    uint_t resIdx = getSelectedVarIdx();
    _moveResIdx = resIdx;
    _moveResCap = string[_stringBase + _moveResIdx];
    cse::DiscreteResource* res = _resources[resIdx];

    // choose resource capacity
    uint_t step = res->stepCap();
    uint_t oldCap = string[_stringBase + _moveResIdx];
    uint_t cap;
    if (singleStep)
    {
        if ((oldCap + step) > _maxCaps[resIdx])
        {
            cap = oldCap - step;
        }
        else if ((oldCap < step) || (oldCap - step) < _minCaps[resIdx])
        {
            cap = oldCap + step;
        }
        else
        {
            uint_t randomNum = _rng->evali(2);
            if (randomNum == 0)
            {
                cap = oldCap - step;
            }
            else
            {
                cap = oldCap + step;
            }
        }
        ASSERTD(cap >= _minCaps[resIdx] && cap <= _maxCaps[resIdx]);
    }
    else
    {
        uint_t range = (_maxCaps[resIdx] - _minCaps[resIdx]) / step;
        cap = _minCaps[resIdx] + (_rng->evali(range) * step);
        if (cap >= oldCap)
            cap = utl::min(cap + step, _maxCaps[resIdx]);
    }
    string[_stringBase + _moveResIdx] = cap;
#ifdef DEBUG_UNIT
    utl::cout << "                                                   "
              << " res:" << res->id() << ", size:" << _maxCaps[resIdx] - _minCaps[resIdx]
              << ", oldCap:" << oldCap << ", cap:" << cap << utl::endlf;
#endif
    cls::DiscreteResource* clsRes = (cls::DiscreteResource*)res->clsResource();
    clsRes->selectCapacity(cap, res->maxCap());
    SchedulingContext* context = (SchedulingContext*)p_context;
    context->manager()->revSet(res->selectedCap(), cap);
    context->manager()->propagate();
    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ResCapMutate::accept()
{
    ASSERTD(_moveSchedule != nullptr);
    if (_moveSchedule->newString())
        _moveSchedule->acceptNewString();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ResCapMutate::undo()
{
    ASSERTD(_moveSchedule != nullptr);
    ASSERTD(_moveResIdx != uint_t_max);
    ASSERTD(_moveResCap != uint_t_max);
    if (_moveSchedule->newString())
        _moveSchedule->deleteNewString();
    gop::String<uint_t>& string = _moveSchedule->string();
    string[_stringBase + _moveResIdx] = _moveResCap;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ResCapMutate::init()
{
    _moveSchedule = nullptr;
    _moveResIdx = uint_t_max;
    _moveResCap = uint_t_max;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ResCapMutate::deInit()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ResCapMutate::setResources(const ClevorDataSet* dataSet)
{
    // note all resources with minCap < maxCap
    _resources.clear();
    res_set_id_t::const_iterator it;
    for (it = dataSet->resources().begin(); it != dataSet->resources().end(); ++it)
    {
        DiscreteResource* res = dynamic_cast<DiscreteResource*>(*it);
        if (res == nullptr)
            continue;
        cls::DiscreteResource* clsRes = (cls::DiscreteResource*)(res->clsResource());
        uint_t minReqCap = roundUp(clsRes->minReqCap(), (uint_t)100);
        uint_t maxReqCap = roundUp(clsRes->maxReqCap(), (uint_t)100);
        ASSERTD(res->maxCap() >= res->minCap());
        ASSERTD(res->maxCap() >= minReqCap);
        //let's prove: maxCap() >= maxCap >= minCap >= minCap()
        //it's obvious from following definition that
        //    maxCap >= minCap and minCap >= minCap()
        //before prove: maxCap() >= maxCap, first let's prove: maxCap() >= minCap
        //because:  maxCap() >= minCap()                 ...(1)
        //          maxCap() >= minReq()                 ...(2)
        //          minCap = max(minCap(),minReq())      ...(3)
        //from (1),(2)and(3) we have: maxCap() >= minCap ...(4)
        //now let's prove maxCap() >= maxCap
        //because: maxCap = max(minCap, min(maxCap(),maxReq())) and (4)
        //we have:
        //          IF minCap >= min(maxCap(),maxReq())
        //             maxCap = minCap <= maxCap()       ...(5)
        //          ELSE (i.e. minCap < min(maxCap(),maxReq())
        //             IF maxCap() >= maxReq()
        //                maxCap = maxReq() <= maxCap()  ...(6)
        //             ELSE (i.e. maxCap() < maxReq())
        //                maxCap = maxCap()              ...(7)
        //             ENDIF
        //          ENDIF
        //from (5),(6)and(7), we have maxCap() > maxCap
        //Joe, July-12-2005

        if ((res->maxCap() - res->minCap()) >= res->stepCap())
        {
            uint_t minCap = utl::max(res->minCap(), minReqCap);
            uint_t maxCap = utl::max(minCap, utl::min(res->maxCap(), maxReqCap));
            if (maxCap > minCap)
            {
                _resources.push_back(res);
                _minCaps.push_back(minCap);
                _maxCaps.push_back(maxCap);
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;
