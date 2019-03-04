#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <libutl/Span.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Array of IntSpans.

   \see IntSpan
   \ingroup clp
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class IntSpanArray : public utl::Object
{
    UTL_CLASS_DECL(IntSpanArray, utl::Object);
    UTL_CLASS_NO_COPY;

public:
    /// \name Accessors (const)
    //@{
    /** Get the size. */
    const uint_t&
    size() const
    {
        return _size;
    }

    /** Get head pointer. */
    const uint_t*
    head() const
    {
        return _array;
    }

    /** Get tail pointer. */
    const uint_t*
    tail() const
    {
        return (_array + (_size * 2));
    }

    /** Get the span at the given index. */
    utl::Span<int>
    get(uint_t pos) const
    {
        ASSERTD(pos < _size);
        uint_t idx = pos * 2;
        return utl::Span<int>(_array[idx], _array[idx + 1] + 1);
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
        ASSERTD(ptr >= _array);
        ASSERTD(ptr < (_array + (_size * 2)));
        min = *(ptr++);
        max = *ptr;
    }

    /** Get the range at a given index. */
    void
    getNext(uint_t*& ptr, int& min, int& max) const
    {
        ASSERTD(ptr >= _array);
        ASSERTD(ptr < (_array + (_size * 2)));
        min = *ptr++;
        max = *ptr++;
    }
    //@}

    /// \name Modification
    //@{
    /** Clear the array. */
    void
    clear()
    {
        _size = 0;
    }

    /** Set the size (an increase in size is not allowed). */
    void
    setSize(uint_t size)
    {
        ASSERTD(size <= _size);
        _size = size;
    }
    /** Add a range of values. */
    void add(int min, int max);
    //@}

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
