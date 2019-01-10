#ifndef CSE_ITEMREQUIREMENT_H
#define CSE_ITEMREQUIREMENT_H

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <cls/PreferredResources.h>
#include <cls/ResourceCapPts.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Item requirement.

   \author Joe Zhou
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class ItemRequirement : public utl::Object
{
    UTL_CLASS_DECL(ItemRequirement);

public:
    /** Copy another instance. */
    virtual void copy(const utl::Object& rhs);

    virtual void
    serialize(utl::Stream& stream, utl::uint_t io, utl::uint_t mode = utl::ser_default);

    /// \name Accessors
    //@{
    /** Get the resource id. */
    utl::uint_t
    itemId() const
    {
        return _itemId;
    }

    /** Get the resource id. */
    utl::uint_t&
    itemId()
    {
        return _itemId;
    }

    /** Get required capacity. */
    utl::uint_t
    quantity() const
    {
        return _quantity;
    }

    /** Get required capacity. */
    utl::uint_t&
    quantity()
    {
        return _quantity;
    }
    //@}

private:
    void init();
    void deInit();

private:
    utl::uint_t _itemId;
    utl::uint_t _quantity;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

typedef std::vector<ItemRequirement*> itemreq_vector_t;

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
