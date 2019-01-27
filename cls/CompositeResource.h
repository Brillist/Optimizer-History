#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <cls/CompositeTimetable.h>
#include <cls/DiscreteResource.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Composite resource.

   \see DiscreteTimetable
   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class CompositeResource : public Resource
{
    UTL_CLASS_DECL(CompositeResource, Resource);

public:
    typedef std::set<uint_t> uint_set_t;
    typedef std::vector<DiscreteResource*> dres_vector_t;
    typedef dres_vector_t::const_iterator iterator;

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

    /** Initialize. */
    void initialize();

    /** Add capacity. */
    void add(int min, int max, uint_t resId);

    /** Allocate capacity. */
    void allocate(int min,
                  int max,
                  uint_t cap,
                  Activity* act,
                  const PreferredResources* pr,
                  const clp::IntExp* breakList = nullptr,
                  uint_t resId = uint_t_max,
                  bool updateDiscrete = true);

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

    /** Get timetable. */
    CompositeTimetable&
    timetable()
    {
        return _timetable;
    }

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
