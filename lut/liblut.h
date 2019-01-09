#ifndef LIBLUT_H
#define LIBLUT_H

//////////////////////////////////////////////////////////////////////////////

#define LUT_NS_DECLARE  namespace lut {}
#define LUT_NS_BEGIN    namespace lut {
#define LUT_NS_END      }
#define LUT_NS_USE      using namespace lut;

//////////////////////////////////////////////////////////////////////////////

/**
   \b libLUT : re-useable classes, functions, types
*/
LUT_NS_DECLARE;

//////////////////////////////////////////////////////////////////////////////

#include <libutl/libutl.h>
#include <libutl/String.h>
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

//////////////////////////////////////////////////////////////////////////////

#define std_hash_map std::unordered_map
#define std_hash_set std::unordered_set

//////////////////////////////////////////////////////////////////////////////

#include <lut/macros.h>
#include <lut/types.h>
#include <lut/lut_util.h>
#include <lut/RBtree.h>
#include <lut/RBtreeNode.h>

//////////////////////////////////////////////////////////////////////////////

#endif
