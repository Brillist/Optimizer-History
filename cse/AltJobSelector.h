#ifndef CSE_ALTJOBSELECTOR_H
#define CSE_ALTJOBSELECTOR_H

//////////////////////////////////////////////////////////////////////////////

#include <cse/ClevorDataSet.h>
#include <cse/Scheduler.h>
#include <cse/SchedulingContext.h>
#include <cse/JobGroup.h>

//////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////

/**
   Choose an alternative job, and generate a schedule.

   AltJobSelector assigns a process plan to an manufactureItem which has 
   multiple process plan, then it invokes another schedule-builder.

   \author Joe Zhou
*/

//////////////////////////////////////////////////////////////////////////////

class AltJobSelector : public Scheduler
{
    UTL_CLASS_DECL(AltJobSelector);
public:
    virtual void copy(const utl::Object& rhs);

    /** Set AltJobMutate's _stringBase. */
    virtual void setStringBase(gop::Operator* op) const;

    /** Get the size of the whole string. */
    virtual utl::uint_t stringSize(const ClevorDataSet& dataSet) const;

    /** Initialize _stringBase and _items. */
    virtual void initialize(
        const gop::DataSet* dataSet = nullptr,
        utl::uint_t stringBase = 0);

    virtual void initializeInd(
        gop::Ind* ind,
        const gop::DataSet* dataSet,
        utl::RandNumGen* rng = nullptr,
        void* param = nullptr);

    /** Make a schedule. */
    virtual void run(gop::Ind* ind, gop::IndBuilderContext* context) const;
private:
    void init();
    void deInit() {}

    /** Set _items. */
    void setAltJobGroups(const ClevorDataSet* dataSet);

    /** Select process plans. */
    void setJobs(
        gop::StringInd<utl::uint_t>* ind,
        SchedulingContext* context) const;
private:
    jobgroup_vector_t _jobGroups;
};

//////////////////////////////////////////////////////////////////////////////

CSE_NS_END;

//////////////////////////////////////////////////////////////////////////////

#endif
