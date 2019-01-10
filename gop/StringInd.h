#ifndef GOP_STRINGIND_H
#define GOP_STRINGIND_H

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <gop/String.h>
#include <gop/Ind.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

GOP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Individual with chromosome (abstract).

   StringInd is an abstract base for individuals that have a string
   chromosome representation.

   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

template <class T> class StringInd : public gop::Ind
{
    UTL_CLASS_DECL_TPL(StringInd, T);

public:
    /** Constructor. */
    StringInd(utl::uint_t size)
    {
        init();
        _string->setSize(size);
    }

    virtual void copy(const utl::Object& rhs);

    virtual utl::String toString() const;

    /** Get the String size. */
    utl::uint_t
    size() const
    {
        return _string->size();
    }

    /** Set the size. */
    void
    setSize(utl::uint_t size)
    {
        _string->setSize(size);
    }

    /** Get the String (const). */
    const String<T>&
    string() const
    {
        return *_string;
    }

    /** Get the String. */
    String<T>&
    string()
    {
        return *_string;
    }

    /** Get string pointer. */
    const String<T>*
    getString() const
    {
        return _string;
    }

    /** Get string pointer. */
    String<T>*
    getString()
    {
        return _string;
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

    /** Get the new String (const). */
    const String<T>*
    newString() const
    {
        return _newString;
    }

    /** Get the new String. */
    String<T>*
    newString()
    {
        return _newString;
    }

    /** delete the new string. */
    void
    deleteNewString()
    {
        delete _newString;
        _newString = nullptr;
    }

    /** create the new string. */
    void
    createNewString()
    {
        if (_newString)
            deleteNewString();
        _newString = _string->clone();
    }

    /** replace the string with the new string. */
    void
    acceptNewString()
    {
        _string->copy(*_newString);
        deleteNewString();
    }

    /**
       Perform a crossover.
       \param off1 offspring 1
       \param off2 offspring 2
       \param rhs rhs String (lhs is self)
       \param pos crossover position
    */
    void
    crossover(StringInd<T>* off1, StringInd<T>* off2, const StringInd<T>& rhs, utl::uint_t pos);

    /** Get a gene. */
    const T&
    get(utl::uint_t idx)
    {
        return (*_string)[idx];
    }

    /** Set a gene. */
    void
    set(utl::uint_t pos, utl::uint_t val)
    {
        (*_string)[pos] = val;
    }

    /** Get the gene at the given index (const). */
    const T& operator[](utl::uint_t idx) const
    {
        return (*_string).operator[](idx);
    }

    /** Get the gene at the given index. */
    T& operator[](utl::uint_t idx)
    {
        return (*_string).operator[](idx);
    }

    /** Override from gop::Ind. */
    virtual bool
    isBuilt() const
    {
        return false;
    }

    /** Override from gop::Ind. */
    virtual void
    buildClear()
    {
    }

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
                        utl::uint_t pos)
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
    _newString = nullptr;
    _stringOwner = true;
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

UTL_CLASS_IMPL_TPL(gop::StringInd, T, gop::Ind);

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
