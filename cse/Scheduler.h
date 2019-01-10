#ifndef CSE_SCHEDULER_H
#define CSE_SCHEDULER_H

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <gop/IndBuilder.h>
#include <cse/SchedulerConfiguration.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

class ClevorDataSet;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Schedule builder (abstract).

   A scheduler is an algorithm that builds a schedule from previously
   made plans which are stored in the Schedule object itself.

   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class Scheduler : public gop::IndBuilder
{
    UTL_CLASS_DECL_ABC(Scheduler);

public:
    virtual void copy(const utl::Object& rhs);

    virtual void
    serialize(utl::Stream& stream, utl::uint_t io, utl::uint_t mode = utl::ser_default);

    virtual void setStringBase(gop::Operator* op) const;

    /** Set the scheduler configuration. */
    void setConfig(SchedulerConfiguration* config);

    /** Set the nested scheduler. */
    void
    setNestedScheduler(Scheduler* nestedScheduler)
    {
        delete _nestedScheduler;
        _nestedScheduler = nestedScheduler;
    }

    /** Get the chromosome string size. */
    virtual utl::uint_t
    stringSize(const ClevorDataSet& dataSet) const
    {
        return 0;
    }

    /** Get the chromosome string base. */
    utl::uint_t
    getStringBase() const
    {
        return _stringBase;
    }

    /** Set the chromosome string base. */
    void
    setStringBase(utl::uint_t stringBase)
    {
        _stringBase = stringBase;
    }

protected:
    utl::uint_t _stringBase;
    SchedulerConfiguration* _config;
    Scheduler* _nestedScheduler;

private:
    void init();
    void deInit();
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
