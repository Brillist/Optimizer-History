#ifndef MRP_PURCHASEITEM_H
#define MRP_PURCHASEITEM_H

//////////////////////////////////////////////////////////////////////////////

#include <mrp/Item.h>

//////////////////////////////////////////////////////////////////////////////

MRP_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////

/**
   Purchased item.

   \author Adam McKee
*/

//////////////////////////////////////////////////////////////////////////////

class PurchaseItem : public Item
{
    UTL_CLASS_DECL(PurchaseItem);
public:
    virtual void copy(const utl::Object& rhs);

    virtual void serialize(
        utl::Stream& stream,
        utl::uint_t io,
        utl::uint_t mode = utl::ser_default);

    /// \name Accessors
    //@{
    /** Lead time. */
    utl::uint_t leadTime() const
    { return _leadTime; }

    /** Lead time. */
    utl::uint_t& leadTime()
    { return _leadTime; }

    /** Min purchase quantity. */
    utl::uint_t eoq() const
    { return _eoq; }

    /** Min purchase quantity. */
    utl::uint_t& eoq()
    { return _eoq; }
    //@}

    utl::String toString() const;
private:
    void init();
    void deInit() {}
private:
    utl::uint_t _leadTime;
    utl::uint_t _eoq;
};

//////////////////////////////////////////////////////////////////////////////

MRP_NS_END;

//////////////////////////////////////////////////////////////////////////////

#endif
