#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <cls/BrkActivity.h>
#include <cse/ClevorDataSet.h>
#include <cse/Scheduler.h>
#include <cse/SchedulingContext.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Choose required capacities, and generate a schedule.

   PtSelector assigns a processing time to all activities that have multiple
   possible processing times, then it invokes another schedule-builder.
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class PtSelector : public Scheduler
{
    UTL_CLASS_DECL(PtSelector, Scheduler);

public:
    virtual void copy(const utl::Object& rhs);

    virtual void setStringBase(gop::Operator* op) const;

    virtual uint_t stringSize(const ClevorDataSet& dataSet) const;

    virtual void initialize(const gop::DataSet* dataSet = nullptr, uint_t stringBase = 0);

    virtual void initializeInd(gop::Ind* ind,
                               const gop::DataSet* dataSet,
                               lut::rng_t* rng = nullptr,
                               void* param = nullptr);

    /** Make a schedule. */
    virtual void run(gop::Ind* ind, gop::IndBuilderContext* context) const;

private:
    typedef std::vector<cls::PtActivity*> act_vector_t;

private:
    void init();
    void
    deInit()
    {
    }

    /** Set activities. */
    void setActs(const ClevorDataSet* dataSet);

    /** Select processing times. */
    void setPts(gop::StringInd<uint_t>* ind, SchedulingContext* context) const;

private:
    act_vector_t _acts;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;
