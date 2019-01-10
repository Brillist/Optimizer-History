#ifndef MRP_BOM_H
#define MRP_BOM_H

////////////////////////////////////////////////////////////////////////////////////////////////////

/* #include <libutl/String.h> */

////////////////////////////////////////////////////////////////////////////////////////////////////

MRP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

class Item;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   BOM.

   \author Joe Zhou
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class BOM : public utl::Object
{
    UTL_CLASS_DECL(BOM);

public:
    virtual void copy(const utl::Object& rhs);

    virtual void
    serialize(utl::Stream& stream, utl::uint_t io, utl::uint_t mode = utl::ser_default);

    /// \name Accessors
    //@{
    /** Item. */
    const Item*
    item() const
    {
        return _item;
    }

    /** Item. */
    Item*
    item()
    {
        return _item;
    }

    /** Child Item. */
    const Item*
    childItem() const
    {
        return _childItem;
    }

    /** Child Item. */
    Item*
    childItem()
    {
        return _childItem;
    }

    /** Quantity. */
    utl::uint_t
    quantity() const
    {
        return _quantity;
    }

    /** Quantity. */
    utl::uint_t&
    quantity()
    {
        return _quantity;
    }
    //@}

    /** reset item. */
    void setItem(Item* item, bool owner = false);

    /** reset child item.*/
    void setChildItem(Item* childItem, bool owner = false);

    utl::String toString() const;

private:
    void init();
    void deInit();

private:
    Item* _item;
    Item* _childItem;
    utl::uint_t _quantity;

    bool _itemOwner;
    bool _childItemOwner;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

typedef std::vector<BOM*> bom_vector_t;

////////////////////////////////////////////////////////////////////////////////////////////////////

MRP_NS_END;

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
