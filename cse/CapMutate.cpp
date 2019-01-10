#include "libcse.h"
#include <libutl/BufferedFDstream.h>
#include <clp/FailEx.h>
#include <cls/ESboundInt.h>
#include "CapMutate.h"
#include "ClevorDataSet.h"

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

UTL_CLASS_IMPL(cse::CapMutate, gop::RevOperator);

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

void
CapMutate::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(CapMutate));
    const CapMutate& arm = (const CapMutate&)rhs;
    RevOperator::copy(arm);

    _numCapChoices = arm._numCapChoices;
    _acts = arm._acts;
    _minCapMultiples = arm._minCapMultiples;
    _maxCapMultiples = arm._maxCapMultiples;

    _moveSchedule = arm._moveSchedule;
    _moveCapIdx = arm._moveCapIdx;
    _moveCap = arm._moveCap;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
CapMutate::initialize(const gop::DataSet* p_dataSet)
{
    RevOperator::initialize();

    ASSERTD(dynamic_cast<const ClevorDataSet*>(p_dataSet) != nullptr);
    const ClevorDataSet* dataSet = (const ClevorDataSet*)p_dataSet;

    setActs(dataSet);
    uint_t numActs = _acts.size();
    for (uint_t i = 0; i < numActs; i++)
    {
        _numCapChoices += _maxCapMultiples[i] - _minCapMultiples[i];
        IntActivity* act = _acts[i];
        JobOp* op = (JobOp*)act->owner();
        addOperatorVar(i, 1, 2, op->job()->activeP());
    }
    setNumChoices(_numCapChoices);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool
CapMutate::execute(gop::Ind* ind, gop::IndBuilderContext* p_context, bool singleStep)
{
    ASSERTD(dynamic_cast<SchedulingContext*>(p_context) != nullptr);
    ASSERTD(dynamic_cast<StringInd<uint_t>*>(ind) != nullptr);
    _moveSchedule = (StringInd<uint_t>*)ind;
    gop::String<uint_t>& string = _moveSchedule->string();

    // choose activity
    uint_t actIdx = getSelectedVarIdx();
#ifdef DEBUG_UNIT
    utl::cout << "                                                   "
              << "varSucRate:" << getSelectedVarP() << ", idx:" << actIdx;
#endif
    _moveCapIdx = actIdx;
    _moveCap = string[_stringBase + _moveCapIdx];

    // choose activity cap
    uint_t oldCap = string[_stringBase + _moveCapIdx];
    uint_t cap;
    if (singleStep)
    {
        if (oldCap == _minCapMultiples[actIdx])
        {
            cap = oldCap + 1;
        }
        else if (oldCap == _maxCapMultiples[actIdx])
        {
            cap = oldCap - 1;
        }
        else
        {
            uint_t randomNum = _rng->evali(2);
            if (randomNum == 0)
            {
                cap = oldCap - 1;
            }
            else
            {
                cap = oldCap + 1;
            }
        }
        ASSERTD(cap >= _minCapMultiples[actIdx] && cap <= _maxCapMultiples[actIdx]);
    }
    else
    {
        cap = _rng->evali(_minCapMultiples[actIdx], _maxCapMultiples[actIdx]);
        if (cap >= oldCap)
            cap++;
    }
    string[_stringBase + _moveCapIdx] = cap;

    ASSERTD(cap != _moveCap);
#ifdef DEBUG_UNIT
    IntActivity* act = _acts[actIdx];
    utl::cout << ", task:" << act->id() << ", oldCap:" << _moveCap << ", newCap:" << cap
              << utl::endl;
#endif
    //no propagation is needed for CapMutate.
    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
CapMutate::accept()
{
    ASSERTD(_moveSchedule != nullptr);
    if (_moveSchedule->newString())
        _moveSchedule->acceptNewString();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
CapMutate::undo()
{
    ASSERTD(_moveSchedule != nullptr);
    if (_moveSchedule->newString())
        _moveSchedule->deleteNewString();
    gop::String<uint_t>& string = _moveSchedule->string();

    if (_moveCapIdx != uint_t_max)
    {
        ASSERTD(_moveCap != uint_t_max);
        string[_stringBase + _moveCapIdx] = _moveCap;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
CapMutate::init()
{
    _numCapChoices = 0;
    _moveSchedule = nullptr;
    _moveCapIdx = uint_t_max;
    _moveCap = uint_t_max;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
CapMutate::deInit()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
CapMutate::setActs(const ClevorDataSet* dataSet)
{
    // init _acts and _minCapMultiples and _maxCapMultiples
    _acts.clear();
    _minCapMultiples.clear();
    _maxCapMultiples.clear();
    const jobop_set_id_t& ops = dataSet->sops();
    jobop_set_id_t::const_iterator it;
    for (it = ops.begin(); it != ops.end(); ++it)
    {
        JobOp* jobOp = *it;
        if (jobOp->frozen() || !jobOp->interruptible())
            continue;
        IntActivity* act = jobOp->intact();
        uint_t minCapMultiple, maxCapMultiple;
        if (act->forward())
        {
            cls::ESboundInt& esb = (ESboundInt&)act->esBound();
            minCapMultiple = esb.minMultiple();
            maxCapMultiple = esb.maxMultiple();
        }
        else
        {
            ABORT();
        }
        ASSERTD(minCapMultiple <= maxCapMultiple);
        if (minCapMultiple == maxCapMultiple)
            continue;
        _acts.push_back(act);
        _minCapMultiples.push_back(minCapMultiple);
        _maxCapMultiples.push_back(maxCapMultiple);
#ifdef DEBUG_UNIT
//         utl::cout << "act:" << act->id()
//                   << ", minCapMultiple:" << minCapMultiple
//                   << ", maxCapMultiple:" << maxCapMultiple
//                   << utl::endl;
#endif
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;
