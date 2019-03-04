#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <libutl/Span.h>
#include <clp/ConstrainedBound.h>
#include <clp/IntExpDomain.h>
#include <clp/RevSet.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

class Bound;
class Constraint;
class Goal;
class IntExpDomainRISC;
class IntExpDomainIt;
class IntSpan;
class Manager;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Integer expression (abstract).

   IntExp is an abstract base for all integer expression classes.  It records the domain of the
   integer expression, and tracks changes to the domain between choice points.

   IntExp provides facilities for updating a ConstrainedBound or another IntExp in response to
   changes in its domain (see \ref addIntersectExp, \ref addDomainBound, \ref addValueBound).

   \see ChoicePoint
   \see ConstrainedBound
   \ingroup clp
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class IntExp : public ConstrainedVar
{
    UTL_CLASS_DECL_ABC(IntExp, ConstrainedVar);

public:
    typedef IntExpDomainIt* iterator;

public:
    /**
       Constructor.
       \param mgr associated Manager
       \param domain domain implementation (default is IntExpDomainRISC)
    */
    IntExp(Manager* mgr, IntExpDomain* domain = nullptr);

    virtual void copy(const utl::Object& rhs);

    /** Return a managed copy of self. */
    virtual IntExp* mclone();

    /** Make managed copies of unmanaged referenced objects. */
    virtual void mcopy();

    virtual String toString() const;

    virtual void backtrack();

    /// \name Accessors (const)
    //@{
    virtual bool managed() const;

    /** Get the manager. */
    Manager*
    manager() const
    {
        return _mgr;
    }

    /** Get the name. */
    const std::string&
    name() const
    {
        return _name;
    }

    /** Empty domain triggers search failure? */
    bool
    failOnEmpty() const
    {
        return _failOnEmpty;
    }

    /** Get the domain. */
    const IntExpDomain*
    domain() const
    {
        return _domain;
    }

    /** Get the domain (which must be of type IntExpDomainRISC). */
    const IntExpDomainRISC* domainRISC() const;

    /** Get begin iterator. */
    iterator
    begin() const
    {
        return _domain->begin();
    }

    /** Get end iterator. */
    iterator
    end() const
    {
        return _domain->end();
    }
    //@}

    /// \name Accessors (non-const)
    //@{
    /** Set the manager. */
    void setManager(Manager* mgr);

    /** Set the \c managed flag. */
    void
    setManaged(bool managed)
    {
        _managed = managed;
    }

    /** Set the name. */
    void
    setName(const std::string& name)
    {
        _name = name;
    }

    /** Indicate whether an empty domain should trigger failure of the current search. */
    void
    setFailOnEmpty(bool failOnEmpty)
    {
        _failOnEmpty = failOnEmpty;
    }
    //@}

    /// \name Domain Queries
    //@{
    /** Get the size of the domain. */
    uint_t size() const
    {
        return _domain->size();
    }

    /** Domain is empty? */
    bool
    empty() const
    {
        return _domain->empty();
    }

    /** Domain contains a single value? */
    bool
    isBound() const
    {
        return (size() == 1);
    }

    /** Return a span that contains all values in the domain. */
    utl::Span<int>
    span() const
    {
        return utl::Span<int>(min(), max() + 1);
    }

    /** Get the smallest value in the domain. */
    int
    min() const
    {
        return _domain->min();
    }

    /** Get the largest value in the domain. */
    int
    max() const
    {
        return _domain->max();
    }

    /** Get the single value in the domain. */
    int
    value() const
    {
        ASSERTD(isBound());
        return _domain->min();
    }

    /** Does the domain include the given value? */
    bool
    has(int val) const
    {
        return _domain->has(val);
    }

    /**
       Return the largest domain value that is smaller than \c val
       (or `utl::int_t_min` if no such domain value exists).
    */
    int
    getPrev(int val) const
    {
        return _domain->getPrev(val);
    }

    /**
       Return the largest domain value that is smaller than \c val
       (or `utl::int_t_min` if no such domain value exists).
    */
    int
    getNext(int val) const
    {
        return _domain->getNext(val);
    }

    /** Get the associated object. */
    void*
    getObject() const
    {
        return _object;
    }

    /** Set the associated object. */
    void
    setObject(void* object)
    {
        _object = object;
    }
    //@}

    /// \name Domain Modification
    //@{
    /**
       Add a value.
       \param val value to add
       \return true if the value was added, false if it was present
    */
    bool add(int val);

    /**
       Add values in the range `[min,max]`.
       \param min minimum of added range
       \param max maximum of added range
       \return count of values in `[min,max]` that were actually added (not already present)
    */
    uint_t add(int min, int max);

    /**
       Remove a value.
       \param val value to remove
       \return true if the value was removed, false if it wasn't present
    */
    bool remove(int val);

    /**
       Remove domain values in the range `[min,max]`.
       \param min minimum value of removed range
       \param max maximum value of removed range
       \return count of values in `[min,max]` that were actually removed (not already absent)
    */
    uint_t remove(int min, int max);

    /** Enable or disable deferment of domain modifications. */
    void setDeferRemoves(bool deferRemoves = true);

    /** Remove values outside `[min,max]`. */
    void setRange(int min, int max);

    /** Remove values greater than `min`. */
    void setMin(int min);

    /** Remove values less than `max`. */
    void setMax(int max);

    /** Remove values except \c val. */
    void
    setValue(int val)
    {
        setRange(val, val);
        ASSERTD(_domain->empty() || (value() == val));
    }

    /** Remove values that are not in the domain of the given expression. */
    void intersect(const IntExp* expr);

    /** Remove values that are not in the given array. */
    void intersect(const int* array, uint_t size);

    /** Remove values that are not in the given array. */
    void intersect(const uint_t* array, uint_t size);

    /** Remove values that are not in the given set. */
    void intersect(const std::set<int>& intSet);
    //@}

    /// \name Events
    //@{
    /**
       Add an "intersect expression" (one whose domain is a subset of this expression's domain).
    */
    void
    addIntersectExp(IntExp* exp)
    {
        _intersectExps.add(exp);
    }

    /**
       Changes in this IntExp's domain will invalidate the given bound.
       \see Bound::invalidate
    */
    void
    addDomainBound(ConstrainedBound* bound)
    {
        _domainBounds.add(bound);
    }

    /**
       Reduction of this IntExp's domain to a single value will invalidate the given bound.
       \see Bound::invalidate
    */
    void
    addValueBound(ConstrainedBound* bound)
    {
        _valueBounds.add(bound);
    }

    /** Undo addDomainBound. */
    void
    removeDomainBound(ConstrainedBound* bound)
    {
        _domainBounds.remove(bound);
    }

    /** Undo addValueBound. */
    void
    removeValueBound(ConstrainedBound* bound)
    {
        _valueBounds.remove(bound);
    }
    //@}
protected:
    void raiseEvents();

protected:
    IntExpDomain* _domain;

private:
    void init();
    void deInit();

private:
    using cb_set_t = RevSet<ConstrainedBound>;
    using intexp_array_t = RevArray<IntExp*>;

private:
    Manager* _mgr;
    bool _managed;
    std::string _name;
    bool _failOnEmpty;
    void* _object;
    intexp_array_t _intersectExps;
    cb_set_t _domainBounds;
    cb_set_t _valueBounds;

    // deferred removals
    uint_t _stateDepth;
    bool _deferRemoves;
    int* _deferredRemoves;
    int* _deferredRemovesPtr;
    int* _deferredRemovesLim;
    size_t _deferredRemovesSize;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_END;
