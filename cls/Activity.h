#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <clp/IntExp.h>
#include <clp/RangeVar.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

class Schedule;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Activity to be executed (abstract).

   Important properties:

   - start and end times represented RangeVar%s, bound to a single value when the
     activity is scheduled
   - a *break list* represented as an IntVar whose domain includes time slots where all of the
     required resources are available for work

   \see PtActivity
   \see BrkActivity
   \see IntActivity
   \ingroup cls
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class Activity : public utl::Object
{
    UTL_CLASS_DECL(Activity, utl::Object);
    UTL_CLASS_NO_COPY;

public:
    /**
       Constructor.
       \param schedule related Schedule
    */
    Activity(Schedule* schedule);

    virtual String toString() const;

    /**
       Select a resource.
       DEPRECATED: what if two requirements share the same resource?
    */
    virtual bool
    selectResource(uint_t resId)
    {
        return false;
    }

    /// Misc. Accessors (const)
    //@{
    /** Get the Manager associated with the parent Schedule. */
    clp::Manager* manager();

    /** Get the parent Schedule . */
    Schedule*
    schedule() const
    {
        return _schedule;
    }

    /** Get the break-list. */
    virtual clp::IntExp*
    breakList() const
    {
        return nullptr;
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

    /** Get the owner. */
    utl::Object*
    owner() const
    {
        return _owner;
    }

    /** Get allocated flag. */
    bool
    allocated() const
    {
        return _allocated;
    }

    /** Get the set of alternative discrete resources. */
    const uint_set_t&
    allResIds() const
    {
        return _allResIds;
    }
    //@}

    /// \name Misc. Accessors (non-const)
    //@{
    /** Set the parent schedule. */
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

    /** Set owning Object. */
    void
    setOwner(utl::Object* owner)
    {
        _owner = owner;
    }

    /** Get non-const allocated flag reference. */
    bool&
    allocated()
    {
        return _allocated;
    }

    /** Get the set of alternative discrete resources. */
    uint_set_t&
    allResIds()
    {
        return _allResIds;
    }

    /** Enable/disable debugging. */
    void setDebugFlag(bool debugFlag);
    //@}

    /// \name Bound Accessors (const)
    //@{
    /** Get the start range-var. */
    const clp::RangeVar&
    start() const
    {
        return _start;
    }

    /** Get the end range-var. */
    const clp::RangeVar&
    end() const
    {
        return _end;
    }

    /** Get the earliest-start bound. */
    const clp::ConstrainedBound&
    esBound() const
    {
        return _start.lowerBound();
    }

    /** Get the latest-start bound. */
    const clp::ConstrainedBound&
    lsBound() const
    {
        return _start.upperBound();
    }

    /** Get the earliest-finish bound. */
    const clp::ConstrainedBound&
    efBound() const
    {
        return _end.lowerBound();
    }

    /** Get the latest-finish bound. */
    const clp::ConstrainedBound&
    lfBound() const
    {
        return _end.upperBound();
    }

    /** Get the earliest-start bound's current value. */
    int
    es() const
    {
        return _start.lb();
    }

    /** Get the latest-start bound's current value. */
    int
    ls() const
    {
        return _start.ub();
    }

    /** Get the earliest-finish bound's current value. */
    int
    ef() const
    {
        return _end.lb();
    }

    /** Get the latest-finish bound's current value. */
    int
    lf() const
    {
        return _end.ub();
    }
    //@}

    /// \name Bound Accessors (non-const)
    //@{
    /** Get the start RangeVar. */
    clp::RangeVar&
    start()
    {
        return _start;
    }

    /** Get the end RangeVar. */
    clp::RangeVar&
    end()
    {
        return _end;
    }

    /** Get the earliest-start bound. */
    clp::ConstrainedBound&
    esBound()
    {
        return _start.lowerBound();
    }

    /** Get the latest-start bound. */
    clp::ConstrainedBound&
    lsBound()
    {
        return _start.upperBound();
    }

    /** Get the earliest-finish bound. */
    clp::ConstrainedBound&
    efBound()
    {
        return _end.lowerBound();
    }

    /** Get the latest-finish bound. */
    clp::ConstrainedBound&
    lfBound()
    {
        return _end.upperBound();
    }
    //@}
private:
    void init();
    void deInit();

private:
    uint_t _id;
    uint_t _serialId;
    Schedule* _schedule;
    clp::RangeVar _start;
    clp::RangeVar _end;
    std::string _name;
    utl::Object* _owner;
    bool _allocated;
    uint_set_t _allResIds;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Order Activity objects by id.

   \ingroup cls
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

struct ActIdOrdering
{
    bool
    operator()(const Activity* lhs, const Activity* rhs) const
    {
        return (lhs->id() < rhs->id());
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Order Activity objects by earliest-start-time.

   \ingroup cls
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

struct ActESordering
{
    bool
    operator()(const Activity* lhs, const Activity* rhs) const
    {
        if (lhs->es() != rhs->es())
            return (lhs->es() < rhs->es());
        // fall-back to id-ordering (for repeatability)
        return (lhs->id() < rhs->id());
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////

using act_set_es_t = std::set<Activity*, ActESordering>;
using act_vect_t = std::vector<Activity*>;

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_END;
