#ifndef CLS_DISCRETETIMETABLE_H
#define CLS_DISCRETETIMETABLE_H

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

   A discrete resource's timetable specifies minimal required and
   maximal provided capacity at each point in time.  The domain
   is represented by an instance of RevIntSpanCol - see the documentation
   of that class for more information.

   \author Adam McKee
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

    /// \name Accessors
    //@{
    /** Get the resource. */
    const DiscreteResource*
    res() const
    {
        return _res;
    }

    /** Get the resource. */
    const DiscreteResource*&
    res()
    {
        return _res;
    }

    /** Get the manager. */
    clp::Manager*
    manager() const
    {
        return _mgr;
    }

    /** Set the manager. */
    void setManager(clp::Manager* mgr);

    /** Get the range of all spans. */
    utl::Span<int> range() const;

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

    /// \name Energy
    //@{
    /** Get the required and provided energy. */
    void energy(uint_t& required, uint_t& provided) const;

    /** Get the required and provided energy over the given span. */
    void energy(const utl::Span<int>& span, uint_t& required, uint_t& provided) const;
    //@}

    /// \name Query
    //@{
    /** Get the maximal capacity at the given time. */
    uint_t max(int t) const;

    /** Find the span that overlaps the given value. */
    const clp::IntSpan* find(int val) const;

    /** Find the first span that overlaps the given span. */
    clp::IntSpan* find(int val);
    //@}

    /// \name Domain Modification
    //@{
    /** Add required,provided capacity in the given span. */
    uint_t add(int min, int max, int minReq, int maxPrv);

    /** Subtract required,provided capacity in the given span. */
    uint_t subtract(int min, int max, int minReq, int maxPrv);
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

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
