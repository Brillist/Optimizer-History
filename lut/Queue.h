#ifndef LUT_QUEUE_H
#define LUT_QUEUE_H

////////////////////////////////////////////////////////////////////////////////////////////////////

LUT_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Queue.

   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

template <class T> class Queue : public utl::Object
{
    UTL_CLASS_DECL_TPL(Queue, T);

public:
    /** Copy another instance. */
    virtual void copy(const utl::Object& rhs);

    /** Get the size of the queue. */
    utl::uint_t
    size() const
    {
        return _size;
    }

    /** Empty? */
    bool
    empty() const
    {
        return (_size == 0);
    }

    /** Clear the queue. */
    void
    clear()
    {
        _head = _tail = _items;
        _size = 0;
    }

    /** Peek at the head of the queue. */
    const T&
    head() const
    {
        ASSERTD(_size > 0);
        return *_head;
    }

    /** Add an item to the tail of the queue. */
    INLINE void
    enQ(const T& item)
    {
        if ((_head == _tail) && (_size > 0))
            grow();
        *_tail++ = item;
        if (_tail == _itemsLim)
            _tail = _items;
        ++_size;
    }

    /** Get the item at the head of the queue. */
    INLINE T
    deQ()
    {
        ASSERTD(_size > 0);
        T item = *_head++;
        if (_head == _itemsLim)
            _head = _items;
        --_size;
        return item;
    }

private:
    void init();
    void
    deInit()
    {
        delete[] _items;
    }
    void grow();

private:
    T* _head;
    T* _tail;
    T* _items;
    T* _itemsLim;
    utl::uint_t _size;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

template <class T>
void
Queue<T>::copy(const utl::Object& rhs)
{
    ASSERTD(rhs.isA(Queue<T>));
    const Queue<T>& q = (const Queue<T>&)rhs;
    utl::uint_t itemsSize = (q._itemsLim - q._items);
    delete[] _items;
    _items = new T[itemsSize];
    _itemsLim = _items + itemsSize;
    memcpy(_items, q._items, itemsSize * sizeof(T));
    _head = _items + (q._head - q._items);
    _tail = _items + (q._tail - q._items);
    _size = q._size;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

template <class T>
void
Queue<T>::init()
{
    _head = _tail = _items = new T[K(4)];
    _itemsLim = _items + K(4);
    _size = 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

template <class T>
void
Queue<T>::grow()
{
    // determine new size
    utl::uint_t oldSize = (_itemsLim - _items);
    utl::uint_t newSize;
    if (oldSize < K(1024))
    {
        newSize = oldSize * 2;
    }
    else
    {
        newSize = oldSize + K(1024);
    }

    // build new array
    T* items = new T[newSize];
    T* tail = items;
    while (_size > 0)
    {
        *tail++ = deQ();
    }

    delete[] _items;
    _head = _items = items;
    _tail = tail;
    _itemsLim = _items + newSize;
    _size = (_tail - _head);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

LUT_NS_END;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL_TPL(lut::Queue, T, utl::Object);

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
