#include "libcse.h"
#include <libutl/BufferedFDstream.h>
#include <clp/FailEx.h>
#include "PtMutate.h"
#include "ClevorDataSet.h"

//////////////////////////////////////////////////////////////////////////////

#ifdef DEBUG
#define DEBUG_UNIT
#endif

//////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;
GOP_NS_USE;
CLP_NS_USE;
CLS_NS_USE;

//////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(cse::PtMutate, gop::RevOperator);

//////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////

void
PtMutate::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(PtMutate));
    const PtMutate& arm = (const PtMutate&)rhs;
    RevOperator::copy(arm);

    _numPtChoices = arm._numPtChoices;
    _acts = arm._acts;
    _ptExps = arm._ptExps;

    _moveSchedule = arm._moveSchedule;
    _movePtIdx = arm._movePtIdx;
    _movePt = arm._movePt;
}

//////////////////////////////////////////////////////////////////////////////

void
PtMutate::initialize(const gop::DataSet* p_dataSet)
{
    RevOperator::initialize();

    ASSERTD(dynamic_cast<const ClevorDataSet*>(p_dataSet) != nullptr);
    const ClevorDataSet* dataSet = (const ClevorDataSet*)p_dataSet;

    setActPtExps(dataSet);
    uint_t numPtExps = _ptExps.size();
    ASSERTD(_acts.size() == _ptExps.size());
    for (uint_t i = 0; i < numPtExps; i++)
    {
        JobOp* op = (JobOp*)_acts[i]->owner();
        _numPtChoices += _ptExps[i]->size();
        addOperatorVar(i,1,2,op->job()->activeP());
    }
    setNumChoices(_numPtChoices);
}

//////////////////////////////////////////////////////////////////////////////

bool
PtMutate::execute(
    gop::Ind* ind,
    gop::IndBuilderContext* p_context,
    bool singleStep)
{
    ASSERTD(dynamic_cast<SchedulingContext*>(p_context) != nullptr);
    ASSERTD(dynamic_cast<StringInd<uint_t>*>(ind) != nullptr);
    SchedulingContext* context = (SchedulingContext*)p_context;
    Manager* mgr = context->manager();
    _moveSchedule = (StringInd<uint_t>*)ind;
    gop::String<uint_t>& string = _moveSchedule->string();

    // choose activity
    uint_t actIdx = getSelectedVarIdx();
#ifdef DEBUG_UNIT
    utl::cout
        << "                                                   "
        << "varSucRate:" << getSelectedVarP()
        << ", idx:" << actIdx;
#endif
    _movePtIdx = _stringBase + actIdx;
    _movePt = string[_movePtIdx];
    PtActivity* act = _acts[actIdx];

    // choose processing-time
    uint_vector_t* pts = _ptExps[actIdx];
    uint_t numPts = pts->size();
    ASSERTD(numPts > 1);

    uint_t ptIdx, pt;
    if (singleStep)
    {
        uint_t oldPtIdx = 0;
        for (uint_vector_t::iterator it = pts->begin();
             it != pts->end(); it++)
        {
            if (_movePt == *it) break;
            oldPtIdx++;
        }
        ASSERTD(oldPtIdx < numPts);
        if (oldPtIdx == 0)
        {
            ptIdx = oldPtIdx + 1;
        }
        else if (oldPtIdx == numPts - 1)
        {
            ptIdx = oldPtIdx - 1;
        }
        else
        {
            uint_t randomNum = _rng->evali(2);
            if (randomNum == 0)
            {
                ptIdx = oldPtIdx - 1;
            }
            else
            {
                ptIdx = oldPtIdx + 1;
            }
        }
        pt = (*pts)[ptIdx];
    }
    else
    {
        ptIdx = _rng->evali(pts->size() - 1);
        pt = (*pts)[ptIdx];
        if (pt >= _movePt) pt = (*pts)[++ptIdx];
    }
    string[_movePtIdx] = pt;

    ASSERTD(pt != _movePt);
#ifdef DEBUG_UNIT
    utl::cout << ", task:" << act->id()
              << ", oldPt:" << _movePt
              << ", newPt:" << pt
              << utl::endl;
#endif
    ASSERTD(act->possiblePts().has(pt));
    act->selectPt(pt);
    mgr->propagate();
    return true;
}

//////////////////////////////////////////////////////////////////////////////

void
PtMutate::accept()
{
    ASSERTD(_moveSchedule != nullptr);
    if (_moveSchedule->newString()) _moveSchedule->acceptNewString();
}

//////////////////////////////////////////////////////////////////////////////

void
PtMutate::undo()
{
    ASSERTD(_moveSchedule != nullptr);
    if (_moveSchedule->newString()) _moveSchedule->deleteNewString();
    gop::String<uint_t>& string = _moveSchedule->string();

    if (_movePtIdx != uint_t_max)
    {
        ASSERTD(_movePt != uint_t_max);
        string[_movePtIdx] = _movePt;
    }
}

//////////////////////////////////////////////////////////////////////////////

void
PtMutate::init()
{
    _numPtChoices = 0;
    _moveSchedule = nullptr;
    _movePtIdx = uint_t_max;
    _movePt = uint_t_max;
}

//////////////////////////////////////////////////////////////////////////////

void
PtMutate::deInit()
{
    deleteCont(_ptExps);
}

//////////////////////////////////////////////////////////////////////////////

void
PtMutate::setActPtExps(const ClevorDataSet* dataSet)
{
    // init _acts and _ptExps
    _acts.clear();
    deleteCont(_ptExps);
    const jobop_set_id_t& ops = dataSet->sops();
    jobop_set_id_t::const_iterator it;
    for (it = ops.begin(); it != ops.end(); ++it)
    {
        JobOp* jobOp = *it;
        if (jobOp->frozen()) continue;
        Activity* act = jobOp->activity();
        ASSERTD(act != nullptr);
        if (!act->isA(PtActivity)) continue;
        PtActivity* ptact = (PtActivity*)act;
        const IntExp* ptExp = ptact->possiblePts();
        if (!ptExp->isBound())
        {
            uint_vector_t* pts = new uint_vector_t();
            uint_t pt = ptExp->min();
            uint_t ptMax = ptExp->max();
            while (pt <= ptMax)
            {
#ifdef DEBUG_UNIT
//                 utl::cout
//                     << "act:" << act->id() << ",#pts:" << ptExp->size()
//                     << ",pt = " << pt << utl::endl;
#endif
                pts->push_back(pt);
                pt = ptExp->getNext(pt);
            }
            _acts.push_back(ptact);
            _ptExps.push_back(pts);
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

CSE_NS_END;
