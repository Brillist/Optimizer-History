#ifndef CSE_JOBGROUP_H
#define CSE_JOBGROUP_H

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <cse/Job.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/* class ManufactureItem; */

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Alternative jobs group. Only one job (called active job)  from the group 
   can be selected for a scheduling run. 

   \author Joe Zhou
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class JobGroup : public utl::Object
{
    UTL_CLASS_DECL(JobGroup, utl::Object);

public:
    virtual void copy(const utl::Object& rhs);

    virtual void
    serialize(utl::Stream& stream, uint_t io, uint_t mode = utl::ser_default);

    /// \name Accessors
    //@{
    /** Manufacture Item. */
    /*     const ManufactureItem* item() const */
    /*     { return _item; } */

    /** Manufacture Item. */
    /*     ManufactureItem*& item() */
    /*     { return _item; } */

    /** Id. */
    uint_t
    id() const
    {
        return _id;
    }

    /** Id. */
    uint_t&
    id()
    {
        return _id;
    }

    /** Name. */
    const std::string&
    name() const
    {
        return _name;
    }

    /** Name. */
    std::string&
    name()
    {
        return _name;
    }

    /** Alternative jobs. */
    const cse::job_set_pref_t&
    jobs() const
    {
        return _jobs;
    }

    /** Alternative jobs. */
    cse::job_set_pref_t&
    jobs()
    {
        return _jobs;
    }

    /** Active job. */
    cse::Job*
    activeJob() const
    {
        return _activeJob;
    }

    /** Own jobs. */
    bool
    jobsOwner() const
    {
        return _jobsOwner;
    }
    //@}

    /** set jobs. */
    void setJobs(job_set_pref_t jobs, bool owner = false);

    /** set activeJob. */
    void setActiveJob(cse::Job* job);

    /** set activeJob. */
    void setActiveJob(uint_t jobIdx);

    /*     String toString() const; */
private:
    void init();
    void deInit();

private:
    /*     ManufactureItem* _item; */
    uint_t _id;
    std::string _name;
    cse::job_set_pref_t _jobs;
    cse::Job* _activeJob;

    bool _jobsOwner;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

struct JobGroupIdOrdering : public std::binary_function<JobGroup*, JobGroup*, bool>
{
    bool operator()(const JobGroup* lhs, const JobGroup* rhs) const;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

typedef std::vector<JobGroup*> jobgroup_vector_t;
typedef std::set<JobGroup*, JobGroupIdOrdering> jobgroup_set_id_t;

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
