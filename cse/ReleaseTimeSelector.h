#ifndef CSE_RELEASETIMESELECTOR_H
#define CSE_RELEASETIMESELECTOR_H

//////////////////////////////////////////////////////////////////////////////

#include <cse/ClevorDataSet.h>
#include <cse/Scheduler.h>
#include <cse/SchedulingContext.h>

//////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////

/**
   Choose an alternative release time.

   ReleaseTimeSelector assigns a release time to a job, then it invokes 
   another schedule-builder.

   \author Joe Zhou

   \date created on November 9, 2007
*/

//////////////////////////////////////////////////////////////////////////////

class ReleaseTimeSelector : public Scheduler
{
    UTL_CLASS_DECL(ReleaseTimeSelector);
public:
    virtual void copy(const utl::Object& rhs);

    /** Set ReleaseTimeMutate's _stringBase. */
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
    void setActs(const ClevorDataSet* dataSet);

    /** Select process plans. */
    void setReleaseTimes(
        gop::StringInd<utl::uint_t>* ind,
        SchedulingContext* context) const;
private:
    cls::act_vect_t _acts;
    lut::uint_vect_t _minRlsTimes;
};

//////////////////////////////////////////////////////////////////////////////

CSE_NS_END;

//////////////////////////////////////////////////////////////////////////////

#endif
