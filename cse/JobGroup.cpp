#include "libcse.h"
#include <libutl/BufferedFDstream.h>
#include <libutl/MemStream.h>
#include "JobGroup.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef DEBUG
#define DEBUG_UNIT
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
UTL_NS_USE;
LUT_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(cse::JobGroup);

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

bool
JobGroupIdOrdering::operator()(const JobGroup* lhs, const JobGroup* rhs) const
{
    return (lhs->id() < rhs->id());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
JobGroup::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(JobGroup));
    const JobGroup& jg = (const JobGroup&)rhs;
    _id = jg._id;
    _name = jg._name;
    _jobs = jg._jobs;
    _activeJob = jg._activeJob;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
JobGroup::serialize(Stream& stream, uint_t io, uint_t)
{
    utl::serialize(_id, stream, io);
    lut::serialize(_name, stream, io);
    uint_vect_t jobIds;
    uint_t activeJobId;
    if (io == io_rd)
    {
        // serialize in jobs and activeJob
        _jobs.clear();
        _activeJob = nullptr;
        lut::serialize<uint_t>(jobIds, stream, io);
        utl::serialize(activeJobId, stream, io);
        //         ASSERTD(jobIds.size() > 1);
        //         ASSERTD(activeJobId != uint_t_max);
        _jobsOwner = true;
        uint_vect_t::iterator it;
        for (it = jobIds.begin(); it != jobIds.end(); it++)
        {
            Job* job = new Job();
            job->id() = *it;
            job->preference() = *it; //necessary for job_set_preference_t
                                     //             utl::cout << "JobGroup::serialize jobId: "
                                     //                       << job->id() << utl::endlf;
            _jobs.insert(job);
            if (job->id() == activeJobId)
            {
                _activeJob = job;
            }
        }
        //         ASSERTD(_jobs.size() > 1);
        //         ASSERTD(_activeJob != nullptr);
    }
    else
    {
        // serialize out jobs and activeJob
        ASSERTD(_jobs.size() > 1);
        ASSERTD(_activeJob != nullptr);
        job_set_id_t::iterator it;
        for (it = _jobs.begin(); it != _jobs.end(); it++)
        {
            Job* job = *it;
            jobIds.push_back(job->id());
        }
        lut::serialize<uint_t>(jobIds, stream, io);
        utl::serialize(_activeJob->id(), stream, io);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
JobGroup::setJobs(job_set_pref_t jobs, bool owner)
{
    if (_jobsOwner)
        deleteCont(_jobs);
    _jobs = jobs;
    _jobsOwner = owner;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
JobGroup::setActiveJob(uint_t jobIdx)
{
    ASSERTD(jobIdx < _jobs.size());
    uint_t jobSid = uint_t_max;
    if (_activeJob != nullptr)
    {
        jobSid = _activeJob->serialId();
        _activeJob->active() = false;
        _activeJob->serialId() = uint_t_max;
    }
    uint_t k = 0;
    for (job_set_pref_t::const_iterator it = _jobs.begin(); it != _jobs.end(); it++)
    {
        Job* job = (*it);
        if (jobIdx == k)
        {
            job->active() = true;
            job->serialId() = jobSid;
            _activeJob = job;
            break;
        }
        k++;
    }
#ifdef DEBUG_UNIT
    for (job_set_pref_t::const_iterator it = _jobs.begin(); it != _jobs.end(); it++)
    {
        Job* job = (*it);
        utl::cout << "setActiveJob: group:" << _id << ", job:" << job->id()
                  << ", active:" << job->active() << ", sid:" << job->serialId() << utl::endlf;
    }
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
JobGroup::setActiveJob(Job* job)
{
    ASSERTD(job != nullptr);
    ASSERTD(job->active());

    if (_activeJob == job)
        return;

    uint_t jobSid = uint_t_max;
    if (_activeJob != nullptr)
    {
        jobSid = _activeJob->serialId();
        _activeJob->active() = false;
        _activeJob->serialId() = uint_t_max;
    }
    job->serialId() = jobSid;
    _activeJob = job;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

// String
// JobGroup::toString() const
// {
//     MemStream str;
//     str << Item::toString()
//         << '\n' << "   "
//         << "itemType:Manufacture"
//         << ", batchSize:" << _batchSize
//         << ", #plans:" << _itemPlans.size();
//     itemplan_set_t::iterator it;
//     for (it = _itemPlans.begin(); it != _itemPlans.end();
//          it++)
//     {
//         ItemPlanRelation* itemPlan = (*it);
//         str << '\n' << "      "
//             << itemPlan->toString();
//     }
//     str << '\0';
//     return String((char*)str.get());
// }

////////////////////////////////////////////////////////////////////////////////////////////////////

void
JobGroup::init()
{
    _id = uint_t_max;
    _activeJob = nullptr;
    _jobsOwner = false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
JobGroup::deInit()
{
    if (_jobsOwner)
    {
        deleteCont(_jobs);
    }
    else
    {
        _jobs.clear();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;
