#ifndef GOP_STRING_H
#define GOP_STRING_H

////////////////////////////////////////////////////////////////////////////////////////////////////

GOP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Chromosome string.

   A chromosome string is a sequence of bytes that represent a construction plan for an individual.
   The format of the instructions must be known by the operators that change them, and by the
   individual-builder that follows them in order to construct individuals.

   \see gop::IndBuilder
   \see gop::Operator
   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

template <class T> class String : public utl::Object
{
    UTL_CLASS_DECL_TPL(String, T, utl::Object);

public:
    /** Constructor. */
    String(uint_t size)
    {
        init(size);
    }

    /** Copy another instance. */
    virtual void copy(const utl::Object& rhs);

    /** Get a human-readable string representation. */
    virtual utl::String toString() const;

    /** Get the size. */
    uint_t
    size() const
    {
        return _size;
    }

    /** Clear. */
    void
    clear()
    {
        setSize(0);
    }

    /** Set the size. */
    void setSize(uint_t size);

    /** Get the array. */
    const T*
    get() const
    {
        return _vect;
    }

    /** Get the array. */
    T*
    get()
    {
        return _vect;
    }

    /** Randomly shuffle. */
    void
    shuffle(lut::rng_t& rng)
    {
        rng.shuffle(_vect, _vect + _size);
    }

    /**
       Perform a crossover.
       \param off1 offspring 1
       \param off2 offspring 2
       \param rhs rhs String (lhs is self)
       \param pos crossover position
    */
    void crossover(String<T>* off1, String<T>* off2, const String<T>& rhs, uint_t pos);

    /** Get the chromosome at the given index (const). */
    const T& operator[](int idx) const
    {
        ASSERTD((idx >= 0) && ((uint_t)idx < _size));
        return _vect[idx];
    }

    /** Get the chromosome at the given index. */
    T& operator[](int idx)
    {
        ASSERTD((idx >= 0) && ((uint_t)idx < _size));
        return _vect[idx];
    }

    /** Get the chromosome at the given index (const). */
    const T& operator[](uint_t idx) const
    {
        ASSERTD(idx < size());
        return _vect[idx];
    }

    /** Get the chromosome at the given index. */
    T& operator[](uint_t idx)
    {
        ASSERTD(idx < size());
        return _vect[idx];
    }

private:
    void init(uint_t size = 0);
    void deInit();

    uint_t _size;
    T* _vect;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

template <class T>
void
String<T>::copy(const utl::Object& rhs)
{
    ASSERTD(rhs.isA(String<T>));
    const String<T>& s = (const String<T>&)rhs;
    setSize(s.size());
    for (uint_t i = 0; i < _size; i++)
    {
        _vect[i] = s._vect[i];
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

template <class T>
utl::String
String<T>::toString() const
{
    std::ostringstream ss;
    ss << "(" << _vect[0];
    for (uint_t i = 1; i < _size; i++)
    {
        ss << "," << _vect[i];
    };
    ss << ")";
    return ss.str().c_str();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

template <class T>
void
String<T>::setSize(uint_t size)
{
    _size = size;
    delete[] _vect;
    if (_size == 0)
    {
        _vect = nullptr;
    }
    else
    {
        _vect = new T[size];
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

template <class T>
void
String<T>::crossover(String<T>* off1, String<T>* off2, const String<T>& rhs, uint_t pos)
{
    ASSERTD(size() == rhs.size());

    // off1
    if (off1 != nullptr)
    {
        off1->setSize(size());
        uint_t i;
        // same as lhs up to pos
        for (i = 0; i < pos; i++)
        {
            off1->_vect[i] = _vect[i];
        }
        // same as rhs for remainder
        for (; i < _size; i++)
        {
            off1->_vect[i] = rhs._vect[i];
        }
    }

    // off2
    if (off2 != nullptr)
    {
        off2->setSize(size());
        uint_t i;
        // same as rhs up to pos
        for (i = 0; i < pos; i++)
        {
            off2->_vect[i] = rhs._vect[i];
        }
        // same as lhs for remainder
        for (; i < _size; i++)
        {
            off2->_vect[i] = _vect[i];
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

template <class T>
void
String<T>::init(uint_t size)
{
    _size = size;
    _vect = new T[size];
}

////////////////////////////////////////////////////////////////////////////////////////////////////

template <class T>
void
String<T>::deInit()
{
    delete[] _vect;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

GOP_NS_END;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL_TPL(gop::String, T);

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
