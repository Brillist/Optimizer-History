#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <clp/IntExpDomainIt.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

class IntExpDomainAR;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Iterator for IntExpDomainAR.

   \ingroup clp
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class IntExpDomainARit : public IntExpDomainIt
{
    UTL_CLASS_DECL(IntExpDomainARit, IntExpDomainIt);
    UTL_CLASS_DEFID;

public:
    /** Constructor. */
    IntExpDomainARit(const IntExpDomainAR* domain, uint_t idx, uint_t val)
    {
        _domain = domain;
        _idx = idx;
        _val = val;
    }

    virtual void copy(const utl::Object& rhs);

    /// \name Movement
    //@{
    virtual void next();

    virtual void prev();
    //@}

private:
    const IntExpDomainAR* _domain;
    uint_t _idx;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_END;
