#ifndef CSE_CAPSELECTOR_H
#define CSE_CAPSELECTOR_H

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <cls/BrkActivity.h>
#include <cse/ClevorDataSet.h>
#include <cse/Scheduler.h>
#include <cse/SchedulingContext.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

class CapSelector : public Scheduler
{
    UTL_CLASS_DECL(CapSelector, Scheduler);

public:
    virtual void copy(const utl::Object& rhs);

    virtual void setStringBase(gop::Operator* op) const;

    virtual uint_t stringSize(const ClevorDataSet& dataSet) const;

    virtual void initialize(const gop::DataSet* dataSet = nullptr, uint_t stringBase = 0);

    virtual void initializeInd(gop::Ind* ind,
                               const gop::DataSet* dataSet,
                               lut::rng_t* rng = nullptr,
                               void* param = nullptr);

    virtual void initializeRandomInd(gop::Ind* ind,
                                     const gop::DataSet* dataSet,
                                     lut::rng_t* rng = nullptr,
                                     void* param = nullptr);

    /** Make a schedule. */
    virtual void run(gop::Ind* ind, gop::IndBuilderContext* context) const;

private:
    typedef std::vector<cls::IntActivity*> act_vector_t;

private:
    void init();
    void
    deInit()
    {
    }

    /** Set activities. */
    void setActs(const ClevorDataSet* dataSet);

    /** Select processing times. */
    void setCaps(gop::StringInd<uint_t>* ind, SchedulingContext* context) const;

private:
    act_vector_t _acts;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
