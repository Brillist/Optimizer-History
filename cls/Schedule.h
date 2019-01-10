#ifndef CLS_SCHEDULE_H
#define CLS_SCHEDULE_H

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
   Manage activities, resources, resource constraints.

   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class Schedule : public utl::Object
{
    UTL_CLASS_DECL(Schedule);
    UTL_CLASS_NO_COPY;

public:
    typedef std::set<Activity*, ActIdOrdering> act_set_t;
    typedef act_set_t::iterator act_iterator;
    typedef act_set_t::const_iterator act_const_iterator;

    typedef std::set<Resource*, ResIdOrdering> res_set_t;
    typedef res_set_t::iterator res_iterator;
    typedef res_set_t::const_iterator res_const_iterator;

public:
    /** Constructor. */
    Schedule(clp::Manager* mgr);

    /** Get the manager. */
    clp::Manager*
    manager() const
    {
        return _mgr;
    }

    /** Get the manager. */
    clp::Manager*&
    manager()
    {
        return _mgr;
    }

    /** Add the given activity. */
    bool add(Activity* act);

    /** Add the given resource. */
    bool add(Resource* res);

    /** Set serial-ids for resources. */
    void serializeResources();

    /// \name Accessors
    //@{
    /** Get the capacity-expression manager. */
    CapExpMgr*
    capExpMgr()
    {
        return _capExpMgr;
    }

    /** Get the calendar manager. */
    ResourceCalendarMgr*
    calendarMgr()
    {
        return _calendarMgr;
    }

    /** Get the activities array. */
    Activity**
    activitiesArray() const
    {
        return _activitiesArray;
    }

    /** Get the resources array. */
    Resource**
    resourcesArray() const
    {
        return _resourcesArray;
    }
    //@}

    /// \name Iterators
    //@{
    /**
       Get an iterator pointing to the beginning of
       the list of managed activities.
    */
    act_const_iterator
    activitiesBegin() const
    {
        return _activities.begin();
    }

    /**
       Get an iterator pointing to the beginning of
       the list of managed activities.
    */
    act_iterator
    activitiesBegin()
    {
        return _activities.begin();
    }

    /**
       Get an iterator pointing to the end of
       the list of managed activities.
    */
    act_const_iterator
    activitiesEnd() const
    {
        return _activities.end();
    }

    /**
       Get an iterator pointing to the end of
       the list of managed activities.
    */
    act_iterator
    activitiesEnd()
    {
        return _activities.end();
    }

    /**
       Get an iterator pointing to the beginning of
       the list of managed resources.
    */
    res_const_iterator
    resourcesBegin() const
    {
        return _resources.begin();
    }

    /**
       Get an iterator pointing to the beginning of
       the list of managed resources.
    */
    res_iterator
    resourcesBegin()
    {
        return _resources.begin();
    }

    /**
       Get an iterator pointing to the end of
       the list of managed resources.
    */
    res_const_iterator
    resourcesEnd() const
    {
        return _resources.end();
    }

    /**
       Get an iterator pointing to the end of
       the list of managed resources.
    */
    res_iterator
    resourcesEnd()
    {
        return _resources.end();
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

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
