#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <clp/ConstrainedVar.h>
#include <clp/Manager.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   RevArray backtracking mode.
   \ingroup clp
*/
enum ra_btmode_t
{
    ra_bt_full, /**< save entire array */
    ra_bt_elem  /**< save modified elements individually */
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//// RevArray //////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Reversible array.

   RevArray is an array that supports backtracking.

   \see Manager
   \ingroup clp
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

template <class T> class RevArray : public utl::Object
{
    UTL_CLASS_DECL_TPL(RevArray, T, utl::Object);
    UTL_CLASS_NO_COPY;

public:
    using value_type = T;
    using iterator = T*;
    using const_iterator = const T*;

public:
    /**
       Constructor.
       \param mgr associated Manager
       \param btMode backtracking mode
    */
    RevArray(Manager* mgr, ra_btmode_t btMode = ra_bt_full)
    {
        init();
        initialize(mgr, btMode);
    }

    /**
       Initialize.
       \param mgr associated Manager
       \param btMode backtracking mode
    */
    void
    initialize(Manager* mgr, ra_btmode_t btMode = ra_bt_full)
    {
        ASSERTD(_mgr == nullptr);
        _mgr = mgr;
        _btMode = btMode;
    }

    /// \name Accessors (const)
    //@{
    /** Get the manager. */
    Manager*
    manager() const
    {
        return _mgr;
    }

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

    /** Index into the array. */
    const T&
    get(uint_t idx) const
    {
        ASSERTD(idx < _size);
        return _array[idx];
    }

    /** Index into the array. */
    const T& operator[](uint_t idx) const
    {
        ASSERTD(idx < _size);
        return _array[idx];
    }

    /** Get begin iterator. */
    const_iterator
    begin() const
    {
        return _array;
    }

    /** Get end iterator. */
    const_iterator
    end() const
    {
        return (_array + _size);
    }
    //@}

    /// \name Accessors (non-const)
    //@{
    /** Get begin iterator. */
    iterator
    begin()
    {
        return _array;
    }

    /** Get end iterator. */
    iterator
    end()
    {
        return (_array + _size);
    }
    //@}

    /// \name Modification
    //@{
    /** Empty the array. */
    void clear();

    /** Add the given object to the end of the array. */
    void add(const T& val);

    /** Set the value of an array element. */
    void set(uint_t idx, const T& val);

    /** Remove an array element. */
    void remove(uint_t idx);
    //@}

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

    /// reversible /////////////////////////////////////////////
    uint_t _stateDepth;
    uint_t _size;
    uint_t _maxSize; // _size at last _saveState() call
    /// reversible /////////////////////////////////////////////

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

    // if we are modifying something that is visible to a prior state, save old value(s)
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

    // one less element
    --_size;

    // removing the last element is a simple case
    if (idx == _size)
    {
        return;
    }

    // NOTE: _size is an index to the last element (not the removed element)

    // if we are modifying something that is visible to a prior state, save old value(s)
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
