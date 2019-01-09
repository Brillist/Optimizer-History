#ifndef MRP_COMPOSITERESOURCE_H
#define MRP_COMPOSITERESOURCE_H

//////////////////////////////////////////////////////////////////////////////

#include <cse/CompositeResource.h>
#include <mrp/Resource.h>

//////////////////////////////////////////////////////////////////////////////

MRP_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////

/**
   Composite resource.

   \author Joe Zhou
*/

//////////////////////////////////////////////////////////////////////////////

class CompositeResource : public Resource
{
    UTL_CLASS_DECL(CompositeResource);
public:
    virtual void copy(const utl::Object& rhs);

    virtual void serialize(
        utl::Stream& stream,
        utl::uint_t io,
        utl::uint_t mode = utl::ser_default);

    /// \name Accessors
    //@{
    /** Resource group-id. */
    utl::uint_t resourceGroupId() const
    { return _resourceGroupId; }

    /** Resource group-id. */
    utl::uint_t& resourceGroupId()
    { return _resourceGroupId; }
    //@}

    /** create a cse::CompositeResource object. */
    cse::CompositeResource* createCompositeResource();
private:
    void init()
    { _resourceGroupId = utl::uint_t_max; }
    void deInit() {}
private:
    utl::uint_t _resourceGroupId;
};

//////////////////////////////////////////////////////////////////////////////

MRP_NS_END;

//////////////////////////////////////////////////////////////////////////////

#endif
