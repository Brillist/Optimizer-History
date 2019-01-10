#ifndef MRP_RESOURCEGROUP_H
#define MRP_RESOURCEGROUP_H

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <cse/ResourceGroup.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

MRP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Resource Group.

   \author Joe Zhou
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class ResourceGroup : public utl::Object
{
    UTL_CLASS_DECL(ResourceGroup);

public:
    virtual void copy(const utl::Object& rhs);

    virtual int compare(const utl::Object& rhs) const;

    virtual void
    serialize(utl::Stream& stream, utl::uint_t io, utl::uint_t mode = utl::ser_default);

    /// \name Accessors
    //@{
    /** Id. */
    utl::uint_t
    id() const
    {
        return _id;
    }

    /** Id. */
    utl::uint_t&
    id()
    {
        return _id;
    }

    /** Name. */
    const std::string&
    name() const
    {
        return _name;
    }

    /** Name. */
    std::string&
    name()
    {
        return _name;
    }

    /** Member resource ids. */
    const lut::uint_set_t&
    resIds() const
    {
        return _resIds;
    }

    /** Member resource ids. */
    lut::uint_set_t&
    resIds()
    {
        return _resIds;
    }
    //@}

    cse::ResourceGroup* createResourceGroup();

    utl::String toString() const;

protected:
    utl::uint_t _id;
    std::string _name;
    lut::uint_set_t _resIds;

private:
    void init();
    void deInit();
};

////////////////////////////////////////////////////////////////////////////////////////////////////

struct ResourceGroupOrderingIncId
    : public std::binary_function<ResourceGroup*, ResourceGroup*, bool>
{
    bool
    operator()(ResourceGroup* lhs, ResourceGroup* rhs) const
    {
        return (lhs->id() < rhs->id());
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////

typedef std::set<ResourceGroup*, ResourceGroupOrderingIncId> resourcegroup_set_id_t;

////////////////////////////////////////////////////////////////////////////////////////////////////

MRP_NS_END;

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
