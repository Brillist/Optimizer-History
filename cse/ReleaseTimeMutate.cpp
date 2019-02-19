#include "libcse.h"
#include <libutl/BufferedFDstream.h>
#include <libutl/Time.h>
#include "ReleaseTimeMutate.h"

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

UTL_CLASS_IMPL(cse::ReleaseTimeMutate);

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ReleaseTimeMutate::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(ReleaseTimeMutate));
    const ReleaseTimeMutate& rtm = (const ReleaseTimeMutate&)rhs;
    RevOperator::copy(rtm);

    _numRlsTimeChoices = rtm._numRlsTimeChoices;
    _acts = rtm._acts;
    _minRlsTimes = rtm._minRlsTimes;

    _moveSchedule = rtm._moveSchedule;
    _moveActIdx = rtm._moveActIdx;
    _moveActRlsTime = rtm._moveActRlsTime;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ReleaseTimeMutate::initialize(const gop::DataSet* p_dataSet)
{
    RevOperator::initialize();

    ASSERTD(dynamic_cast<const ClevorDataSet*>(p_dataSet) != nullptr);
    const ClevorDataSet* dataSet = (const ClevorDataSet*)p_dataSet;

    setActs(dataSet);
    // init _numRlsTimeChoices
    uint_t numActs = _acts.size();
    for (uint_t i = 0; i < numActs; i++)
    {
        Activity* act = _acts[i];
        JobOp* op = (JobOp*)(act->owner());
        Job* job = op->job();
        //// suppose there are only two values for each act
        if (job->active())
            _numRlsTimeChoices += 2;
        addOperatorVar(i, 1, 2, job->activeP());
    }
    setNumChoices(_numRlsTimeChoices);
    // init _changeStep: set it to a day
    _changeStep = dataSet->schedulerConfig()->durationToTimeSlot(24 * 3600);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool
ReleaseTimeMutate::execute(gop::Ind* ind, gop::IndBuilderContext* p_context, bool singleStep)
{
    ASSERTD(dynamic_cast<SchedulingContext*>(p_context) != nullptr);
    ASSERTD(dynamic_cast<StringInd<uint_t>*>(ind) != nullptr);
    SchedulingContext* context = (SchedulingContext*)p_context;
    _moveSchedule = (StringInd<uint_t>*)ind;
    gop::String<uint_t>& string = _moveSchedule->string();

    // choose an activity
    uint_t actIdx = this->varIdx();
#ifdef DEBUG_UNIT
    utl::cout << "                                                   "
              << "varSucRate:" << this->varP() << ", idx:" << actIdx;
#endif
    _moveActIdx = _stringBase + actIdx;
    _moveActRlsTime = string[_moveActIdx];
    Activity* act = _acts[actIdx];

    // change the activity's release time
    uint_t newRlsTime;
    uint_t oldRlsTime = string[_moveActIdx];
    if (oldRlsTime - _changeStep < _minRlsTimes[actIdx])
    {
        newRlsTime = oldRlsTime + _changeStep;
    }
    else
    {
        uint_t randomNum = _rng->uniform(0, 1);
        if (randomNum == 0)
        {
            newRlsTime = oldRlsTime + _changeStep;
        }
        else
        {
            newRlsTime = oldRlsTime - _changeStep;
        }
    }
    string[_moveActIdx] = newRlsTime;

#ifdef DEBUG_UNIT
    time_t minT, oldT, newT, existingT;
    const ClevorDataSet* dataSet = context->clevorDataSet();
    minT = dataSet->schedulerConfig()->timeSlotToTime(_minRlsTimes[actIdx]);
    oldT = dataSet->schedulerConfig()->timeSlotToTime(oldRlsTime);
    newT = dataSet->schedulerConfig()->timeSlotToTime(newRlsTime);
    existingT = dataSet->schedulerConfig()->timeSlotToTime(act->es());
    utl::cout << ", job(ropId):" << act->id() << ", minReleaseT:" << Time(minT).toString() << "("
              << _minRlsTimes[actIdx] << "), oldReleaseT:" << Time(oldT).toString() << "("
              << oldRlsTime << "), newReleaseT:" << Time(newT).toString() << "(" << newRlsTime
              << "), existingES:" << Time(existingT).toString() << "(" << act->es() << ")"
              << utl::endlf;
#endif
    // set new release time and propagate
    if (act->es() < (int)newRlsTime)
    {
        act->start().setLB(newRlsTime);
        context->manager()->propagate();
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ReleaseTimeMutate::accept()
{
    ASSERTD(_moveSchedule != nullptr);
    if (_moveSchedule->newString())
        _moveSchedule->acceptNewString();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ReleaseTimeMutate::undo()
{
    ASSERTD(_moveSchedule != nullptr);
    ASSERTD(_moveActIdx != uint_t_max);
    ASSERTD(_moveActRlsTime != uint_t_max);
    if (_moveSchedule->newString())
        _moveSchedule->deleteNewString();
    gop::String<uint_t>& string = _moveSchedule->string();
    string[_moveActIdx] = _moveActRlsTime;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ReleaseTimeMutate::init()
{
    _changeStep = uint_t_max;
    _moveSchedule = nullptr;
    _moveActIdx = uint_t_max;
    _moveActRlsTime = uint_t_max;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ReleaseTimeMutate::deInit()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ReleaseTimeMutate::setActs(const ClevorDataSet* dataSet)
{
    //init _acts and _minRlsTimes
    const job_set_id_t& jobs = dataSet->jobs();
    job_set_id_t::iterator jobIt;
    for (jobIt = jobs.begin(); jobIt != jobs.end(); jobIt++)
    {
        Job* job = (*jobIt);
        SummaryOp* op = job->rootSummaryOp();
        ASSERT(op != nullptr);
#ifdef DEBUG_UNIT
        utl::cout << op->toString() << utl::endlf;
#endif
        const unaryct_vect_t& unaryCts = op->unaryCts();
        if (unaryCts.size() == 0)
            continue;
        unaryct_vect_t::const_iterator it;
        bool hasRlsTime = false;
        uint_t rlsTime = uint_t_max;
        for (it = unaryCts.begin(); it != unaryCts.end(); it++)
        {
            UnaryCt* uct = *it;
            if (uct->type() == uct_startNoSoonerThan)
            {
                hasRlsTime = true;
                rlsTime = dataSet->schedulerConfig()->timeToTimeSlot(uct->time());
#ifdef DEBUG_UNIT
                utl::cout << "summaryOp:" << op->id()
                          << " has a release date:" << Time(uct->time()).toString() << utl::endlf;
#endif
                break;
            }
        }
        if (!hasRlsTime)
            continue;
        //         ASSERTD(job->releaseTime() != -1);
        Activity* act = op->activity();
#ifdef DEBUG_UNIT
        utl::cout << act->toString() << ", rlsTime:" << rlsTime << ", act->es():" << act->es()
                  << utl::endlf;
#endif
        _acts.push_back(act);                                    // _acts
        _minRlsTimes.push_back(min(rlsTime, (uint_t)act->es())); // _minRlsTimes
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;
