#ifndef CLPI_INTSPANARRAY_H
#define CLPI_INTSPANARRAY_H

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <libutl/Span.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Array of int-spans.

   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class IntSpanArray : public utl::Object
{
    UTL_CLASS_DECL(IntSpanArray, utl::Object);
    UTL_CLASS_NO_COPY;

public:
    /** Clear the array. */
    void
    clear()
    {
        _size = 0;
    }

    /** Get the size. */
    const uint_t&
    size() const
    {
        return _size;
    }

    uint_t&
    size()
    {
        return _size;
    }

    /** Set the size. */
    void
    setSize(uint_t size)
    {
        ASSERTD(size <= _size);
        _size = size;
    }

    /** Add a range of values. */
    void add(int min, int max);

    /** Get the start pointer. */
    const uint_t*
    head()
    {
        return _array;
    }

    /** Get the tail pointer. */
    const uint_t*
    tail()
    {
        return (_array + (_size << 1));
    }

    /** Get the range at the given index. */
    void
    get(uint_t pos, int& min, int& max) const
    {
        ASSERTD(pos < _size);
        uint_t idx = pos * 2;
        min = _array[idx];
        max = _array[idx + 1];
    }

    /** Get the range at the given index. */
    void
    get(uint_t* ptr, int& min, int& max) const
    {
        ASSERTD(ptr < (_array + (_size << 1)));
        min = *(ptr++);
        max = *ptr;
    }

    /** Get the span at the given index. */
    utl::Span<int>
    get(uint_t pos) const
    {
        ASSERTD(pos < _size);
        uint_t idx = pos * 2;
        return utl::Span<int>(_array[idx], _array[idx + 1] + 1);
    }

    /** Get the range at a given index. */
    void
    getNext(uint_t*& ptr, int& min, int& max) const
    {
        ASSERTD(ptr < (_array + (_size << 1)));
        min = *(ptr++);
        max = *(ptr++);
    }

private:
    void
    init()
    {
        _array = nullptr;
        _size = _reservedSize = 0;
    }

    void deInit();

    void grow();

private:
    uint_t* _array;
    uint_t _size;
    uint_t _reservedSize;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_END;

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
