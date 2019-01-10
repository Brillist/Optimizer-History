#ifndef LIBGOP_H
#define LIBGOP_H

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
   \b libGOP : optimization framework
*/
GOP_NS_DECLARE;

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
