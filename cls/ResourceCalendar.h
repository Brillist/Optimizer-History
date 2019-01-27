#ifndef CLS_RESOURCECALENDAR_H
#define CLS_RESOURCECALENDAR_H

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <libutl/SpanCol.h>
#include <clp/Manager.h>
#include <clp/IntVar.h>
#include <cls/ResourceCalendarSpan.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

enum rescal_t
{
    rc_simple,       /**< simple resource calendar */
    rc_allAvailable, /**< all resources are available */
    rc_anyAvailable, /**< any resource is available */
    rc_undefined     /**< undefined type */
};

////////////////////////////////////////////////////////////////////////////////////////////////////

class ResourceCalendarSpec : public utl::Object
{
    UTL_CLASS_DECL(ResourceCalendarSpec, utl::Object);

public:
    typedef std::set<uint_t> uint_set_t;

public:
    /** Constructor. */
    ResourceCalendarSpec(rescal_t type, const uint_set_t& calIds);

    /** Copy another instance. */
    virtual void copy(const utl::Object& rhs);

    /** Get type. */
    rescal_t
    type() const
    {
        return _type;
    }

    /** Get composite calendar ids. */
    const uint_set_t&
    calIds() const
    {
        return _calIds;
    }

    /** Less-than comparison operator. */
    bool operator<(const ResourceCalendarSpec& rhs) const;

private:
    void
    init()
    {
        _type = rc_undefined;
    }
    void
    deInit()
    {
    }

private:
    rescal_t _type;     // type of calendar
    uint_set_t _calIds; // composite calendar ids
};

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Resource calendar.

   Store a resource's calendar, and provide efficient implementations of
   common calendar-related queries.

   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class ResourceCalendar : public utl::SpanCol<int>
{
    UTL_CLASS_DECL(ResourceCalendar, utl::SpanCol<int>);

public:
    virtual int compare(const utl::Object& rhs) const;

    virtual void
    serialize(utl::Stream& stream, uint_t io, uint_t mode = utl::ser_default);

    virtual bool isMergeable(const utl::Object& lhs, const utl::Object& rhs) const;

    void dump(utl::Stream& os, time_t originTime, uint_t timeStep);

    /** Get spec. */
    const ResourceCalendarSpec&
    spec() const
    {
        return _spec;
    }

    /** Get spec. */
    ResourceCalendarSpec&
    spec()
    {
        return _spec;
    }

    /** Get serial id. */
    uint_t
    serialId() const
    {
        return _serialId;
    }

    /** Get serial id. */
    uint_t&
    serialId()
    {
        return _serialId;
    }

    /** Get unique id. */
    uint_t id() const;

    /** Add a break. */
    void addBreak(int begin, int end);

    /** Compile the list to allow efficient access. */
    void compile(int horizonTS);

    /** Create a break-list. */
    void makeBreakList(clp::Manager* mgr);

    /** Get break-list. */
    clp::IntVar*
    breakList() const
    {
        return _breakList;
    }

    /** Find a valid es,ef pair. */
    void findForward(int& es, int& ef, uint_t pt) const;

    /** Find a valid es,ef pair. */
    void findBackward(int& lf, int& ls, uint_t pt) const;

    /** Add compiled spans to another calendar. */
    void addCompiledSpansTo(ResourceCalendar* cal, rcs_status_t status) const;

    /** Get non-break time in the given time span. */
    uint_t
    getNonBreakTime(int begin, int end) const
    {
        return ((end - begin + 1) - getBreakTime(begin, end));
    }

    /** Get break time during the given span. */
    uint_t getBreakTime(int begin, int end) const;

    /** Get maximum break time-slot <= t. */
    int getBreakTimePrev(int t) const;

    /** Get minimum break time-slot >= t. */
    int getBreakTimeNext(int t) const;

    /** Get maximum non-break time-slot <= t. */
    int getNonBreakTimePrev(int t) const;

    /** Get minimum non-break time-slot >= t. */
    int getNonBreakTimeNext(int t) const;

    /** Get start-time corresponding to end time. */
    int getStartTimeForEndTime(int endTime, uint_t pt) const;

    /** Get end-time corresponding to a start time. */
    int getEndTimeForStartTime(int startTime, uint_t pt) const;

private:
    void init();
    void deInit();
    void makeSpansArray(int horizonTS);
    void check(const iterator* testIt = nullptr);
    uint_t tsPT(int ts) const;
    int ptTS(uint_t pt) const;
    const ResourceCalendarSpan* findSpanByTime(int ts) const;
    const ResourceCalendarSpan* findSpanByPt(int pt) const;

private:
    //
    // A few notes that might be helpful...
    //
    // ts == time-slot, pt == processing-time
    //
    // ptTS[pt] = ts s.t. tsPT[ts] = pt
    // tsPT[ts] = pt s.t. processing time during [0,ts] = pt
    // ptTS[tsPT[ts]] = pt   iff   ts not in break
    // tsPT[ptTS[pt]] = ts
    //
    uint_t _serialId;
    ResourceCalendarSpec _spec;
    ResourceCalendarSpan** _spans;
    ResourceCalendarSpan** _spansLim;
    int _minBreakTime;
    int _maxBreakTime;
    int _minNonBreakTime;
    int _maxNonBreakTime;
    uint_t _maxPT;
    int _maxTS;
    clp::IntVar* _breakList;
    mutable ResourceCalendarSpan _searchSpan;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Order ResourceCalendar objects by comparing their spans.

   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

struct ResourceCalendarOrdering
    : public std::binary_function<ResourceCalendar*, ResourceCalendar*, bool>
{
    bool
    operator()(const ResourceCalendar* lhs, const ResourceCalendar* rhs) const
    {
        return (lhs->compare(*rhs) < 0);
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Order ResourceCalendar objects by specification.

   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

struct ResourceCalendarSpecOrdering
    : public std::binary_function<ResourceCalendar*, ResourceCalendar*, bool>
{
    bool
    operator()(const ResourceCalendar* lhs, const ResourceCalendar* rhs) const
    {
        return (lhs->spec() < rhs->spec());
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_END;

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
