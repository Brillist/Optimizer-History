#ifndef MRP_RESOURCE_H
#define MRP_RESOURCE_H

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

MRP_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////

/**
   Resource.

   \author Adam McKee
*/

//////////////////////////////////////////////////////////////////////////////

class Resource : public utl::Object
{
    UTL_CLASS_DECL(Resource);
public:
    virtual void copy(const utl::Object& rhs);

    virtual int compare(const utl::Object& rhs) const;

    virtual void serialize(
        utl::Stream& stream,
        utl::uint_t io,
        utl::uint_t mode = utl::ser_default);

    /// \name Accessors
    //@{
    /** Id. */
    utl::uint_t id() const
    { return _id; }

    /** Id. */
    utl::uint_t& id()
    { return _id; }

    /** Name. */
    const std::string& name() const
    { return _name; }

    /** Name. */
    std::string& name()
    { return _name; }
    //@}

    utl::String toString() const;
protected:
    utl::uint_t _id;
    std::string _name;
private:
    void init();
    void deInit();
};


//////////////////////////////////////////////////////////////////////////////

struct ResourceOrderingIncId 
    : public std::binary_function<Resource*,Resource*,bool>
{
    bool operator()(Resource* lhs, Resource* rhs) const
    {
        return (lhs->id() < rhs->id());
    }
};

//////////////////////////////////////////////////////////////////////////////

typedef std::set<Resource*, ResourceOrderingIncId> resource_set_id_t;

//////////////////////////////////////////////////////////////////////////////

MRP_NS_END;

//////////////////////////////////////////////////////////////////////////////

#endif
