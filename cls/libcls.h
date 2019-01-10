#ifndef LIBCLS_H
#define LIBCLS_H

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
   \b libCLS : scheduling-related constraints and goals (depends on \b libCLP)
*/
CLS_NS_DECLARE;

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_END;

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
