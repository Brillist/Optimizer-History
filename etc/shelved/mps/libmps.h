#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <mrp/libmrp.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

#define MPS_NS_DECLARE                                                                             \
    namespace mps                                                                                  \
    {                                                                                              \
    }
#define MPS_NS_BEGIN                                                                               \
    namespace mps                                                                                  \
    {
#define MPS_NS_END }
#define MPS_NS_USE using namespace mps;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   \b libMPS : MPS functionality.
*/
MPS_NS_DECLARE;
