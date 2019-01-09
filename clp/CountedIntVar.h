#ifndef CLP_COUNTEDINTVAR_H
#define CLP_COUNTEDINTVAR_H

//////////////////////////////////////////////////////////////////////////////

#include <clp/IntExp.h>

//////////////////////////////////////////////////////////////////////////////

CLP_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////

/**
   Integer variable.

   IntVar is an integer expression (IntExp) that simply stores its domain
   instead of computing it.

   \author Adam McKee
*/

//////////////////////////////////////////////////////////////////////////////

class CountedIntVar : public IntExp
{
    UTL_CLASS_DECL(CountedIntVar);
    UTL_CLASS_DEFID;
public:
    typedef std::map<int, utl::uint_t> int_uint_map_t;
public:
    /**
       Constructor.
       \param mgr owning manager
       \param domain (value,count) tuples for initialization
    */
    CountedIntVar(Manager* mgr, const int_uint_map_t& domain);

    /** Decrement the count for the given value. */
    utl::uint_t decrement(int val, utl::uint_t num = 1);
};

//////////////////////////////////////////////////////////////////////////////

CLP_NS_END;

//////////////////////////////////////////////////////////////////////////////

#endif
