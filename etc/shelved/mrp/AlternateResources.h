#pragma once

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
    UTL_CLASS_DECL(AlternateResources, Resource);

public:
    virtual void copy(const utl::Object& rhs);

    virtual void serialize(utl::Stream& stream, uint_t io, uint_t mode = utl::ser_default);

    /// \name Accessors
    //@{
    /** Resource group-id. */
    uint_t
    resourceGroupId() const
    {
        return _resourceGroupId;
    }

    /** Resource group-id. */
    uint_t&
    resourceGroupId()
    {
        return _resourceGroupId;
    }
    //@}

private:
    void
    init()
    {
        _resourceGroupId = uint_t_max;
    }
    void
    deInit()
    {
    }

private:
    uint_t _resourceGroupId;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

MRP_NS_END;
