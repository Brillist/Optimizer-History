#ifndef CLS_RESOURCE_H
#define CLS_RESOURCE_H

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <libutl/SpanCol.h>
#include <clp/Manager.h>
#include <cls/ResourceCalendar.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

class Schedule;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Resource (abstract).

   Resource is an abstract base for all resource classes.  When all
   resource constraints involving a resource are known, the resource should
   be closed (with close()) to enable constraint propagation.

   <b>Attributes:</b>

   \arg breaks : During break times (see addBreak()), the resource is
        unavailable for execution of activities.  The execution of breakable
        activities (BrkActivity) can be interrupted by resource breaks.

   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class Resource : public utl::Object
{
    UTL_CLASS_DECL(Resource, utl::Object);
    UTL_CLASS_NO_COPY;

public:
    /** Constructor. */
    Resource(Schedule* schedule)
    {
        init();
        _schedule = schedule;
    }

    /** Get the manager. */
    clp::Manager* manager() const;

    /** Get the schedule. */
    Schedule*
    schedule() const
    {
        return _schedule;
    }

    /** Get the schedule. */
    Schedule*&
    schedule()
    {
        return _schedule;
    }

    /** Get the id. */
    uint_t
    id() const
    {
        return _id;
    }

    /** Get the id. */
    uint_t&
    id()
    {
        return _id;
    }

    /** Get the serial-id. */
    uint_t
    serialId() const
    {
        return _serialId;
    }

    /** Get the serial-id. */
    uint_t&
    serialId()
    {
        return _serialId;
    }

    /** Get the name. */
    const std::string&
    name() const
    {
        return _name;
    }

    /** Get the name. */
    std::string&
    name()
    {
        return _name;
    }

    /** Get the associated object. */
    void*
    object() const
    {
        return _object;
    }

    /** Get the associated object. */
    void*&
    object()
    {
        return _object;
    }

    /** Get visited flag. */
    bool
    visited() const
    {
        return _visited;
    }

    /** Get visited flag. */
    bool&
    visited()
    {
        return _visited;
    }

private:
    void init();
    void deInit();

private:
    uint_t _id;
    //Note: _serialId is a bad name. it actually represents the position of
    //      the resource in cls::Schedule::_resourcesArray. Joe, Dec 27, 2007
    uint_t _serialId;
    Schedule* _schedule;
    std::string _name;
    void* _object;
    bool _visited;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Order resources by id.

   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

struct ResIdOrdering : public std::binary_function<Resource*, Resource*, bool>
{
    bool
    operator()(const Resource* lhs, const Resource* rhs) const
    {
        return (lhs->id() < rhs->id());
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_END;

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
