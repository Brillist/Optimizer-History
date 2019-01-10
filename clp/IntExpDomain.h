#ifndef CLP_INTEXPDOMAIN_H
#define CLP_INTEXPDOMAIN_H

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <clp/IntExpDomainIt.h>
#include <clp/Manager.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Integer expression domain.

   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class IntExpDomain : public utl::Object
{
    UTL_CLASS_DECL_ABC(IntExpDomain);

public:
    /** Constructor. */
    IntExpDomain(Manager* mgr)
    {
        init(mgr);
    }

    /** Copy another instance. */
    virtual void copy(const utl::Object& rhs);

    /** Get a human-readable string representation. */
    virtual utl::String toString() const;

    /** Remove domain values that are not in the given array. */
    virtual void intersect(const int* array, utl::uint_t size);

    /** Remove domain values that are not in the given array. */
    void
    intersect(const utl::uint_t* array, utl::uint_t size)
    {
        intersect((const int*)array, size);
    }

    /** Remove domain values that are not in the given set. */
    virtual void intersect(const std::set<int>& intSet);

    /** Intersect with the given domain. */
    virtual void intersect(const IntExpDomain* rhs);

    /** Add the given value. */
    void
    add(int val)
    {
        addRange(val, val);
    }

    /** Add the given span. */
    void
    add(int min, int max)
    {
        if (min > max)
            return;
        addRange(min, max);
    }

    /** Remove the given value. */
    void
    remove(int val)
    {
        if ((val > _max) || (val < _min))
            return;
        removeRange(val, val);
    }

    /** Remove the given span. */
    void
    remove(int min, int max)
    {
        if ((min > max) || (min > _max) || (max < _min))
            return;
        removeRange(min, max);
    }

    /// \name Accessors
    //@{
    /** Contains the given value? */
    virtual bool has(int val) const = 0;

    /** Get begin iterator. */
    virtual IntExpDomainIt* begin() const = 0;

    /** Get end iterator. */
    virtual IntExpDomainIt* end() const = 0;

    /** Get the maximum value < val. */
    virtual int getPrev(int val) const = 0;

    /** Get the minimum value > val. */
    virtual int getNext(int val) const = 0;

    /** Get the minimum value. */
    int
    min() const
    {
        return _min;
    }

    /** Get the maximum value. */
    int
    max() const
    {
        return _max;
    }

    /** Empty? */
    bool
    empty() const
    {
        return ((_size == 0) || ((_events & ef_empty) != 0));
    }

    /** Get the size. */
    utl::uint_t
    size() const
    {
        return _size;
    }
    //@}

    /// \name Events
    //@{
    /** Any event? */
    bool
    anyEvent() const
    {
        return (_events != 0);
    }

    /** Clear event flags. */
    void
    clearEvents()
    {
        _events = 0;
    }

    /** Empty event? */
    bool
    emptyEvent() const
    {
        return ((_events & ef_empty) != 0);
    }

    /** Domain event? */
    bool
    domainEvent() const
    {
        return ((_events & ef_domain) != 0);
    }

    /** Min event? */
    bool
    minEvent() const
    {
        return ((_events & ef_min) != 0);
    }

    /** Max event? */
    bool
    maxEvent() const
    {
        return ((_events & ef_max) != 0);
    }

    /** Range event? */
    bool
    rangeEvent() const
    {
        return ((_events & ef_range) != 0);
    }

    /** Value event? */
    bool
    valueEvent() const
    {
        return ((_events & ef_value) != 0);
    }
    //@}
protected:
    virtual void _saveState();

    virtual void
    addRange(int min, int max)
    {
        ABORT();
    }

    virtual void
    removeRange(int min, int max)
    {
        ABORT();
    }

protected:
    enum event_flag_t
    {
        ef_empty = 1,
        ef_domain = 2,
        ef_min = 4,
        ef_max = 8,
        ef_range = 16,
        ef_value = 32
    };

protected:
    Manager* _mgr;

    utl::uint_t _events;

    // reversible //////////////////////////////////////
    int _min;
    int _max;
    utl::uint_t _size;
    // reversible //////////////////////////////////////
private:
    void init(Manager* mgr = nullptr);
    void
    deInit()
    {
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_END;

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
