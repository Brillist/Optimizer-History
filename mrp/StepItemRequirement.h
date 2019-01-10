#ifndef MRP_STEPITEMREQUIREMENT_H
#define MRP_STEPITEMREQUIREMENT_H

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
    UTL_CLASS_DECL(StepItemRequirement);

public:
    virtual void copy(const utl::Object& rhs);

    virtual void
    serialize(utl::Stream& stream, utl::uint_t io, utl::uint_t mode = utl::ser_default);

    /// \name Accessors
    //@{
    /** Item id. */
    utl::uint_t
    itemId() const
    {
        return _itemId;
    }

    /** Item id. */
    utl::uint_t&
    itemId()
    {
        return _itemId;
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

    utl::String toString() const;

private:
    void
    init()
    {
        _itemId = _quantity = utl::uint_t_max;
    }
    void
    deInit()
    {
    }

private:
    utl::uint_t _itemId;
    utl::uint_t _quantity;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

MRP_NS_END;

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
