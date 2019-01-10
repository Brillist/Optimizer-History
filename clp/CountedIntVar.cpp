#include "libclp.h"
#include "IntExpDomainCAR.h"
#include "Manager.h"
#include "CountedIntVar.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(clp::CountedIntVar, clp::IntExp);

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

CountedIntVar::CountedIntVar(Manager* mgr, const int_uint_map_t& domain)
    : IntExp(mgr, new IntExpDomainCAR(mgr, domain))
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////

uint_t
CountedIntVar::decrement(int val, uint_t num)
{
    IntExpDomainCAR* domain = (IntExpDomainCAR*)_domain;
    uint_t count = domain->decrement(val, num);
    if (domain->anyEvent())
    {
        raiseEvents();
    }
    return count;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_END;
