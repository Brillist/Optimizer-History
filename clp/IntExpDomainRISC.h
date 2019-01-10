#ifndef CLP_INTEXPDOMAINRISC_H
#define CLP_INTEXPDOMAINRISC_H

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <clp/IntExpDomain.h>
#include <clp/RevIntSpanCol.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Integer expression domain (RevIntSpanCol representation).

   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class IntExpDomainRISC : public IntExpDomain, public RevIntSpanCol
{
    UTL_CLASS_DECL(IntExpDomainRISC);

public:
    /** Constructor. */
    IntExpDomainRISC(Manager* mgr)
        : IntExpDomain(mgr)
    {
        init();
    }

    /** Copy another instance. */
    virtual void copy(const utl::Object& rhs);

    /** Get a human-readable string representation. */
    virtual utl::String toString() const;

    /** Intersect with the given domain. */
    virtual void intersect(const IntExpDomain* rhs);

    /** Contains the given value? */
    virtual bool has(int val) const;

    /** Get begin iterator. */
    virtual IntExpDomainIt* begin() const;

    /** Get end iterator. */
    virtual IntExpDomainIt* end() const;

    /** Get the maximum value < val. */
    virtual int getPrev(int val) const;

    /** Get the minimum value > val. */
    virtual int getNext(int val) const;

protected:
    Manager*
    mgr()
    {
        return IntExpDomain::_mgr;
    }

    virtual void _saveState();

    virtual void addRange(int min, int max);

    virtual void removeRange(int min, int max);

    virtual void set(IntSpan* span);

    void
    set(int min, int max, utl::uint_t v0, utl::uint_t v1)
    {
        RevIntSpanCol::set(min, max, v0, v1);
    }

    virtual utl::uint_t validate(bool initialized = true) const;

private:
    void init();
    void
    deInit()
    {
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_END;

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
