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
   LUT package namespace.
   \ingroup lut
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
#include <lut/util.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   \defgroup lut Package LUT
   \short Various functions and types for project-wide use.
*/
