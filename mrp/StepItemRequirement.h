#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////

MRP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Item requirement belonging to a ProcessStep.

   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class StepItemRequirement : public utl::Object
{
    UTL_CLASS_DECL(StepItemRequirement, utl::Object);

public:
    virtual void copy(const utl::Object& rhs);

    virtual void
    serialize(utl::Stream& stream, uint_t io, uint_t mode = utl::ser_default);

    /// \name Accessors
    //@{
    /** Item id. */
    uint_t
    itemId() const
    {
        return _itemId;
    }

    /** Item id. */
    uint_t&
    itemId()
    {
        return _itemId;
    }

    /** Quantity. */
    uint_t
    quantity() const
    {
        return _quantity;
    }

    /** Quantity. */
    uint_t&
    quantity()
    {
        return _quantity;
    }
    //@}

    String toString() const;

private:
    void
    init()
    {
        _itemId = _quantity = uint_t_max;
    }
    void
    deInit()
    {
    }

private:
    uint_t _itemId;
    uint_t _quantity;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

MRP_NS_END;
