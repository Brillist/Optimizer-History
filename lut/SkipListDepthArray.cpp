#include "liblut.h"
#include <libutl/R250.h>
#include "SkipListDepthArray.h"

//////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;

//////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(lut::SkipListDepthArray, utl::Object);

//////////////////////////////////////////////////////////////////////////////

LUT_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////

SkipListDepthArray::SkipListDepthArray(uint_t depth)
{
    init();
    set(depth);
}

//////////////////////////////////////////////////////////////////////////////

void
SkipListDepthArray::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(SkipListDepthArray));
    const SkipListDepthArray& slda = (const SkipListDepthArray&)rhs;
    _size = slda._size;
    _maxDepth = slda._maxDepth;
    _depth = new byte_t[_size];
    memcpy(_depth, slda._depth, _size);
}

//////////////////////////////////////////////////////////////////////////////

void
SkipListDepthArray::set(uint_t depth)
{
    // determine _size
    _maxDepth = depth - 1;
    _size = 1;
    for (uint_t i = 0; i < depth; ++i)
    {
        _size *= 2;
    }
    --_size;

    // create depth[] array
    delete [] _depth;
    _depth = new byte_t[_size];
    uint_t idx = 0;
    dfs(idx, 1, _maxDepth);
}

//////////////////////////////////////////////////////////////////////////////

void
SkipListDepthArray::dfs(uint_t& idx, uint_t node, uint_t level)
{
    uint_t node_x_2 = node * 2;
    if (node_x_2 <= _size)
    {
        dfs(idx, node_x_2, level - 1);
    }
    _depth[idx++] = level;
    if ((node_x_2 + 1) <= _size)
    {
        dfs(idx, node_x_2 + 1, level - 1);
    }
}

//////////////////////////////////////////////////////////////////////////////

LUT_NS_END;
