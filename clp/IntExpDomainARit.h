#ifndef CLP_INTEXPDOMAINARIT_H
#define CLP_INTEXPDOMAINARIT_H

//////////////////////////////////////////////////////////////////////////////

#include <clp/IntExpDomainIt.h>

//////////////////////////////////////////////////////////////////////////////

CLP_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////

class IntExpDomainAR;

//////////////////////////////////////////////////////////////////////////////

/**
   Integer expression domain.

   \author Adam McKee
*/

//////////////////////////////////////////////////////////////////////////////

class IntExpDomainARit : public IntExpDomainIt
{
    UTL_CLASS_DECL(IntExpDomainARit);
    UTL_CLASS_DEFID;
public:
    /** Constructor. */
    IntExpDomainARit(
        const IntExpDomainAR* domain,
        utl::uint_t idx,
        utl::uint_t val)
    { _domain = domain; _idx = idx; _val = val; }

    /** Copy another instance. */
    virtual void copy(const utl::Object& rhs);

    /** Move forward. */
    virtual void next();

    /** Move backward. */
    virtual void prev();
private:
    const IntExpDomainAR* _domain;
    utl::uint_t _idx;
};

//////////////////////////////////////////////////////////////////////////////

CLP_NS_END;

//////////////////////////////////////////////////////////////////////////////

#endif
