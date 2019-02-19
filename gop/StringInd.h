#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <gop/String.h>
#include <gop/Ind.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

GOP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Individual with construction string (abstract).

   StringInd is an abstract base for individuals that have a String representation.

   \ingroup gop
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

template <class T> class StringInd : public gop::Ind
{
    UTL_CLASS_DECL_TPL(StringInd, T, gop::Ind);

public:
    /**
       Constructor.
       \param size String size
    */
    StringInd(uint_t size)
    {
        init();
        _string->setSize(size);
    }

    virtual void copy(const utl::Object& rhs);

    virtual utl::String toString() const;

    /** Override from gop::Ind. */
    virtual void
    buildClear()
    {
    }

    /// \name Accessors (const)
    //@{
    /** Get the String size. */
    uint_t
    size() const
    {
        return _string->size();
    }

    /** Get the String. */
    const String<T>&
    string() const
    {
        return *_string;
    }

    /** Get the String (pointer). */
    const String<T>*
    stringPtr() const
    {
        return _string;
    }

    /** Get the new String. */
    const String<T>*
    newString() const
    {
        return _newString;
    }

    /** Get a value from the String. */
    const T&
    get(uint_t idx) const
    {
        return (*_string)[idx];
    }

    /** Get the String value at the given index. */
    const T& operator[](uint_t idx) const
    {
        return (*_string).operator[](idx);
    }

    /** Override from gop::Ind. */
    virtual bool
    isBuilt() const
    {
        return false;
    }
    //@}

    /// \name Accessors (non-const)
    //@{
    /** Get the String. */
    String<T>&
    string()
    {
        return *_string;
    }

    /** Get string pointer. */
    String<T>*
    stringPtr()
    {
        return _string;
    }

    /** Get the new String. */
    String<T>*
    newString()
    {
        return _newString;
    }

    /** Get the value at the given String index. */
    T& operator[](uint_t idx)
    {
        return (*_string).operator[](idx);
    }
    //@}

    /// \name Modification
    //@{
    /** Set the size. */
    void
    setSize(uint_t size)
    {
        _string->setSize(size);
    }

    /**
       Set a value in the String.
       \param pos position of value to modify
       \param val value to set at position \c pos
    */
    void
    set(uint_t pos, uint_t val)
    {
        (*_string)[pos] = val;
    }

    /** Set string pointer. */
    void
    setString(String<T>* string, bool owner = true)
    {
        if (_stringOwner)
            delete _string;
        _string = string;
        _stringOwner = owner;
    }

    /** Delete the new string. */
    void
    deleteNewString()
    {
        delete _newString;
        _newString = nullptr;
    }

    /** Create the new string. */
    void
    createNewString()
    {
        delete _newString;
        _newString = _string->clone();
    }

    /** replace the string with the new string. */
    void
    acceptNewString()
    {
        _string->copy(*_newString);
        deleteNewString();
    }
    //@}

    /**
       Construct two new individuals by performing a crossover with the given StringInd.
       \param off1 (output) offspring 1
       \param off2 (output) offspring 2
       \param rhs rhs String (lhs is self)
       \param pos crossover position
    */
    void
    crossover(StringInd<T>* off1, StringInd<T>* off2, const StringInd<T>& rhs, uint_t pos) const;

protected:
    String<T>* _string;
    String<T>* _newString;
    bool _stringOwner;

private:
    void init();
    void deInit();
};

////////////////////////////////////////////////////////////////////////////////////////////////////

template <class T>
void
StringInd<T>::copy(const utl::Object& rhs)
{
    ASSERTD(rhs.isA(StringInd<T>));
    const StringInd<T>& si = (const StringInd<T>&)rhs;
    gop::Ind::copy(si);
    setString(si._string);
    if (_stringOwner)
    {
        _string = _string->clone();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

template <class T>
utl::String
StringInd<T>::toString() const
{
    std::ostringstream ss;
    ss << _string->toString().get();
    return ss.str().c_str();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

template <class T>
void
StringInd<T>::crossover(StringInd<T>* off1,
                        StringInd<T>* off2,
                        const StringInd<T>& rhs,
                        uint_t pos) const
{
    _string->crossover((off1 == nullptr) ? nullptr : &off1->string(),
                       (off2 == nullptr) ? nullptr : &off2->string(), rhs.string(), pos);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

template <class T>
void
StringInd<T>::init()
{
    _string = new String<T>();
    _stringOwner = true;
    _newString = nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

template <class T>
void
StringInd<T>::deInit()
{
    setString(nullptr);
    deleteNewString();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

GOP_NS_END;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL_TPL(gop::StringInd, T);
