#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <gop/libgop.h>
#include <cls/libcls.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

#define CSE_NS_DECLARE                                                                             \
    namespace cse                                                                                  \
    {                                                                                              \
    }
#define CSE_NS_BEGIN                                                                               \
    namespace cse                                                                                  \
    {
#define CSE_NS_END }
#define CSE_NS_USE using namespace cse;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   \b libCSE : scheduling engine (depends on \b libCLP, \b libCLS, \b libGOP)
*/
CSE_NS_DECLARE;
