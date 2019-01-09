#ifndef LUT_SKIPLISTDEPTHARRAY_H
#define LUT_SKIPLISTDEPTHARRAY_H

//////////////////////////////////////////////////////////////////////////////

LUT_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////

/**
   Skip-list depth array.

   \author Adam McKee
*/

//////////////////////////////////////////////////////////////////////////////

class SkipListDepthArray : public utl::Object
{
    UTL_CLASS_DECL(SkipListDepthArray);
public:
    /** Constructor. */
    SkipListDepthArray(utl::uint_t maxDepth);

    /** Copy another instance. */
    virtual void copy(const utl::Object& rhs);

    /** Initialize. */
    void set(utl::uint_t depth);

    /** Get the next depth. */
    utl::uint_t getNext(utl::uint_t& pos) const
    {
        ASSERTD(pos < _size);
        utl::uint_t res = _depth[pos];
        if (++pos == _size) pos = 0;
        return res;
    }
private:
    void init()
    { _size = 0; _maxDepth = 0; _depth = nullptr; }

    void deInit()
    { delete [] _depth; }

    void dfs(utl::uint_t& idx, utl::uint_t node, utl::uint_t level);
private:
    utl::uint_t _size;
    utl::uint_t _maxDepth;
    utl::byte_t* _depth;
};

//////////////////////////////////////////////////////////////////////////////

LUT_NS_END;

//////////////////////////////////////////////////////////////////////////////

#endif
