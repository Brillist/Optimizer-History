#include "libcse.h"
#include <libutl/BufferedFDstream.h>
#include <clp/FailEx.h>
#include "JobSeqMutate.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef DEBUG
// #define DEBUG_UNIT
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;
GOP_NS_USE;
CLP_NS_USE;
CLS_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(cse::JobSeqMutate);

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

void
JobSeqMutate::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(JobSeqMutate));
    const JobSeqMutate& jsmutate = (const JobSeqMutate&)rhs;
    RevOperator::copy(jsmutate);
    _jobs = jsmutate._jobs;
    _swapJobs = jsmutate._swapJobs;

    _moveSchedule = jsmutate._moveSchedule;
    _moveJobSid = jsmutate._moveJobSid;
    _moveJobs = jsmutate._moveJobs;
    _moveJobIdxs = jsmutate._moveJobIdxs;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
JobSeqMutate::initialize(const gop::DataSet* p_dataSet)
{
    RevOperator::initialize();
    ASSERTD(dynamic_cast<const ClevorDataSet*>(p_dataSet) != nullptr);
    const ClevorDataSet* dataSet = (const ClevorDataSet*)p_dataSet;
    setJobs(dataSet);

    uint_t numChoices = 0;
    ASSERTD(_jobs.size() == _swapJobs.size());
    for (uint_t i = 0; i < _swapJobs.size(); i++)
    {
        Job* job = _jobs[i];
        uint_t numSwapJobs = numActiveSwapJobs(*_swapJobs[i]);
        if (numSwapJobs == 0)
        {
            addOperatorVar(i, 0, 2, job->activeP());
        }
        else
        {
            addOperatorVar(i, 1, 2, job->activeP());
        }
        if (job->active())
        {
            numChoices += numSwapJobs;
        }
    }
    ASSERTD(numChoices % 2 == 0);
    setNumChoices(numChoices / 2);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool
JobSeqMutate::execute(gop::Ind* ind, gop::IndBuilderContext* context, bool singleStep)
{
    ASSERTD(dynamic_cast<StringInd<uint_t>*>(ind) != nullptr);
    _moveSchedule = (StringInd<uint_t>*)ind;
    gop::String<uint_t>& string = _moveSchedule->string();
    //     ASSERTD(string[_stringBase] != uint_t_max);

    //init all job sids for failed initOpRun and multiple strings
    uint_t numJobs = _jobs.size();
    uint_t i;
    for (i = 0; i < numJobs; i++)
    {
        Job* job1 = _jobs[i];
        job1->serialId() = string[_stringBase + i];
    }

    //select a job - (jobIdx)
    uint_t jobIdx = getSelectedVarIdx();
#ifdef DEBUG_UNIT
    utl::cout << "          varSucRate:" << getSelectedVarP() << ", idx:" << jobIdx;
#endif
    _moveJobIdx = jobIdx;
    Job* job = _jobs[jobIdx];

    //select a swap job - (swapJobIdx)
    job_vector_t* swapJobs = _swapJobs[jobIdx];
    Job* swapJob = selectActiveSwapJob(*swapJobs);
    if (swapJob == nullptr)
        swapJob = job; // basically it will do nothing
                       //     uint_t numSwapJobs = numActiveSwapJobs(swapJobs);
                       //     uint_t idx = _rng->evali(swapJobs->size());
                       //     Job* swapJob = (*swapJobs)[idx];

        //swap two jobs
#ifdef DEBUG_UNIT
    uint_t jobId = job->id();
    uint_t swapJobId = swapJob->id();
#endif
    uint_t jobSid = job->serialId();
    uint_t swapJobSid = swapJob->serialId();

    ////collect and sort all jobs between job and swapJob
    uint_t minJobSid = utl::min(jobSid, swapJobSid);
    uint_t maxJobSid = utl::max(jobSid, swapJobSid);
    _moveJobSid = minJobSid;
    _moveJobs.clear();
    _moveJobIdxs.clear();
    for (uint_t i = 0; i < numJobs; i++)
    {
        Job* job = _jobs[i];
        uint_t sid = job->serialId();
        if (sid >= minJobSid && sid <= maxJobSid)
        {
            _moveJobs.push_back(job);
            _moveJobIdxs.push_back(i);
        }
    }
    std::sort(_moveJobs.begin(), _moveJobs.end(), JobSerialIdOrdering());
    job_vector_t changedJobs = _moveJobs;
    ////move firstOp and all it's sucessors to the end
    Job* firstJob = *(changedJobs.begin());
    const cg_revset_t& allSuccCGs = firstJob->cycleGroup()->allSuccCGs();
    Job* nextJob = firstJob;
    job_vector_t::iterator changedJobsEnd = changedJobs.end();
    Job* lastJob = *(--changedJobsEnd);
    job_vector_t::iterator startIt = changedJobs.begin();
    while (nextJob != lastJob)
    {
        if (nextJob == firstJob || allSuccCGs.find(nextJob->cycleGroup()) != allSuccCGs.end())
        {
            changedJobs.erase(startIt);
            changedJobs.push_back(nextJob);
        }
        else
        {
            startIt++;
        }
        nextJob = *startIt;
    }
    ////re-assign sid
    uint_t newSid = minJobSid;
    for (job_vector_t::iterator it = changedJobs.begin(); it != changedJobs.end(); it++)
    {
        Job* job1 = *it;
        job1->serialId() = newSid++;
        //         utl::cout << "newSeqOp:" << op3->getId()
        //                   << ", sid:" << op3->getSerialId() << std::endl;
    }
    ////update string
    for (uint_vector_t::iterator it = _moveJobIdxs.begin(); it != _moveJobIdxs.end(); it++)
    {
        uint_t idx = *it;
        Job* job1 = _jobs[idx];
        string[_stringBase + idx] = job1->serialId();
    }
    //disable the following two lines because job can equal to swapJob
//     ASSERTD((job->serialId() - swapJob->serialId() == 1) ||
//             (swapJob->serialId() - job->serialId() == 1));
#ifdef DEBUG_UNIT
    utl::cout << "                                                   "
              << ", job1:" << jobId << "(" << jobSid << "->" << job->serialId()
              << "), job2:" << swapJobId << "(" << swapJobSid << "->" << swapJob->serialId() << ")"
              << utl::endl;
#endif
    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
JobSeqMutate::setJobs(const ClevorDataSet* dataSet)
{
    //init _jobs and _swapJobs
    const job_set_id_t& jobs = dataSet->jobs();
    job_set_id_t::const_iterator jobIt;

    // make list of job-CGs
    cg_set_id_t jobCGs;
    for (jobIt = jobs.begin(); jobIt != jobs.end(); jobIt++)
    {
        Job* job = *jobIt;
        jobCGs.insert(job->cycleGroup());
    }

    for (jobIt = jobs.begin(); jobIt != jobs.end(); jobIt++)
    {
        Job* job = *jobIt;
        CycleGroup* cg = job->cycleGroup();
        _jobs.push_back(job);
        const cg_revset_t& allPredCGs = cg->allPredCGs();
        const cg_revset_t& allSuccCGs = cg->allSuccCGs();
        cg_set_id_t tempCandidates, cgCandidates;
        std::set_difference(jobCGs.begin(), jobCGs.end(), allPredCGs.begin(), allPredCGs.end(),
                            std::inserter(tempCandidates, tempCandidates.begin()),
                            CycleGroupIdOrdering());
        std::set_difference(tempCandidates.begin(), tempCandidates.end(), allSuccCGs.begin(),
                            allSuccCGs.end(), std::inserter(cgCandidates, cgCandidates.begin()),
                            CycleGroupIdOrdering());
        job_vector_t* swapjobs = new job_vector_t();
        cg_set_id_t::iterator cgIt;
        for (cgIt = cgCandidates.begin(); cgIt != cgCandidates.end(); ++cgIt)
        {
            CycleGroup* candCG = *cgIt;
            CycleGroup::iterator cbIt, cbEnd = candCG->end();
            for (cbIt = candCG->begin(); cbIt != cbEnd; ++cbIt)
            {
                ConstrainedBound* cb = *cbIt;
                if (!cb->owner()->isA(Job))
                    continue;
                Job* candJob = (Job*)cb->owner();
                if (job != candJob)
                {
                    swapjobs->push_back(candJob);
                }
            }
        }
        std::sort(swapjobs->begin(), swapjobs->end(), JobIdOrdering());
        _swapJobs.push_back(swapjobs);

        //check code. DO NOT DELETE!
        //         std::cout << "job:" << job->id() << ", #jobs:" << jobs.size()
        //                   << ", #predCGs:" << allPredCGs.size()
        //                   << ", #succCGss:" << allSuccCGs.size()
        //                   << ", #candidateCGs:" << cgCandidates.size()
        //                   << ", #swapJobs:" << swapjobs->size() << std::endl;
        //         std::cout << "all jobs:" << std::endl;
        //         for (job_set_id_t::iterator it = jobs.begin();
        //              it != jobs.end(); it++)
        //             std::cout << (*it)->id() << ", ";
        //         std::cout << std::endl << std::endl;
        //         std::cout << "all CGs:" << std::endl;
        //         for (cg_set_id_t::iterator it = jobCGs.begin();
        //              it != jobCGs.end(); it++)
        //             std::cout << (*it)->id() << ", ";
        //         std::cout << std::endl << std::endl;
        //         std::cout << "all predCGs:" << std::endl;
        //         for (cg_revset_t::iterator it = allPredCGs.begin();
        //              it != allPredCGs.end(); it++)
        //             std::cout << (*it)->id() << ", ";
        //         std::cout << std::endl << std::endl;
        //         std::cout << "all succCGs:" << std::endl;
        //         for (cg_revset_t::iterator it = allSuccCGs.begin();
        //              it != allSuccCGs.end(); it++)
        //             std::cout << (*it)->id() << ", ";
        //         std::cout << std::endl << std::endl;
        //         std::cout << "all tempjobs:" << std::endl;
        //         for (cg_set_id_t::const_iterator it = tempCandidates.begin();
        //              it != tempCandidates.end(); it++)
        //             std::cout << (*it)->id() << ", ";
        //         std::cout << std::endl << std::endl;
        //         std::cout << "all candidateCGs:" << std::endl;
        //         for (cg_set_id_t::iterator it = cgCandidates.begin();
        //              it != cgCandidates.end(); it++)
        //             std::cout << (*it)->id() << ", ";
        //         std::cout << std::endl << std::endl;
        //         std::cout << "all swapjobs:" << std::endl;
        //         for (job_vector_t::iterator it = swapjobs->begin();
        //              it != swapjobs->end(); it++)
        //             std::cout << (*it)->id() << ", ";
        //         std::cout << std::endl << std::endl;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
JobSeqMutate::accept()
{
    ASSERTD(_moveSchedule != nullptr);
    if (_moveSchedule->newString())
        _moveSchedule->acceptNewString();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
JobSeqMutate::undo()
{
    ASSERTD(_moveSchedule != nullptr);
    ASSERTD(_moveJobs.size() != 0);
    ASSERTD(_moveJobIdxs.size() != 0);
    if (_moveSchedule->newString())
        _moveSchedule->deleteNewString();
    gop::String<uint_t>& string = _moveSchedule->string();

    uint_t startSid = _moveJobSid;
    for (uint_t i = 0; i < _moveJobs.size(); i++)
    {
        Job* job = _moveJobs[i];
        ASSERTD(job->serialId() != startSid);
        job->serialId() = startSid++;
    }
    for (uint_vector_t::iterator it = _moveJobIdxs.begin(); it != _moveJobIdxs.end(); it++)
    {
        uint_t idx = *it;
        Job* job = _jobs[idx];
        string[_stringBase + idx] = job->serialId();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

uint_t
JobSeqMutate::numActiveSwapJobs(const job_vector_t& swapJobs)
{
    uint_t n = 0;
    job_vector_t::const_iterator it;
    for (it = swapJobs.begin(); it != swapJobs.end(); it++)
    {
        Job* job = (*it);
        if (job->active())
            n++;
    }
    return n;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Job*
JobSeqMutate::selectActiveSwapJob(const job_vector_t& swapJobs)
{
    job_vector_t jobs;
    job_vector_t::const_iterator it;
    for (it = swapJobs.begin(); it != swapJobs.end(); it++)
    {
        Job* job = (*it);
        if (job->active())
            jobs.push_back(job);
    }
    if (jobs.size() == 0)
        return nullptr;
    uint_t idx = _rng->uniform((size_t)0, jobs.size() - 1);
    return jobs[idx];
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
JobSeqMutate::init()
{
    _moveJobSid = uint_t_max;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
JobSeqMutate::deInit()
{
    deleteCont(_swapJobs);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;
