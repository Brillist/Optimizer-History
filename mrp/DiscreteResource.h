#ifndef MRP_DISCRETERESOURCE_H
#define MRP_DISCRETERESOURCE_H

//////////////////////////////////////////////////////////////////////////////

/* #include <cls/ResourceCalendar.h> */
#include <cse/DiscreteResource.h>
/* #include <cse/ResourceCost.h> */
#include <mrp/Resource.h>
#include <mrp/SetupGroup.h>

//////////////////////////////////////////////////////////////////////////////

MRP_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////

/**
   Discrete resource.

   \author Adam McKee
*/

//////////////////////////////////////////////////////////////////////////////

class DiscreteResource : public Resource
{
    UTL_CLASS_DECL(DiscreteResource);
public:
    virtual void copy(const utl::Object& rhs);

    virtual void serialize(
        utl::Stream& stream,
        utl::uint_t io,
        utl::uint_t mode = utl::ser_default);

    /// \name Accessors
    //@{
    /** Capacity. */
    utl::uint_t capacity() const
    { return _capacity; }

    /** Capacity. */
    utl::uint_t& capacity()
    { return _capacity; }

    /** Setup group. */
    const SetupGroup* setupGroup() const
    { return _setupGroup; }

    /** Setup group. */
    SetupGroup* setupGroup()
    { return _setupGroup; }
    //@}

    /** reset setup group. */
    void setSetupGroup(SetupGroup* group, bool owner = false);

    /** create a cse::DiscreteResource object. */
    cse::DiscreteResource* createDiscreteResource();

    utl::String toString() const;
private:
    void init();
    void deInit();
private:
    utl::uint_t _capacity;
    SetupGroup* _setupGroup;
    cse::ResourceCost* _cost;
    cls::ResourceCalendar* _defaultCalendar;
    cls::ResourceCalendar* _detailedCalendar;

    bool _setupGroupOwner;
};

//////////////////////////////////////////////////////////////////////////////

MRP_NS_END;

//////////////////////////////////////////////////////////////////////////////

#endif
