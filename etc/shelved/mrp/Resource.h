#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////

MRP_NS_BEGIN;

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
    virtual void copy(const utl::Object& rhs);

    virtual int compare(const utl::Object& rhs) const;

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
    //@}

    String toString() const;

protected:
    uint_t _id;
    std::string _name;

private:
    void init();
    void deInit();
};

////////////////////////////////////////////////////////////////////////////////////////////////////

struct ResourceOrderingIncId
{
    bool
    operator()(Resource* lhs, Resource* rhs) const
    {
        return (lhs->id() < rhs->id());
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////

typedef std::set<Resource*, ResourceOrderingIncId> resource_set_id_t;

////////////////////////////////////////////////////////////////////////////////////////////////////

MRP_NS_END;
