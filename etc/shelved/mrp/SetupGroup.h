#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <mrp/Setup.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

MRP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Setup group.

   All resources that generally have a setup requirement must belong to a 
   setup group. A setup group can be used by more than one resources. It is
   like a setup matrix for resources.

   A setup group should have more than one setup states (setups).

   \author Joe Zhou
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class SetupGroup : public utl::Object
{
    UTL_CLASS_DECL(SetupGroup, utl::Object);

public:
    /** Constructor. */
    SetupGroup(uint_t id)
    {
        _id = id;
    }

    virtual void copy(const utl::Object& rhs);

    virtual void serialize(utl::Stream& stream, uint_t io, uint_t mode = utl::ser_default);

    /// \name Accessors
    //@{
    /** Id. */
    uint_t
    id() const
    {
        return _id;
    }

    /** Id. */
    uint_t&
    id()
    {
        return _id;
    }
    //@}

    String toString() const;

private:
    void
    init()
    {
    }
    void deInit();

private:
    uint_t _id;
    setup_set_id_t _setups;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

struct SetupGroupOrderingIncId
{
    bool operator()(SetupGroup* lhs, SetupGroup* rhs) const;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

typedef std::set<SetupGroup*, SetupGroupOrderingIncId> setupgroup_set_id_t;

////////////////////////////////////////////////////////////////////////////////////////////////////

MRP_NS_END;
