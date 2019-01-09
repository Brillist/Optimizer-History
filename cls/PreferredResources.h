#ifndef CLS_PREFERREDRESOURCES_H
#define CLS_PREFERREDRESOURCES_H

//////////////////////////////////////////////////////////////////////////////

CLS_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////

/**
   List of preferred resources.

   \author Adam McKee
*/

//////////////////////////////////////////////////////////////////////////////

class PreferredResources : public utl::Object
{
    UTL_CLASS_DECL(PreferredResources);
    UTL_CLASS_DEFID;
public:
    typedef std::vector<utl::uint_t> uint_vector_t;
public:
    /** Copy another instance. */
    virtual void copy(const utl::Object& rhs);

    virtual void serialize(
        utl::Stream& stream,
        utl::uint_t io,
        utl::uint_t mode = utl::ser_default);

    /** Get the list of resource ids. */
    const uint_vector_t& resIds() const
    { return _resIds; }

    /** Get the list of resource ids. */
    uint_vector_t& resIds()
    { return _resIds; }
private:
    uint_vector_t _resIds;
};

//////////////////////////////////////////////////////////////////////////////

CLS_NS_END;

//////////////////////////////////////////////////////////////////////////////

#endif
