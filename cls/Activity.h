#ifndef CLS_ACTIVITY_H
#define CLS_ACTIVITY_H

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

   An activity has the following important properties:

   - <b>start time</b> : when the activity begins execution
   - <b>duration</b> : the total time required to complete the activity
   - <b>end time</b> : when the activity completes execution

   The following relationship is pretty obvious:

   \code
   startTime + duration = endTime
   \endcode

   Precedence constraints involving activities are easy to formulate.
   For example, if an activity A is required to complete execution before
   another activity B can begin execution, this is easily expressed as:

   \code
   A.endTime <= B.startTime
   \endcode

   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class Activity : public utl::Object
{
    UTL_CLASS_DECL(Activity);
    UTL_CLASS_NO_COPY;

public:
    typedef std::set<utl::uint_t> uint_set_t;

public:
    /** Constructor. */
    Activity(Schedule* schedule);

    /** Get a human-readable string representation. */
    virtual utl::String toString() const;

    /** Get the manager associated with the invoking activity's schedule. */
    clp::Manager* manager();

    /** Get the parent schedule. */
    Schedule*
    schedule() const
    {
        return _schedule;
    }

    /** Set the parent schedule. */
    Schedule*&
    schedule()
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
    utl::uint_t
    id() const
    {
        return _id;
    }

    /** Get the id. */
    utl::uint_t&
    id()
    {
        return _id;
    }

    /** Get the serial-id. */
    utl::uint_t
    serialId() const
    {
        return _serialId;
    }

    /** Get the serial-id. */
    utl::uint_t&
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

    /** Enable/disable debugging. */
    void setDebugFlag(bool debugFlag);

    /** Get the start range-var. */
    const clp::RangeVar&
    start() const
    {
        return _start;
    }

    /** Get the start range-var. */
    clp::RangeVar&
    start()
    {
        return _start;
    }

    /** Get the end range-var. */
    const clp::RangeVar&
    end() const
    {
        return _end;
    }

    /** Get the end range-var. */
    clp::RangeVar&
    end()
    {
        return _end;
    }

    /** Get the owner. */
    void*
    owner() const
    {
        return _owner;
    }

    /** Get the owner. */
    void*&
    owner()
    {
        return _owner;
    }

    /** Get allocated flag. */
    bool
    allocated() const
    {
        return _allocated;
    }

    /** Get allocated flag. */
    bool&
    allocated()
    {
        return _allocated;
    }

    /// \name Bound Accessors
    //@{
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

    /** Get the earliest-start bound. */
    int
    es() const
    {
        return _start.lb();
    }

    /** Get the latest-start. */
    int
    ls() const
    {
        return _start.ub();
    }

    /** Get the earliest-finish. */
    int
    ef() const
    {
        return _end.lb();
    }

    /** Get the latest-finish. */
    int
    lf() const
    {
        return _end.ub();
    }

    /** Get all discrete resource the activity may use */
    const uint_set_t&
    allResIds() const
    {
        return _allResIds;
    }

    /** Get all discrete resource the activity may use */
    uint_set_t&
    allResIds()
    {
        return _allResIds;
    }

    /** Select a resource. */
    virtual bool
    selectResource(utl::uint_t resId)
    {
        return false;
    }
    //@}
private:
    void init();
    void deInit();

private:
    utl::uint_t _id;
    utl::uint_t _serialId;
    Schedule* _schedule;
    clp::RangeVar _start;
    clp::RangeVar _end;
    std::string _name;
    void* _owner;
    bool _allocated;
    uint_set_t _allResIds; //discrete resources only
};

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Order activities by id.

   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

struct ActIdOrdering : public std::binary_function<Activity*, Activity*, bool>
{
    bool
    operator()(const Activity* lhs, const Activity* rhs) const
    {
        return (lhs->id() < rhs->id());
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Order activities by earliest-start-time.

   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

struct ActESordering : public std::binary_function<Activity*, Activity*, bool>
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

typedef std::set<Activity*, ActESordering> act_set_es_t;
typedef std::vector<Activity*> act_vect_t;

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_END;

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
