#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <clp/IntExp.h>
#include <clp/RevIntSpanCol.h>
#include <cls/CompositeSpan.h>
#include <cls/PreferredResources.h>
#include <cls/PtActivity.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Timetable representation for composite resources.

   \ingroup cls
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class CompositeTimetableDomain : public utl::Object, public clp::RevIntSpanCol
{
    UTL_CLASS_DECL(CompositeTimetableDomain, utl::Object);
    UTL_CLASS_NO_COPY;

public:
    /**
       Constructor.
       \param schedule related Schedule
       \param resIds ids of DiscreteResource%s belonging to the CompositeResource
    */
    CompositeTimetableDomain(Schedule* schedule, const uint_set_t& resIds)
    {
        init();
        initialize(schedule, resIds);
    }

    /**
       Initialize.
       \param schedule related Schedule
       \param resIds ids of DiscreteResource%s belonging to the CompositeResource
    */
    void initialize(Schedule* schedule, const uint_set_t& resIds);

    /// \name Capacity Expressions
    //@{
    /** Add capacity-expression. */
    clp::IntExp* addCapExp(uint_t cap);

    /** Remove capacity-expression. */
    void remCapExp(uint_t cap);
    //@}

    /// \name Capacity Provision and Allocation
    //@{
    /**
       Add capacity.
       \param min start of capacity provision span
       \param max end of capacity provision span
       \param resId id of DiscreteResource providing capacity
    */
    void add(int min, int max, uint_t resId);

    /**
       Allocate capacity.
       \param min start of allocation span
       \param max end of allocation span
       \param cap allocated capacity
       \param act responsible Activity
       \param pr preferred resources
       \param breakList break list to be observed (default: use the activity's breakList)
       \param resId id of DiscreteResource to allocate from (default: use PreferredResources)
       \param updateDiscrete also allocate capacity in DiscreteResource%s? (default: true)
       \see Activity::breakList
       \see CompositeTimetable::allocate
    */
    void allocate(int min,
                  int max,
                  uint_t cap,
                  Activity* act,
                  const PreferredResources* pr,
                  const clp::IntExp* breakList = nullptr,
                  uint_t resId = uint_t_max,
                  bool updateDiscrete = true);
    //@}

    /// \name Events
    //@{
    /** Any event? */
    bool
    anyEvent() const
    {
        return (_events != 0);
    }

    /** Clear event flags. */
    void
    clearEvents()
    {
        _events = 0;
    }

    /** Empty event? */
    bool
    emptyEvent() const
    {
        return ((_events & ef_empty) != 0);
    }

    /** Range event? */
    bool
    rangeEvent() const
    {
        return ((_events & ef_range) != 0);
    }
    //@}
protected:
    enum event_flag_t
    {
        ef_empty = 1,
        ef_range = 2
    };

protected:
    CompositeSpan*
    newCS(int min, int max, clp::IntExpDomainAR* resIds = nullptr, uint_t level = uint_t_max);

    virtual clp::IntSpan*
    newIntSpan(int min, int max, uint_t v0, uint_t v1, uint_t level = uint_t_max);

private:
    void init();
    void deInit();

    bool allocate(clp::IntExpDomainAR* resIds,
                  uint_t cap,
                  IntActivity* act,
                  const PreferredResources* pr,
                  int min = utl::int_t_max,
                  int max = utl::int_t_max,
                  uint_t resId = uint_t_max,
                  bool updateDiscrete = true);

private:
    clp::IntSpan* _dummyIntSpan;
    Schedule* _schedule;
    int* _values;
    uint_t _numValues;
    clp::IntExp** _capExps;
    uint_t* _capExpCounts;
    size_t _capExpsSize;
    size_t _capExpCountsSize;
    uint_t _events;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_END;
