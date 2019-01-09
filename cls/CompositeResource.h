#ifndef CLS_COMPOSITERESOURCE_H
#define CLS_COMPOSITERESOURCE_H

//////////////////////////////////////////////////////////////////////////////

#include <cls/CompositeTimetable.h>
#include <cls/DiscreteResource.h>

//////////////////////////////////////////////////////////////////////////////

CLS_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////

/**
   Composite resource.

   \see DiscreteTimetable
   \author Adam McKee
*/

//////////////////////////////////////////////////////////////////////////////

class CompositeResource : public Resource
{
    UTL_CLASS_DECL(CompositeResource);
public:
    typedef std::set<utl::uint_t> uint_set_t;
    typedef std::vector<DiscreteResource*> dres_vector_t;
    typedef dres_vector_t::const_iterator iterator;
public:
    /** Constructor. */
    CompositeResource(Schedule* schedule)
        : Resource(schedule)
    { init(); }

    /** Add a resource. */
    void add(DiscreteResource* res)
    { _resources.push_back(res); }

    /** Initialize. */
    void initialize();

    /** Add capacity. */
    void add(int min, int max, utl::uint_t resId);

    /** Allocate capacity. */
    void allocate(
        int min,
        int max,
        utl::uint_t cap,
        Activity* act,
        const PreferredResources* pr,
        const clp::IntExp* breakList = nullptr,
        utl::uint_t resId = utl::uint_t_max,
        bool updateDiscrete = true);

    /** Get number of resources. */
    utl::uint_t numResources() const
    { return _resources.size(); }

    /** Get resources begin iterator. */
    iterator begin() const
    { return _resources.begin(); }

    /** Get resources end iterator. */
    iterator end() const
    { return _resources.end(); }

    /** Get timetable. */
    const CompositeTimetable& timetable() const
    { return _timetable; }

    /** Get timetable. */
    CompositeTimetable& timetable()
    { return _timetable; }
private:
    void init() {}
    void deInit() {}
private:
    dres_vector_t _resources;
    CompositeTimetable _timetable;
};

//////////////////////////////////////////////////////////////////////////////

CLS_NS_END;

//////////////////////////////////////////////////////////////////////////////

#endif
