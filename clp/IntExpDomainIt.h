#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Iterator for IntExpDomain.

   \ingroup clp
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class IntExpDomainIt : public utl::Object
{
    UTL_CLASS_DECL_ABC(IntExpDomainIt, utl::Object);
    UTL_CLASS_DEFID;

public:
    /// \name Accessors (const)
    //@{
    /** At end of domain? */
    bool
    atEnd() const
    {
        return (_val == utl::int_t_max);
    }

    /** Get current value. */
    int
    get() const
    {
        return _val;
    }

    /** Get current value. */
    int operator*() const
    {
        return _val;
    }
    //@}

    /// \name Movement
    //@{
    /** Move forward. */
    virtual void next() = 0;

    /** Move backward. */
    virtual void prev() = 0;
    //@}

protected:
    int _val;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_END;
