#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <cls/CompositeResource.h>
#include <cse/Resource.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Composite resource.

   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class CompositeResource : public Resource
{
    UTL_CLASS_DECL(CompositeResource, Resource);

public:
    /** Constructor. */
    CompositeResource(uint_t id, const std::string& name)
        : Resource(id, name)
    {
        init();
    }

    /** Copy another instance. */
    virtual void copy(const utl::Object& rhs);

    virtual void
    serialize(utl::Stream& stream, uint_t io, uint_t mode = utl::ser_default);

    /** Get the resource-group id. */
    uint_t
    resourceGroupId() const
    {
        return _resGroupId;
    }

    /** Get the resource-group id. */
    uint_t&
    resourceGroupId()
    {
        return _resGroupId;
    }

    /** Get the cls-resource. */
    cls::CompositeResource*
    clsResource() const
    {
        ASSERTD(dynamic_cast<cls::CompositeResource*>(_clsResource) != nullptr);
        return (cls::CompositeResource*)_clsResource;
    }

    /** Get the cls-resource. */
    cls::Resource*&
    clsResource()
    {
        return _clsResource;
    }

private:
    void init();
    void deInit();

private:
    uint_t _resGroupId;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;
