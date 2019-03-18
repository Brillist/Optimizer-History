#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <cls/CompositeTimetable.h>
#include <cls/DiscreteResource.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Composite resource.

   A CompositeResource combines one or more DiscreteResource%s into a "virtual" resource,
   with capacity in each time slot equal to the number of those component DiscreteResource%s that
   are available for work during that time.

   \see IntActivity
   \see CompositeResourceRequirement
   \see CompositeTimetable
   \ingroup cls
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class CompositeResource : public Resource
{
    UTL_CLASS_DECL(CompositeResource, Resource);

public:
    using dres_vector_t = std::vector<DiscreteResource*>;
    using iterator = dres_vector_t::const_iterator;

public:
    /** Constructor. */
    CompositeResource(Schedule* schedule)
        : Resource(schedule)
    {
        init();
    }

    /** Add a resource. */
    void
    add(DiscreteResource* res)
    {
        _resources.push_back(res);
    }

    /// \name Accessors (const)
    //@{
    /** Get number of resources. */
    uint_t
    numResources() const
    {
        return _resources.size();
    }

    /** Get resources begin iterator. */
    iterator
    begin() const
    {
        return _resources.begin();
    }

    /** Get resources end iterator. */
    iterator
    end() const
    {
        return _resources.end();
    }

    /** Get timetable. */
    const CompositeTimetable&
    timetable() const
    {
        return _timetable;
    }
    //@}

    /// \name Accessors (non-const)
    //@{
    /** Get timetable. */
    CompositeTimetable&
    timetable()
    {
        return _timetable;
    }
    //@}

    /// \name Capacity Provision and Allocation
    //@{
    /**
       Initialize the CompositeTimetable by adding capacity for each component DiscreteResource.
    */
    void initialize();

    /**
       Add capacity.
       \param min start of capacity provision span
       \param max end of capacity provision span
       \param resId id of DiscreteResource providing capacity
    */
    void add(int min, int max, uint_t resId);

    /**
       Allocate capacity.
       \param min start of allocation span
       \param max end of allocation span
       \param cap allocated capacity
       \param act responsible Activity
       \param pr preferred resources
       \param breakList break list to be observed (default: use the activity's breakList)
       \param resId id of DiscreteResource to allocate from (default: use PreferredResources)
       \param updateDiscrete also allocate capacity in DiscreteResource%s? (default: true)
       \see Activity::breakList
       \see CompositeTimetable::allocate
       \see DiscreteResource::allocate
    */
    void allocate(int min,
                  int max,
                  uint_t cap,
                  Activity* act,
                  const PreferredResources* pr,
                  const clp::IntExp* breakList = nullptr,
                  uint_t resId = uint_t_max,
                  bool updateDiscrete = true);
    //@}

private:
    void
    init()
    {
    }
    void
    deInit()
    {
    }

private:
    dres_vector_t _resources;
    CompositeTimetable _timetable;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_END;
