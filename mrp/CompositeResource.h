#ifndef MRP_COMPOSITERESOURCE_H
#define MRP_COMPOSITERESOURCE_H

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <cse/CompositeResource.h>
#include <mrp/Resource.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

MRP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Composite resource.

   \author Joe Zhou
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class CompositeResource : public Resource
{
    UTL_CLASS_DECL(CompositeResource, Resource);

public:
    virtual void copy(const utl::Object& rhs);

    virtual void
    serialize(utl::Stream& stream, uint_t io, uint_t mode = utl::ser_default);

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

    /** create a cse::CompositeResource object. */
    cse::CompositeResource* createCompositeResource();

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

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
