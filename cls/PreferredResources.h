#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   List of preferred resources.

   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class PreferredResources : public utl::Object
{
    UTL_CLASS_DECL(PreferredResources, utl::Object);
    UTL_CLASS_DEFID;

public:
    typedef std::vector<uint_t> uint_vector_t;

public:
    /** Copy another instance. */
    virtual void copy(const utl::Object& rhs);

    virtual void serialize(utl::Stream& stream, uint_t io, uint_t mode = utl::ser_default);

    /** Get the list of resource ids. */
    const uint_vector_t&
    resIds() const
    {
        return _resIds;
    }

    /** Get the list of resource ids. */
    uint_vector_t&
    resIds()
    {
        return _resIds;
    }

private:
    uint_vector_t _resIds;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_END;
