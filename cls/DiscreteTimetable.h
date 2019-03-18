#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <clp/ConstrainedBound.h>
#include <cls/DiscreteTimetableDomain.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

class DiscreteResource;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Discrete resource timetable.

   A discrete resource's timetable records minimal required and maximal provided capacity at
   each point in time.

   A DiscreteTimetable can create "capacity expressions" which are IntVar objects whose domains
   track the availability of various required capacities.  Capacity expressions simplify the
   task of finding times when an activity can execute.

   \see ESboundTimetable
   \see LFboundTimetable
   \see DiscreteTimetableDomain
   \ingroup cls
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class DiscreteTimetable : public utl::Object
{
    UTL_CLASS_DECL(DiscreteTimetable, utl::Object);
    UTL_CLASS_NO_COPY;

public:
    /** Constructor. */
    DiscreteTimetable(clp::Manager* mgr)
    {
        init();
        setManager(mgr);
    }

    /// \name Accessors (const)
    //@{
    /** Get the resource. */
    const DiscreteResource*
    resource() const
    {
        return _res;
    }

    /** Get the manager. */
    clp::Manager*
    manager() const
    {
        return _mgr;
    }

    /** Get the domain. */
    const DiscreteTimetableDomain*
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

    /// \name Accessors (non-const)
    //@{
    /** Get the resource. */
    void setResource(const DiscreteResource* res)
    {
        _res = res;
    }

    /** Set the manager. */
    void setManager(clp::Manager* mgr);
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
    /** Get the required and provided energy. */
    void energy(uint_t& required, uint_t& provided) const;

    /** Get the required and provided energy over the given span. */
    void energy(const utl::Span<int>& span, uint_t& required, uint_t& provided) const;

    /** Get the maximal capacity at the given time. */
    uint_t max(int t) const;

    /** Find the span that overlaps the given value. */
    const clp::IntSpan* find(int val) const;

    /** Find the span that overlaps the given value. */
    clp::IntSpan* find(int val);
    //@}

    /// \name Domain Modification
    //@{
    /**
       Add required and/or provided capacity over a time span.
       \param min start of affected span
       \param max end of affected span
       \param reqCap addition to required capacity (may be negative)
       \param prvCap addition to provided capacity (may be negative)
       \return minimum available capacity in the time span
    */
    uint_t add(int min, int max, int reqCap, int prvCap);
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
    DiscreteTimetableDomain _domain;

private:
    void init();
    void deInit();

    void raiseEvents();

private:
    typedef clp::RevSet<clp::ConstrainedBound> cb_set_t;

private:
    const DiscreteResource* _res;
    clp::Manager* _mgr;
    bool _managed;
    cb_set_t _rangeBounds;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_END;
