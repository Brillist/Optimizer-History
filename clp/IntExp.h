#ifndef CLP_INTEXP_H
#define CLP_INTEXP_H

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <libutl/Span.h>
#include <clp/ConstrainedBound.h>
#include <clp/IntExpDomain.h>
#include <clp/IntSpanArray.h>
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
   Constrained integer expression (abstract).

   IntExp is an abstract base for all integer expression classes.  It
   records the domain of the integer expression, and tracks changes to the
   domain between choice points (see ChoicePoint).  IntExp also keeps track
   of the constraints that are interested in changes to it, and notifies
   those constraints when events of interest occur.  See RevIntSpanCol for a
   description of the events.

   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class IntExp : public ConstrainedVar
{
    UTL_CLASS_DECL_ABC(IntExp, ConstrainedVar);

public:
    typedef IntExpDomainIt* iterator;

public:
    /** Constructor. */
    IntExp(Manager* mgr, IntExpDomain* domain = nullptr);

    /** Copy another instance. */
    virtual void copy(const utl::Object& rhs);

    /** Backtrack. */
    virtual void
    backtrack()
    {
        ABORT();
    }

    /** Return a fully managed copy of self. */
    virtual IntExp* mclone();

    /** Make managed copies of unmanaged referenced objects. */
    virtual void
    mcopy()
    {
    }

    /** Post constraints. */
    virtual void
    postConstraints()
    {
    }

    /// \name Accessors
    //@{
    /** Get a human-readable string representation. */
    virtual String toString() const;

    /** Get the manager. */
    Manager*
    manager() const
    {
        return _mgr;
    }

    /** Set the manager. */
    void setManager(Manager* mgr);

    /** Get the name. */
    const std::string&
    getName() const
    {
        return _name;
    }

    /** Set the name. */
    void
    setName(const std::string& name)
    {
        _name = name;
    }

    /** Fail-on-empty? */
    bool
    failOnEmpty() const
    {
        return _failOnEmpty;
    }

    /** Fail-on-empty? */
    bool&
    failOnEmpty()
    {
        return _failOnEmpty;
    }

    /** Get the domain span. */
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
    getValue() const
    {
        ASSERTD(isBound());
        return _domain->min();
    }

    /** Get the size of the domain. */
    virtual uint_t
    size() const
    {
        return _domain->size();
    }

    /** Empty domain? */
    bool
    empty()
    {
        return _domain->empty();
    }

    /** Does the domain contain a single value? */
    virtual bool
    isBound() const
    {
        return (size() == 1);
    }

    /** Does the domain include the given value? */
    bool
    has(int val) const
    {
        return _domain->has(val);
    }

    /** Get the domain. */
    const IntExpDomain*
    domain() const
    {
        return _domain;
    }

    /** Get the IntExpDomainRISC. */
    const IntExpDomainRISC* domainRISC() const;

    /** Get head. */
    const IntSpan* head() const;

    /** Get tail. */
    const IntSpan* tail() const;

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

    /**
       If \b val is less than or equal to min(), return utl::int_t_min.
       Otherwise, return the first domain value that is less than val.
    */
    int
    getPrev(int val) const
    {
        return _domain->getPrev(val);
    }

    /**
       If \b val is greater than or equal to max(), return utl::int_t_max.
       Otherwise, return the first domain value that is greater than val.
    */
    int
    getNext(int val) const
    {
        return _domain->getNext(val);
    }

    /** Get the \b managed flag. */
    bool
    isManaged() const
    {
        return _managed;
    }

    /** Set the \b managed flag. */
    void
    setManaged(bool managed)
    {
        _managed = managed;
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
    /** Set the domain. */
    void set(int min, int max);

    /** Remove all domain values smaller than min. */
    void
    setMin(int min)
    {
        if (min <= _domain->min())
            return;
        _domain->remove(utl::int_t_min, min - 1);
        if (_domain->anyEvent())
            raiseEvents();
    }

    /** Remove all domain values larger than max. */
    void
    setMax(int max)
    {
        if (max >= _domain->max())
            return;
        _domain->remove(max + 1, utl::int_t_max);
        if (_domain->anyEvent())
            raiseEvents();
    }

    /** Remove from the domain all values except \b val. */
    void
    setValue(int val)
    {
        setRange(val, val);
        ASSERTD(_domain->empty() || (getValue() == val));
    }

    /**
       Remove domain values that are not in the domain of
       the given expression.
    */
    void intersect(const IntExp* expr);

    /** Remove domain values that are not in the given array. */
    void intersect(const int* array, uint_t size);

    /** Remove domain values that are not in the given array. */
    void intersect(const uint_t* array, uint_t size);

    /** Remove domain values that are not in the given set. */
    void intersect(const std::set<int>& intSet);

    /** Add the given value to the domain. */
    bool add(int val);

    /** Add domain values in the range [min,max]. */
    uint_t add(int min, int max);

    /** Remove the given value from the domain. */
    bool remove(int val);

    /** Defer domain modification. */
    void deferRemoves(bool b);

    /** Remove domain values in the range [min,max]. */
    uint_t remove(int min, int max);

    /** Remove domain values less than \b min or greater than \b max. */
    void
    setRange(int min, int max)
    {
        if (min > _domain->min())
            _domain->remove(utl::int_t_min, min - 1);
        if (max < _domain->max())
            _domain->remove(max + 1, utl::int_t_max);
        if (_domain->anyEvent())
        {
            raiseEvents();
        }
    }
    //@}

    void
    addIntersectExp(IntExp* exp)
    {
        _intersectExps.add(exp);
    }

    /// \name Events
    //@{
    /** Add a domain-bound. */
    void
    addDomainBound(ConstrainedBound* bound)
    {
        _domainBounds.add(bound);
    }

    /** Remove a domain-bound. */
    void
    removeDomainBound(ConstrainedBound* bound)
    {
        _domainBounds.remove(bound);
    }

    /** Add a value-bound. */
    void
    addValueBound(ConstrainedBound* bound)
    {
        _valueBounds.add(bound);
    }

    /** Remove a value-bound. */
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
    typedef RevSet<ConstrainedBound> cb_set_t;
    typedef RevArray<IntExp*> intexp_array_t;

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

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
