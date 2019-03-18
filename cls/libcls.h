#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <clp/libclp.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

#define CLS_NS_DECLARE                                                                             \
    namespace cls                                                                                  \
    {                                                                                              \
    }
#define CLS_NS_BEGIN                                                                               \
    namespace cls                                                                                  \
    {
#define CLS_NS_END }
#define CLS_NS_USE using namespace cls;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   CLS package namespace.
   \ingroup cls
*/
CLS_NS_DECLARE;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   \defgroup cls Package CLS
   \short Constraint logic scheduling framework.
*/
