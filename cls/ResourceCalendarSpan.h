#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <libutl/SpanCol.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Resource calendar span type.
   \ingroup cls
*/
enum rcs_t
{
    rcs_dayOfWeek, /**< default */
    rcs_exception, /**< exception */
    rcs_undefined  /**< undefined/null */
};

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Resource calendar span status.
   \ingroup cls
*/
enum rcs_status_t
{
    rcss_available, /**< available */
    rcss_onBreak,   /**< unavailable */
    rcss_undefined  /**< undefined/null */
};

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Resource schedule span.

   A ResourceCalendarSpan specifies the working status of a resource during a time period.

   A resource is always in one of the following states:

      - working
      - on break

   A break in availability represents a period of time where a resource is unavailable for work.
   Execution of an activity is paused while one or more of its required DiscreteResource%s is
   on break.

   \see IntActivity
   \see BrkActivity
   \ingroup cls
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class ResourceCalendarSpan : public utl::Span<int>
{
    UTL_CLASS_DECL(ResourceCalendarSpan, utl::Span<int>);

public:
    /**
       Constructor.
       \param begin first time slot of span
       \param end last time slot of span
       \param type span type (cls::rcs_dayOfWeek or cls::rcs_exception)
       \param status working status (cls::rcss_available or cls::rcss_onBreak)
       \param cap available capacity
    */
    ResourceCalendarSpan(
        int begin, int end, rcs_t type, rcs_status_t status, uint_t cap = uint_t_max)
        : utl::Span<int>(begin, end)
    {
        init(type, status, cap);
    }

    virtual void copy(const utl::Object& rhs);

    virtual int compare(const utl::Object& rhs) const;

    virtual void serialize(utl::Stream& stream, uint_t io, uint_t mode = utl::ser_default);

    virtual String toString() const;

    /** Get a human-readable string representation. */
    String toString(time_t originTime, uint_t timeStep) const;

    /// \name Accessors (const)
    //@{
    /** Get the type. */
    rcs_t
    type() const
    {
        return _type;
    }

    /** Get the status. */
    rcs_status_t
    status() const
    {
        return _status;
    }

    /** Get the capacity. */
    uint_t
    capacity() const
    {
        return _cap;
    }

    /** Get the cumulative processing-time. */
    const uint_t&
    cumPt() const
    {
        return _cumPt;
    }

    /** Get the prev pointer. */
    ResourceCalendarSpan*
    prev() const
    {
        return _prev;
    }

    /** Get the next ptr. */
    ResourceCalendarSpan*
    next() const
    {
        return _next;
    }
    //@}

    /// \name Accessors (non-const)
    //@{
    /** Set capacity. */
    void
    setCapacity(uint_t cap)
    {
        _cap = cap;
    }

    /** Set cumulative processing-time. */
    void
    setCumPt(uint_t cumPt)
    {
        _cumPt = cumPt;
    }

    /** Set the prev pointer. */
    void
    setPrev(ResourceCalendarSpan* prev)
    {
        _prev = prev;
    }

    /** Get the next pointer. */
    void
    setNext(ResourceCalendarSpan* next)
    {
        _next = next;
    }
    //@}

private:
    void init(rcs_t type = rcs_undefined, rcs_status_t = rcss_undefined, uint_t cap = 0);

    void
    deInit()
    {
    }

private:
    rcs_t _type;
    rcs_status_t _status;
    uint_t _cap;
    uint_t _cumPt;
    ResourceCalendarSpan* _prev;
    ResourceCalendarSpan* _next;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Order ResourceCalendarSpan objects by end time.

   \ingroup cls
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

struct ResourceCalendarSpanEndOrdering
{
    bool
    operator()(const ResourceCalendarSpan* lhs, const ResourceCalendarSpan* rhs) const
    {
        return (lhs->end() < rhs->end());
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Order ResourceCalendarSpan objects by cumulative processing-time.

   \ingroup cls
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

struct ResourceCalendarSpanCumPtOrdering
{
    bool
    operator()(const ResourceCalendarSpan* lhs, const ResourceCalendarSpan* rhs) const
    {
        return (lhs->cumPt() < rhs->cumPt());
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_END;
