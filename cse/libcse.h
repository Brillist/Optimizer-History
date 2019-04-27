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
   CSE package namespace.
   \ingroup cse
*/
CSE_NS_DECLARE;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   \defgroup cse Package CSE
   \short Scheduling Engine.
*/
