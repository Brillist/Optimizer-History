#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <clp/ConstrainedVar.h>
#include <clp/Manager.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/** Precedence relationship. */
enum ra_btmode_t
{
    ra_bt_full, /**< save entire array */
    ra_bt_elem  /**< save individually modified elements */
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//// RevArray //////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Reversible array.

   RevArray is an array that tracks changes to itself, so they can be reversed in the event of
   backtracking.

   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

template <class T> class RevArray : public utl::Object
{
    UTL_CLASS_DECL_TPL(RevArray, T, utl::Object);
    UTL_CLASS_NO_COPY;

public:
    typedef T value_type;
    typedef T* iterator;
    typedef const T* const_iterator;

public:
    /** Constructor. */
    RevArray(Manager* mgr, ra_btmode_t btMode = ra_bt_full)
    {
        init();
        initialize(mgr, btMode);
    }

    /** Get the manager. */
    Manager*
    manager() const
    {
        return _mgr;
    }

    /** Initialize. */
    void
    initialize(Manager* mgr, ra_btmode_t btMode = ra_bt_full)
    {
        ASSERTD(_mgr == nullptr);
        _mgr = mgr;
        _btMode = btMode;
    }

    /** Empty the array. */
    void clear();

    /** Get the size. */
    uint_t
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

    /** Add the given object to the end of the array. */
    void add(const T& val);

    /** Index into the array. */
    const T&
    get(uint_t idx) const
    {
        ASSERTD(idx < _size);
        return _array[idx];
    }

    /** Set the value of an array element. */
    void set(uint_t idx, const T& val);

    /** Remove an array element. */
    void remove(uint_t idx);

    /** Get a begin iterator. */
    const_iterator
    begin() const
    {
        return _array;
    }

    /** Get an end iterator. */
    const_iterator
    end() const
    {
        return (_array + _size);
    }

    /** Get a begin iterator. */
    iterator
    begin()
    {
        return _array;
    }

    /** Get an end iterator. */
    iterator
    end()
    {
        return (_array + _size);
    }

    /** Index into the array. */
    const T& operator[](uint_t idx) const
    {
        ASSERTD(idx < _size);
        return _array[idx];
    }

private:
    void
    init()
    {
        _mgr = nullptr;
        _btMode = ra_bt_full;
        _btFullDepth = 0;
        _stateDepth = 0;
        _maxSize = _size = _allocSize = 0;
        _array = nullptr;
    }

    void
    deInit()
    {
        delete[] _array;
    }

    void
    saveState()
    {
        if (_stateDepth < _mgr->depth())
            _saveState();
    }

    void _saveState();

private:
    Manager* _mgr;
    ra_btmode_t _btMode;
    uint_t _btFullDepth;

    /// reversible //////////////////////////////////////////
    uint_t _stateDepth;
    uint_t _maxSize;
    uint_t _size;
    /// reversible //////////////////////////////////////////

    T* _array;
    size_t _allocSize;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

template <class T>
void
RevArray<T>::clear()
{
    saveState();
    _size = 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

template <class T>
void
RevArray<T>::add(const T& val)
{
    saveState();

    // grow array if necessary
    if (_size == _allocSize)
    {
        utl::arrayGrow(_array, _allocSize);
    }

    // if we are modifying something that is visible to a prior state,
    // we must save the old array value(s)
    if (_size < _maxSize)
    {
        if (_btMode == ra_bt_full)
        {
            if (_btFullDepth < _stateDepth)
            {
                _mgr->revSet(_btFullDepth);
                _btFullDepth = _stateDepth;
                _mgr->revSetIndirect(_array, 0, _maxSize);
            }
        }
        else // (_btMode == ra_bt_elem)
        {
            _mgr->revSetIndirect(_array, _size);
        }
    }

    // finally add the value
    _array[_size++] = val;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

template <class T>
void
RevArray<T>::set(uint_t idx, const T& val)
{
    ASSERTD(idx < _size);
    if (_size < _maxSize)
    {
        if (_btMode == ra_bt_full)
        {
            if (_btFullDepth < _stateDepth)
            {
                _mgr->revSet(_btFullDepth);
                _btFullDepth = _stateDepth;
                _mgr->revSetIndirect(_array, 0, _maxSize);
            }
        }
        else
        {
            _mgr->revSetIndirect(_array, idx);
        }
    }
    _array[idx] = val;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

template <class T>
void
RevArray<T>::remove(uint_t idx)
{
    ASSERTD(idx < _size);

    saveState();

    // cleaner to do this up front
    --_size;

    // removing the last element is a simple case
    if (idx == _size)
    {
        return;
    }

    // if we are modifying something that is visible to a prior state,
    // we must save the old array value(s)
    if (idx < _maxSize)
    {
        if (_btMode == ra_bt_full)
        {
            if (_btFullDepth < _stateDepth)
            {
                _mgr->revSet(_btFullDepth);
                _btFullDepth = _stateDepth;
                _mgr->revSetIndirect(_array, 0, _maxSize);
            }
        }
        else // (_btMode == ra_bt_elem)
        {
            _mgr->revSetIndirect(_array, idx);
        }
    }

    // finally remove the value
    _array[idx] = _array[_size];
}

////////////////////////////////////////////////////////////////////////////////////////////////////

template <class T>
void
RevArray<T>::_saveState()
{
    _mgr->revSet(&_stateDepth, 3);
    _stateDepth = _mgr->depth();
    _maxSize = utl::max(_maxSize, _size);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_END;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL_TPL(clp::RevArray, T);
