#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <gop/StringInd.h>
#include <cse/ClevorDataSet.h>
#include <cse/Scheduler.h>
#include <cse/SchedulingContext.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Choose among alternate resources, and generate a schedule.

   For each alternate-resource requirement, AltResSelector chooses one of the possible resources,
   then invokes another schedule-builder.
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class AltResSelector : public Scheduler
{
    UTL_CLASS_DECL(AltResSelector, Scheduler);

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
    typedef std::vector<cls::DiscreteResourceRequirement*> rr_vector_t;

private:
    void init();
    void
    deInit()
    {
    }

    /** Set alternate-resource requirements. */
    void setResGroupReqs(const ClevorDataSet* dataSet);

    /** Select resources. */
    void setSelectedResources(gop::StringInd<uint_t>* ind, SchedulingContext* context) const;

private:
    rr_vector_t _resGroupReqs;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;
