#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <lut/liblut.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

#define CLP_NS_DECLARE                                                                             \
    namespace clp                                                                                  \
    {                                                                                              \
    }
#define CLP_NS_BEGIN                                                                               \
    namespace clp                                                                                  \
    {
#define CLP_NS_END }
#define CLP_NS_USE using namespace clp;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   CLP package namespace.
   \ingroup clp
*/
CLP_NS_DECLARE;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   \defgroup clp Package CLP
   \short Constraint logic programming framework.
*/
