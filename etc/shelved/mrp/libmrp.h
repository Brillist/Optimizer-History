#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <cse/libcse.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

#define MRP_NS_DECLARE                                                                             \
    namespace mrp                                                                                  \
    {                                                                                              \
    }
#define MRP_NS_BEGIN                                                                               \
    namespace mrp                                                                                  \
    {
#define MRP_NS_END }
#define MRP_NS_USE using namespace mrp;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   \b libMRP : MRP functionality.
*/
MRP_NS_DECLARE;
