#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <clp/IntExp.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Integer variable.

   IntVar is an integer expression (IntExp) that stores its domain instead of computing it.

   \ingroup clp
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class IntVar : public IntExp
{
    UTL_CLASS_DECL(IntVar, IntExp);
    UTL_CLASS_DEFID;

public:
    /**
       Constructor.
       \param mgr associated Manager
       \param domain domain implementation (default is IntExpDomainRISC)
    */
    IntVar(Manager* mgr, IntExpDomain* domain = nullptr)
        : IntExp(mgr, domain)
    {
    }

    /**
       Constructor.
       \param mgr owning manager
       \param val initially bound value
    */
    IntVar(Manager* mgr, int val)
        : IntExp(mgr)
    {
        setRange(val, val);
    }

    /**
       Constructor.
       \param mgr owning manager
       \param min minimum value
       \param max maximum value
    */
    IntVar(Manager* mgr, int min, int max)
        : IntExp(mgr)
    {
        setRange(min, max);
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_END;
