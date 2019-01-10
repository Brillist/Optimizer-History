#ifndef CLP_RANGEVAR_H
#define CLP_RANGEVAR_H

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <libutl/Span.h>
#include <clp/ConstrainedBound.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Range of values expressed as a lower and upper bound.

   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class RangeVar : public utl::Object
{
    UTL_CLASS_DECL(RangeVar);

public:
    /** Constructor. */
    RangeVar(ConstrainedBound* lb, ConstrainedBound* ub, bool failOnEmpty);

    /** Copy another instance. */
    virtual void copy(const utl::Object& rhs);

    /** Get a human-readable string representation. */
    virtual utl::String toString() const;

    /// \name Accessors
    //@{
    /** Get the name. */
    const std::string&
    name() const
    {
        return _name;
    }

    /** Set the name. */
    std::string&
    name()
    {
        return _name;
    }

    /** Get the object. */
    void*
    object() const
    {
        return _object;
    }

    /** Set the object. */
    void*&
    object()
    {
        return _object;
    }

    /** Get the domain span. */
    utl::Span<int>
    span() const
    {
        return utl::Span<int>(lb(), ub() + 1);
    }

    /** Set debug flag. */
    void setDebugFlag(bool debugFlag);

    /** Get the lower bound. */
    int
    lb() const
    {
        return _lb->get();
    }

    /** Get the upper bound. */
    int
    ub() const
    {
        return _ub->get();
    }

    /** Lower bound defined? */
    bool
    hasLowerBound() const
    {
        return (_lb != nullptr);
    }

    /** Upper bound defined? */
    bool
    hasUpperBound() const
    {
        return (_ub != nullptr);
    }

    /** Get the lower bound. */
    ConstrainedBound&
    lowerBound()
    {
        ASSERTD(_lb != nullptr);
        return *_lb;
    }

    /** Get the upper bound. */
    ConstrainedBound&
    upperBound()
    {
        ASSERTD(_ub != nullptr);
        return *_ub;
    }

    /** Get the lower bound. */
    const ConstrainedBound&
    lowerBound() const
    {
        ASSERTD(_lb != nullptr);
        return *_lb;
    }

    /** Get the upper bound. */
    const ConstrainedBound&
    upperBound() const
    {
        ASSERTD(_ub != nullptr);
        return *_ub;
    }

    /** Get the single value in the range. */
    int
    value() const
    {
        ASSERTD(isBound());
        return lb();
    }

    /** Get the size of the domain. */
    virtual utl::uint_t
    size() const
    {
        ASSERTD(!empty());
        return (ub() - lb() + 1);
    }

    /** Empty domain? */
    bool
    empty() const
    {
        return (lb() > ub());
    }

    /** Does the domain contain a single value? */
    bool
    isBound() const
    {
        return (lb() == ub());
    }

    /** Does the domain include the given value? */
    bool
    has(int val) const
    {
        return ((val >= lb()) && (val <= ub()));
    }
    //@}

    /// \name Domain Modification
    //@{
    /** Set bounds. */
    void
    set(ConstrainedBound* lb, ConstrainedBound* ub)
    {
        setLB(lb);
        setUB(ub);
    }

    /** Set the lower bound. */
    void setLB(ConstrainedBound* lb);

    /** Set the upper bound. */
    void setUB(ConstrainedBound* ub);

    /** Set a lower bound. */
    void
    setLB(int lb)
    {
        _lb->setLB(lb);
    }

    /** Set an upper bound. */
    void
    setUB(int ub)
    {
        _ub->setUB(ub);
    }

    /** Remove all values except 'val'. */
    void
    setValue(int val)
    {
        setRange(val, val);
    }

    /**
       Remove domain values that are not in the given range.
    */
    void
    intersect(const RangeVar* rangeVar)
    {
        setRange(rangeVar->lb(), rangeVar->ub());
    }

    /** Remove domain values less than \b lb or greater than \b ub. */
    void
    setRange(int lb, int ub)
    {
        _lb->setLB(lb);
        _ub->setUB(ub);
    }
    //@}
private:
    void init();
    void deInit();

private:
    std::string _name;
    void* _object;
    ConstrainedBound* _lb;
    ConstrainedBound* _ub;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_END;

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
