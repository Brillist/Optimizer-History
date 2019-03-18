#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Preferred resources in a CompositeResourceRequirement.

   \see CompositeResourceRequirement
   \ingroup cls
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class PreferredResources : public utl::Object
{
    UTL_CLASS_DECL(PreferredResources, utl::Object);
    UTL_CLASS_DEFID;

public:
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
