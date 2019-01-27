#ifndef CSE_RESOURCE_H
#define CSE_RESOURCE_H

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <cls/Resource.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Resource.

   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class Resource : public utl::Object
{
    UTL_CLASS_DECL(Resource, utl::Object);

public:
    /** Constructor. */
    Resource(uint_t id, const std::string& name)
    {
        init();
        _id = id;
        _name = name;
    }

    /** Copy another instance. */
    virtual void copy(const utl::Object& rhs);

    virtual void
    serialize(utl::Stream& stream, uint_t io, uint_t mode = utl::ser_default);

    /** Get the id. */
    uint_t
    id() const
    {
        return _id;
    }

    /** Get the id. */
    uint_t&
    id()
    {
        return _id;
    }

    /** Get the serial id. */
    uint_t
    serialId() const
    {
        return _serialId;
    }

    /** Get the serial id. */
    uint_t&
    serialId()
    {
        return _serialId;
    }

    /** Get the name. */
    const std::string&
    name() const
    {
        return _name;
    }

    /** Get the name. */
    std::string&
    name()
    {
        return _name;
    }

    /** Get the cls-resource. */
    cls::Resource*
    clsResource() const
    {
        return _clsResource;
    }

    /** Get the cls-resource. */
    cls::Resource*&
    clsResource()
    {
        return _clsResource;
    }

protected:
    uint_t _id;
    uint_t _serialId;
    std::string _name;
    cls::Resource* _clsResource;

private:
    void init();
    void deInit();
};

////////////////////////////////////////////////////////////////////////////////////////////////////
/// ResourceIdOrdering /////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Order Resource objects by id.

   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

struct ResourceIdOrdering : public std::binary_function<Resource*, Resource*, bool>
{
    /** Compare two individuals. */
    bool
    operator()(const Resource* lhs, const Resource* rhs) const
    {
        return (lhs->id() < rhs->id());
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////

typedef std::vector<Resource*> res_vector_t;
typedef std::set<Resource*, ResourceIdOrdering> res_set_id_t;

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
