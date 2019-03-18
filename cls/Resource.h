#pragma once

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

   \see CompositeResource
   \see DiscreteResource
   \ingroup cls
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class Resource : public utl::Object
{
    UTL_CLASS_DECL(Resource, utl::Object);
    UTL_CLASS_NO_COPY;

public:
    /**
       Constructor.
       \param schedule related Schedule
    */
    Resource(Schedule* schedule)
    {
        init();
        _schedule = schedule;
    }

    /// \name Accessors (const)
    //@{
    /** Get the manager. */
    clp::Manager* manager() const;

    /** Get the schedule. */
    Schedule*
    schedule() const
    {
        return _schedule;
    }

    /** Get the id. */
    uint_t
    id() const
    {
        return _id;
    }

    /** Get the serial-id. */
    uint_t
    serialId() const
    {
        return _serialId;
    }

    /** Get the name. */
    const std::string&
    name() const
    {
        return _name;
    }

    /** Get the associated object. */
    void*
    object() const
    {
        return _object;
    }

    /** Get visited flag. */
    bool
    visited() const
    {
        return _visited;
    }
    //@}

    /// \name Accessors (non-const)
    //@{
    /** Set the schedule. */
    void
    setSchedule(Schedule* schedule)
    {
        _schedule = schedule;
    }

    /** Set the id. */
    void
    setId(uint_t id)
    {
        _id = id;
    }

    /** Set the serial-id. */
    void
    setSerialId(uint_t serialId)
    {
        _serialId = serialId;
    }

    /** Set the name. */
    void
    setName(const std::string& name)
    {
        _name = name;
    }

    /** Get the associated object. */
    void
    setObject(void* object)
    {
        _object = object;
    }

    /** Get visited flag. */
    void
    setVisited(bool visited)
    {
        _visited = visited;
    }
    //@}

private:
    void init();
    void deInit();

private:
    uint_t _id;
    uint_t _serialId; // position within cls::Schedule::_resourcesArray[]
    Schedule* _schedule;
    std::string _name;
    void* _object;
    bool _visited;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Order Resource%s by id.

   \ingroup cls
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

struct ResIdOrdering
{
    bool
    operator()(const Resource* lhs, const Resource* rhs) const
    {
        return (lhs->id() < rhs->id());
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_END;
