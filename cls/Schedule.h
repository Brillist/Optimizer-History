#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <cls/Activity.h>
#include <cls/Resource.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

class CapExpMgr;
class ResourceCalendarMgr;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Central management of scheduling objects.

   A Schedule is a central store for:
   
   - the set of activities being scheduled
   - the set of resources the activities require (or *may* require) to execute
   - a single CapExpMgr instance
   - a single ResourceCalendarMgr instance

   \see Activity
   \see Resource
   \see CapExpMgr
   \see ResourceCalendarMgr
   \ingroup cls
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class Schedule : public utl::Object
{
    UTL_CLASS_DECL(Schedule, utl::Object);
    UTL_CLASS_NO_COPY;

public:
    using act_set_t = std::set<Activity*, ActIdOrdering>;
    using act_iterator = act_set_t::const_iterator;
    using res_set_t = std::set<Resource*, ResIdOrdering>;
    using res_iterator = res_set_t::const_iterator;

public:
    /**
       Constructor.
       \param mgr related Manager
    */
    Schedule(clp::Manager* mgr);

    /// \name Modification
    //@{
    /** Add the given activity. */
    bool add(Activity* act);

    /** Add the given resource. */
    bool add(Resource* res);

    /** Set serial-ids for resources. */
    void serializeResources();
    //@}

    /// \name Accessors (const)
    //@{
    /** Get the manager. */
    clp::Manager*
    manager() const
    {
        return _mgr;
    }

    /** Get the capacity expression manager. */
    CapExpMgr*
    capExpMgr() const
    {
        return _capExpMgr;
    }

    /** Get the calendar manager. */
    ResourceCalendarMgr*
    calendarMgr() const
    {
        return _calendarMgr;
    }

    /** Get the activities array. */
    Activity* const*
    activitiesArray() const
    {
        return _activitiesArray;
    }

    /** Get the resources array. */
    Resource* const*
    resourcesArray() const
    {
        return _resourcesArray;
    }

    /**
       Get an iterator pointing to the beginning of
       the list of managed activities.
    */
    act_iterator
    activitiesBegin() const
    {
        return _activities.begin();
    }

    /**
       Get an iterator pointing to the end of
       the list of managed activities.
    */
    act_iterator
    activitiesEnd() const
    {
        return _activities.end();
    }

    /**
       Get an iterator pointing to the beginning of
       the list of managed resources.
    */
    res_iterator
    resourcesBegin() const
    {
        return _resources.begin();
    }

    /**
       Get an iterator pointing to the end of
       the list of managed resources.
    */
    res_iterator
    resourcesEnd() const
    {
        return _resources.end();
    }
    //@}

    /// \name Accessors (non-const)
    //@{
    /** Set the manager. */
    void
    setManager(clp::Manager* mgr)
    {
        _mgr = mgr;
    }
    //@}

private:
    void init();
    void deInit();

private:
    clp::Manager* _mgr;
    CapExpMgr* _capExpMgr;
    ResourceCalendarMgr* _calendarMgr;

    // activities
    act_set_t _activities;
    Activity** _activitiesArray;
    size_t _activitiesArraySize;

    // resources
    res_set_t _resources;
    Resource** _resourcesArray;
    size_t _resourcesArraySize;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_END;
