#ifndef CLP_INTVAR_H
#define CLP_INTVAR_H

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <clp/IntExp.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Integer variable.

   IntVar is an integer expression (IntExp) that simply stores its domain
   instead of computing it.

   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class IntVar : public IntExp
{
    UTL_CLASS_DECL(IntVar, IntExp);
    UTL_CLASS_DEFID;

public:
    /**
       Constructor.
       \param mgr owning manager
    */
    IntVar(Manager* mgr, IntExpDomain* domain = nullptr)
        : IntExp(mgr, domain)
    {
    }

    /**
       Constructor.
       \param mgr owning manager
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

    virtual void
    mcopy()
    {
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_END;

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
