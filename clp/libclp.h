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
   \b libCLP : constraint/logic programming framework.
*/
CLP_NS_DECLARE;
