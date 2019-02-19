#include "libcse.h"
#include <libutl/BufferedFDstream.h>
#include <clp/FailEx.h>
#include "AltJobMutate.h"

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

UTL_CLASS_IMPL(cse::AltJobMutate);

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

void
AltJobMutate::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(AltJobMutate));
    const AltJobMutate& ppm = (const AltJobMutate&)rhs;
    RevOperator::copy(ppm);

    _numPlanChoices = ppm._numPlanChoices;
    _jobGroups = ppm._jobGroups;

    _moveSchedule = ppm._moveSchedule;
    _moveGroupIdx = ppm._moveGroupIdx;
    _moveJobIdx = ppm._moveJobIdx;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
AltJobMutate::initialize(const gop::DataSet* p_dataSet)
{
    RevOperator::initialize();

    ASSERTD(dynamic_cast<const ClevorDataSet*>(p_dataSet) != nullptr);
    const ClevorDataSet* dataSet = (const ClevorDataSet*)p_dataSet;

    setAltJobGroups(dataSet);
    uint_t numGroups = _jobGroups.size();
    for (uint_t i = 0; i < numGroups; i++)
    {
        _numPlanChoices += _jobGroups[i]->jobs().size();
        addOperatorVar(i, 1, 2);
    }
    setNumChoices(_numPlanChoices);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool
AltJobMutate::execute(gop::Ind* ind, gop::IndBuilderContext* p_context, bool singleStep)
{
    ASSERTD(dynamic_cast<SchedulingContext*>(p_context) != nullptr);
    ASSERTD(dynamic_cast<StringInd<uint_t>*>(ind) != nullptr);
    _moveSchedule = (StringInd<uint_t>*)ind;
    gop::String<uint_t>& string = _moveSchedule->string();

    // choose an item
    uint_t groupIdx = this->varIdx();
    _moveGroupIdx = _stringBase + groupIdx;
    _moveJobIdx = string[_moveGroupIdx];
    JobGroup* group = _jobGroups[groupIdx];

    // choose a job to activate
    uint_t numJobs = group->jobs().size();
    ASSERTD(numJobs > 1);

    uint_t jobIdx;
    if (singleStep)
    {
        uint_t oldJobIdx = _moveJobIdx;
        if (oldJobIdx == 0)
        {
            jobIdx = oldJobIdx + 1;
        }
        else if (oldJobIdx == numJobs - 1)
        {
            jobIdx = oldJobIdx - 1;
        }
        else
        {
            uint_t randomNum = _rng->uniform(0, 1);
            if (randomNum == 0)
            {
                jobIdx = oldJobIdx - 1;
            }
            else
            {
                jobIdx = oldJobIdx + 1;
            }
        }
    }
    else
    {
        jobIdx = _rng->uniform((uint_t)0, numJobs - 2);
        if (jobIdx >= _moveJobIdx)
            jobIdx++;
    }
    string[_moveGroupIdx] = jobIdx;

    ASSERTD(jobIdx != _moveJobIdx);
#ifdef DEBUG_UNIT
    utl::cout << "                                                   "
              << "group:" << group->id() << ", oldJobIdx:" << _moveJobIdx
              << ", newJobIdx:" << jobIdx << utl::endl;
#endif
    // set active job
    // propagation is also done by
    // changing job's sid.reset selected Job
    ASSERTD(group->jobs().size() > 1);
    group->setActiveJob(jobIdx);
    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
AltJobMutate::accept()
{
    ASSERTD(_moveSchedule != nullptr);
    if (_moveSchedule->newString())
        _moveSchedule->acceptNewString();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
AltJobMutate::undo()
{
    ASSERTD(_moveSchedule != nullptr);
    if (_moveSchedule->newString())
        _moveSchedule->deleteNewString();
    gop::String<uint_t>& string = _moveSchedule->string();

    if (_moveGroupIdx != uint_t_max)
    {
        ASSERTD(_moveJobIdx != uint_t_max);
        string[_moveGroupIdx] = _moveJobIdx;
        JobGroup* group = _jobGroups[_moveGroupIdx - _stringBase]; // groupIdx
        // specially for AltJobMutate
        // because partial propagation from processPlan->serialId
        // and SchedulingContext::clear() doesn't reset active jobs.
        group->setActiveJob(_moveJobIdx);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
AltJobMutate::init()
{
    _numPlanChoices = 0;
    _moveSchedule = nullptr;
    _moveGroupIdx = uint_t_max;
    _moveJobIdx = uint_t_max;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
AltJobMutate::deInit()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
AltJobMutate::setAltJobGroups(const ClevorDataSet* dataSet)
{
    ASSERT(dataSet != nullptr);
    jobgroup_set_id_t::const_iterator jobgroupIt;
    for (jobgroupIt = dataSet->jobGroups().begin(); jobgroupIt != dataSet->jobGroups().end();
         jobgroupIt++)
    {
        JobGroup* jobGroup = *jobgroupIt;
        _jobGroups.push_back(jobGroup);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;
