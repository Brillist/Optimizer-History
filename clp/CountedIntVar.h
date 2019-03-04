#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <clp/IntExp.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Counted integer variable.

   A CountedIntVar is initialized with a set of `[value,count]` tuples, and its domain includes
   all values whose `count` is non-zero.  The count for a value can be reversibly decremented
   by calling \ref decrement.

   \ingroup clp
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class CountedIntVar : public IntExp
{
    UTL_CLASS_DECL(CountedIntVar, IntExp);
    UTL_CLASS_DEFID;

public:
    using int_uint_map_t = std::map<int, uint_t>;

public:
    /**
       Constructor.
       \param mgr owning manager
       \param domain (value,count) tuples for initialization
    */
    CountedIntVar(Manager* mgr, const int_uint_map_t& domain);

    /**
       Decrement the count for a value.
       \param val value to decrement the count for
       \param num amount to deduct (default is 1)
    */
    uint_t decrement(int val, uint_t num = 1);
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_END;
