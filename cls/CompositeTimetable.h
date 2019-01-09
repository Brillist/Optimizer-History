#ifndef CLS_COMPOSITETIMETABLE_H
#define CLS_COMPOSITETIMETABLE_H

//////////////////////////////////////////////////////////////////////////////

#include <clp/ConstrainedBound.h>
#include <cls/CompositeTimetableDomain.h>

//////////////////////////////////////////////////////////////////////////////

CLS_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////

class CompositeResource;

//////////////////////////////////////////////////////////////////////////////

/**
   Composite resource timetable.

   For each point in time, a composite resource's timetable specifies
   the minimal and maximal provided capacity, as well as the list of
   available resources.

   \author Adam McKee
*/

//////////////////////////////////////////////////////////////////////////////

class CompositeTimetable : public utl::Object
{
    UTL_CLASS_DECL(CompositeTimetable);
    UTL_CLASS_NO_COPY;
public:
    typedef std::set<utl::uint_t> uint_set_t;
public:
    /** Constructor. */
    CompositeTimetable(
        CompositeResource* res,
        Schedule* schedule,
        const uint_set_t& resIds)
    { init(); initialize(res, schedule, resIds); }

    /// \name Accessors
    //@{
    /** Get the manager. */
    clp::Manager* manager() const
    { return _mgr; }

    /** Set the manager. */
    void initialize(
        CompositeResource* res,
        Schedule* schedule,
        const uint_set_t& resIds);

    /** Get the range of all spans. */
    utl::Span<int> range() const;

    /** Get the domain. */
    const CompositeTimetableDomain* domain() const
    { return &_domain; }

    /** Head iterator. */
    const clp::IntSpan* head() const
    { return _domain.head(); }

    /** End iterator. */
    const clp::IntSpan* tail() const
    { return _domain.tail(); }

    /** Add capacity-expression. */
    clp::IntExp* addCapExp(utl::uint_t cap)
    { return _domain.addCapExp(cap); }

    /** Remove capacity-expression. */
    void remCapExp(utl::uint_t cap)
    { _domain.remCapExp(cap); }
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
    /** Add provided capacity. */
    void add(int min, int max, utl::uint_t resId);

    /** Allocate capacity. */
    void allocate(
        int min,
        int max,
        utl::uint_t cap,
        Activity* act,
        const PreferredResources* pr,
        const clp::IntExp* breakList = nullptr,
        utl::uint_t resId = utl::uint_t_max,
        bool updateDiscrete = true);
    //@}

    /// \name Events
    //@{
    /** Add a range-bound. */
    void addRangeBound(clp::ConstrainedBound* bound)
    { _rangeBounds.add(bound); }

    /** Remove a domain-bound. */
    void removeRangeBound(clp::ConstrainedBound* bound)
    { _rangeBounds.remove(bound); }
    //@}
protected:
    CompositeTimetableDomain _domain;
private:
    void init();
    void deInit();

    void raiseEvents();
private:
    typedef clp::RevSet<clp::ConstrainedBound> cb_set_t;
private:
    const CompositeResource* _res;
    clp::Manager* _mgr;
    bool _managed;
    cb_set_t _rangeBounds;
};

//////////////////////////////////////////////////////////////////////////////

CLS_NS_END;

//////////////////////////////////////////////////////////////////////////////

#endif
