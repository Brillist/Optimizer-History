#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <libutl/SpanCol.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/** Resource calendar span type. */
enum rcs_t
{
    rcs_dayOfWeek, /**< default */
    rcs_exception, /**< exception */
    rcs_undefined  /**< undefined/null */
};

////////////////////////////////////////////////////////////////////////////////////////////////////

/** Resource calendar span status. */
enum rcs_status_t
{
    rcss_available, /**< available */
    rcss_onBreak,   /**< unavailable */
    rcss_undefined  /**< undefined/null */
};

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Resource schedule span.

   A resource is always in one of the following states:

      - working
      - on break

   A break in availability represents a period of time where a resource is
   unavailable for work -- any work that the resource is performing when the
   break commences will be suspended and resumed when the break is over.

   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class ResourceCalendarSpan : public utl::Span<int>
{
    UTL_CLASS_DECL(ResourceCalendarSpan, utl::Span<int>);

public:
    /** Constructor. */
    ResourceCalendarSpan(
        int begin, int end, rcs_t type, rcs_status_t status, uint_t cap = uint_t_max)
        : utl::Span<int>(begin, end)
    {
        init(type, status, cap);
    }

    /** Copy another instance. */
    virtual void copy(const utl::Object& rhs);

    /** Compare with another instance. */
    virtual int compare(const utl::Object& rhs) const;

    virtual void serialize(utl::Stream& stream, uint_t io, uint_t mode = utl::ser_default);

    /** Get a human-readable string representation. */
    virtual String toString() const;

    /** Get a human-readable string representation. */
    String toString(time_t originTime, uint_t timeStep) const;

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

    /** Get the capacity. */
    uint_t&
    capacity()
    {
        return _cap;
    }

    /** Get the cumulative processing-time. */
    const uint_t&
    cumPt() const
    {
        return _cumPt;
    }

    /** Get the cumulative processing-time. */
    uint_t&
    cumPt()
    {
        return _cumPt;
    }

    /** Get the prev ptr. */
    ResourceCalendarSpan*
    prev() const
    {
        return _prev;
    }

    /** Get the prev ptr. */
    ResourceCalendarSpan*&
    prev()
    {
        return _prev;
    }

    /** Get the next ptr. */
    ResourceCalendarSpan*
    next() const
    {
        return _next;
    }

    /** Get the next ptr. */
    ResourceCalendarSpan*&
    next()
    {
        return _next;
    }

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

   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

struct ResourceCalendarSpanEndOrdering
    : public std::binary_function<ResourceCalendarSpan*, ResourceCalendarSpan*, bool>
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

   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

struct ResourceCalendarSpanCumPtOrdering
    : public std::binary_function<ResourceCalendarSpan*, ResourceCalendarSpan*, bool>
{
    bool
    operator()(const ResourceCalendarSpan* lhs, const ResourceCalendarSpan* rhs) const
    {
        return (lhs->cumPt() < rhs->cumPt());
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_END;
