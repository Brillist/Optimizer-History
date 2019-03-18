#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <clp/ConstrainedBound.h>
#include <cls/CompositeTimetableDomain.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

class CompositeResource;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Composite resource timetable.

   For each point in time, a composite resource's timetable specifies the minimal and maximal
   provided capacity, as well as the list of available resources.

   \see CompositeSpan
   \see CompositeTimetableDomain
   \ingroup cls
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class CompositeTimetable : public utl::Object
{
    UTL_CLASS_DECL(CompositeTimetable, utl::Object);
    UTL_CLASS_NO_COPY;

public:
    /**
       Constructor.
       \param res related CompositeResource
       \param schedule related Schedule
       \param resIds ids of DiscreteResource%s belonging to the CompositeResource
    */
    CompositeTimetable(CompositeResource* res, Schedule* schedule, const uint_set_t& resIds)
    {
        init();
        initialize(res, schedule, resIds);
    }

    /**
       Initialize.
       \param res related CompositeResource
       \param schedule related Schedule
       \param resIds ids of DiscreteResource%s belonging to the CompositeResource
    */
    void initialize(CompositeResource* res, Schedule* schedule, const uint_set_t& resIds);

    /// \name Accessors (const)
    //@{
    /** Get the manager. */
    clp::Manager*
    manager() const
    {
        return _mgr;
    }

    /** Get the range of all spans. */
    utl::Span<int> range() const;

    /** Get the domain. */
    const CompositeTimetableDomain*
    domain() const
    {
        return &_domain;
    }

    /** Head iterator. */
    const clp::IntSpan*
    head() const
    {
        return _domain.head();
    }

    /** End iterator. */
    const clp::IntSpan*
    tail() const
    {
        return _domain.tail();
    }
    //@}

    /// \name Capacity Expressions
    //@{
    /** Add capacity-expression. */
    clp::IntExp*
    addCapExp(uint_t cap)
    {
        return _domain.addCapExp(cap);
    }

    /** Remove capacity-expression. */
    void
    remCapExp(uint_t cap)
    {
        _domain.remCapExp(cap);
    }
    //@}

    /// \name Query
    //@{
    /** Find the span that overlaps the given value. */
    const CompositeSpan* find(int val) const;

    /** Find the first span that overlaps the given span. */
    CompositeSpan* find(int val);
    //@}

    /// \name Domain Modification
    //@{
    /**
       Add provided capacity.
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
       \param breakList break list to be observed (default: use the activity's breakList
       \param resId id of DiscreteResource to allocate from (default: use PreferredResources)
       \param updateDiscrete also allocate capacity in DiscreteResource%s? (default: true)
       \see Activity::breakList
       \see CompositeResource::allocate
       \see CompositeTimetableDomain::allocate
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
    /** Add a range-bound. */
    void
    addRangeBound(clp::ConstrainedBound* bound)
    {
        _rangeBounds.add(bound);
    }

    /** Remove a domain-bound. */
    void
    removeRangeBound(clp::ConstrainedBound* bound)
    {
        _rangeBounds.remove(bound);
    }
    //@}
protected:
    CompositeTimetableDomain _domain;

private:
    void init();
    void deInit();

    void raiseEvents();

private:
    using cb_set_t = clp::RevSet<clp::ConstrainedBound>;

private:
    const CompositeResource* _res;
    clp::Manager* _mgr;
    bool _managed;
    cb_set_t _rangeBounds;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_END;
