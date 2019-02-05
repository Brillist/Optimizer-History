#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <mrp/Item.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

MRP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Purchased item.

   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class PurchaseItem : public Item
{
    UTL_CLASS_DECL(PurchaseItem, Item);

public:
    virtual void copy(const utl::Object& rhs);

    virtual void serialize(utl::Stream& stream, uint_t io, uint_t mode = utl::ser_default);

    /// \name Accessors
    //@{
    /** Lead time. */
    uint_t
    leadTime() const
    {
        return _leadTime;
    }

    /** Lead time. */
    uint_t&
    leadTime()
    {
        return _leadTime;
    }

    /** Min purchase quantity. */
    uint_t
    eoq() const
    {
        return _eoq;
    }

    /** Min purchase quantity. */
    uint_t&
    eoq()
    {
        return _eoq;
    }
    //@}

    String toString() const;

private:
    void init();
    void
    deInit()
    {
    }

private:
    uint_t _leadTime;
    uint_t _eoq;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

MRP_NS_END;
