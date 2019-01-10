#ifndef MRP_ALTERNATERESOURCES_H
#define MRP_ALTERNATERESOURCES_H

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <mrp/Resource.h>
#include <cse/ResourceGroup.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

MRP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Alternate resources.

   \author Joe Zhou
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class AlternateResources : public Resource
{
    UTL_CLASS_DECL(AlternateResources);

public:
    virtual void copy(const utl::Object& rhs);

    virtual void
    serialize(utl::Stream& stream, utl::uint_t io, utl::uint_t mode = utl::ser_default);

    /// \name Accessors
    //@{
    /** Resource group-id. */
    utl::uint_t
    resourceGroupId() const
    {
        return _resourceGroupId;
    }

    /** Resource group-id. */
    utl::uint_t&
    resourceGroupId()
    {
        return _resourceGroupId;
    }
    //@}

private:
    void
    init()
    {
        _resourceGroupId = utl::uint_t_max;
    }
    void
    deInit()
    {
    }

private:
    utl::uint_t _resourceGroupId;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

MRP_NS_END;

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
