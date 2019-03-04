#pragma once

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
    UTL_CLASS_DECL(RangeVar, utl::Object);

public:
    /** Constructor. */
    RangeVar(ConstrainedBound* lb, ConstrainedBound* ub, bool failOnEmpty);

    virtual void copy(const utl::Object& rhs);

    virtual String toString() const;

    /// \name Accessors (const)
    //@{
    /** Get the name. */
    const std::string&
    name() const
    {
        return _name;
    }

    /** Get the object. */
    void*
    object() const
    {
        return _object;
    }

    /** Get the domain span. */
    utl::Span<int>
    span() const
    {
        return utl::Span<int>(lb(), ub() + 1);
    }

    /** Only one value in range? */
    bool
    isBound() const
    {
        return (lb() == ub());
    }

    /** Get the single value in the range. */
    int
    value() const
    {
        ASSERTD(isBound());
        return lb();
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
    //@}

    /// \name Accessors (non-const)
    //@{
    /** Set the name. */
    void
    setName(const std::string& name)
    {
        _name = name;
    }

    /** Set the object. */
    void setObject(void* object)
    {
        _object = object;
    }

    /** Set debug flag. */
    void setDebugFlag(bool debugFlag);

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
    //@}

    /// \name Modification
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

    /** Remove all values except \c val. */
    void
    setValue(int val)
    {
        setRange(val, val);
    }

    /** Remove values not in the given range. */
    void
    intersect(const RangeVar* rangeVar)
    {
        setRange(rangeVar->lb(), rangeVar->ub());
    }

    /** Remove values that don't fall in \c [lb,ub]. */
    void
    setRange(int lb, int ub)
    {
        _lb->setLB(lb);
        _ub->setUB(ub);
    }
    //@}
private:
    void init();
    void deInit()
    {
    }

private:
    std::string _name;
    void* _object;
    ConstrainedBound* _lb;
    ConstrainedBound* _ub;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_END;
