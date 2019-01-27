#ifndef CSE_FORWARDSCHEDULER_H
#define CSE_FORWARDSCHEDULER_H

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <gop/StringInd.h>
#include <cse/ClevorDataSet.h>
#include <cse/Scheduler.h>
#include <cse/SchedulingContext.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Generate a forward schedule.

   The operation of this scheduler is very simple.  At each iteration,
   it selects the unscheduled job whose due date is minimum.  Then, all
   operations belonging to the selected job are scheduled at their
   earliest possible start time.  The loop is repeated until all
   jobs have been scheduled.

   \code
   while (!schedule.allJobsScheduled())
   {
      job = schedule.unscheduledJobWithMinDueTime()
      while (!job.allOpsScheduled())
      {
         op = job.unscheduledOpWithMinStartTime()
         schedule.scheduleAtMinStartTime(op)
      }
   }
   \endcode

   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class ForwardScheduler : public Scheduler
{
    UTL_CLASS_DECL(ForwardScheduler, Scheduler);

public:
    virtual void copy(const utl::Object& rhs);

    virtual void run(gop::Ind* ind, gop::IndBuilderContext* context) const;

    /** initialize _sortedJobs, _sortedJobOps and _sortedOps. */
    void setJobs(const ClevorDataSet* dataSet) const;
    void setJobOps(const ClevorDataSet* dataSet) const;
    void setOps(const ClevorDataSet* dataSet) const;

    /** get jobs start position in the string. */
    uint_t&
    jobStartPosition()
    {
        return _jobStartPosition;
    }

    /** get ops start position in the string */
    uint_t&
    opStartPosition()
    {
        return _opStartPosition;
    }

private:
    typedef std::vector<uint_t> uint_vector_t;
    typedef std::map<Job*, jobop_vector_t*, JobIdOrdering> job_jobopvector_map_t;

private:
    void init();
    void deInit();

    /** modify the optInitRun's scheduling sequence. */
    void setInitialSeq(gop::StringInd<uint_t>* ind, SchedulingContext* context) const;

    /** generate a default scheduling sequence when optInitRun fails. */
    void setDefaultInitialSeq(gop::StringInd<uint_t>* ind, SchedulingContext* context) const;

    /** write a scheduling sequence to the string. */
    void setSequenceString(gop::StringInd<uint_t>* ind, SchedulingContext* context) const;

private:
    uint_t _jobStartPosition;
    uint_t _opStartPosition;
    mutable bool _initOptRun;
    mutable job_vector_t _sortedJobs;
    mutable jobop_vector_t _sortedOps;
    mutable job_jobopvector_map_t _sortedJobOps;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
