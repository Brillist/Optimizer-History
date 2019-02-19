#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <lut/liblut.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

#define GOP_NS_DECLARE                                                                             \
    namespace gop                                                                                  \
    {                                                                                              \
    }
#define GOP_NS_BEGIN                                                                               \
    namespace gop                                                                                  \
    {
#define GOP_NS_END }
#define GOP_NS_USE using namespace gop;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   GOP package namespace.
   \ingroup gop
*/
GOP_NS_DECLARE;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   \defgroup gop Package GOP
   \short Optimization framework.
*/
