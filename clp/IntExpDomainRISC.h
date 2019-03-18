#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <clp/IntExpDomain.h>
#include <clp/RevIntSpanCol.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Integer expression domain (RevIntSpanCol representation).

   \ingroup clp
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class IntExpDomainRISC : public IntExpDomain, public RevIntSpanCol
{
    UTL_CLASS_DECL(IntExpDomainRISC, IntExpDomain);

public:
    /**
       Constructor.
       \param mgr associated Manager
    */
    IntExpDomainRISC(Manager* mgr)
        : IntExpDomain(mgr)
    {
        init();
    }

    virtual void copy(const utl::Object& rhs);

    virtual String toString() const;

    virtual void intersect(const IntExpDomain* rhs);

    /// \name Accessors (const)
    //@{
    virtual bool has(int val) const;

    virtual IntExpDomainIt* begin() const;

    virtual IntExpDomainIt* end() const;

    virtual int getPrev(int val) const;

    virtual int getNext(int val) const;
    //@}

protected:
    Manager*
    mgr()
    {
        return super::_mgr;
    }

    virtual void _saveState();

    virtual void addRange(int min, int max);

    virtual void removeRange(int min, int max);

    virtual void set(IntSpan* span);

    void
    set(int min, int max, uint_t v0, uint_t v1)
    {
        RevIntSpanCol::set(min, max, v0, v1);
    }

    virtual uint_t validate(bool initialized = true) const;

private:
    void init();
    void
    deInit()
    {
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_END;
