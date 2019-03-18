#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

LUT_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Skip-list depth array.

   When adding a new node to a skip list a depth must be chosen for it.  At the highest level
   of the skip list there should only be a few nodes, roughly twice as many nodes at the next
   level down, etc.  Given the maximum depth of a skip list, SkipListDepthArray generates a
   sequence of depths to be assigned to newly added nodes in the skip list.

   For example if our skip-list is (maximally) 3 levels deep, we number the nodes this way:

   \code
          1          <- level 2
      2       3      <- level 1
    4   5   6   7    <- level 0
   \endcode

   SkipListDepthArray will generate the following list of 7 node levels: [0,1,0,2,0,1,0].
   
   \ingroup lut
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class SkipListDepthArray : public utl::Object
{
    UTL_CLASS_DECL(SkipListDepthArray, utl::Object);

public:
    /** Constructor. */
    SkipListDepthArray(uint_t depth);

    virtual void copy(const utl::Object& rhs);

    /**
       Initialize.
       \param depth maximum tree/list depth
    */
    void set(uint_t depth);

    /**
       Get the next depth and advance to the next position.
       \return next depth
       \param pos reference to current position
    */
    uint_t
    next(uint_t& pos) const
    {
        ASSERTD(pos < _size);
        uint_t res = _depth[pos++];
        if (pos == _size)
            pos = 0;
        return res;
    }

private:
    void
    init()
    {
        _size = 0;
        _maxDepth = 0;
        _depth = nullptr;
    }

    void
    deInit()
    {
        delete[] _depth;
    }

    void dfs(uint_t& idx, uint_t node, uint_t level);

private:
    uint_t _size;
    uint_t _maxDepth;
    byte_t* _depth;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

LUT_NS_END;
