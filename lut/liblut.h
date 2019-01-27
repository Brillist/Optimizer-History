#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#define LUT_NS_DECLARE                                                                             \
    namespace lut                                                                                  \
    {                                                                                              \
    }
#define LUT_NS_BEGIN                                                                               \
    namespace lut                                                                                  \
    {
#define LUT_NS_END }
#define LUT_NS_USE using namespace lut;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   \b libLUT : re-useable classes, functions, types
*/
LUT_NS_DECLARE;

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <libutl/libutl.h>
#include <libutl/RandUtils.h>
#include <libutl/String.h>

// standard library headers
#undef new
#include <algorithm>
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <math.h>
#include <queue>
#include <set>
#include <stack>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <libutl/gblnew_macros.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <lut/types.h>
#include <lut/lut_util.h>
#include <lut/RBtree.h>
#include <lut/RBtreeNode.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

// for convenience
using utl::byte_t;
using utl::String;
using utl::uint_t;
using utl::uint_t_max;
using utl::uint32_t;
using utl::uint32_t_max;
using utl::uint64_t;
using utl::uint64_t_max;
using utl::int64_t;
using utl::int64_t_min;
using utl::int64_t_max;
