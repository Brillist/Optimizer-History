#pragma once

/**
   Various utility classes & functions.
   \file util.h
   \ingroup lut
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <libutl/algorithms_inl.h>
#include <lut/RCobject.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

LUT_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

// forward declarations
template <class Cont> void deleteCont(Cont& cont);
template <class Map> void deleteMapSecond(Map& map);

////////////////////////////////////////////////////////////////////////////////////////////////////

/// \name Flags
///@{
/**
   Toggle a boolean value now and toggle it back upon leaving the current scope (RAII).
   \ingroup lut
*/
class FlagToggle
{
public:
    /**
       Constructor.
       \param val (reference to) boolean value
    */
    FlagToggle(bool& val)
        : _b(val)
    {
        _b = !_b;
    }

    /** Destructor (toggle the value again). */
    ~FlagToggle()
    {
        _b = !_b;
    }

private:
    bool& _b;
};

/**
   Get the flag at the given bit-position.
   \return value of selected bit (as bool)
   \param flags 32-bit flags value
   \param bit selection (0-31)
   \ingroup lut
*/
bool getFlag(uint32_t flags, uint32_t bit);

/**
   Set the flag at the given bit-position.
   \return true iff flag is set
   \param flags 32-bit flags value
   \param bit bit selection (0-31)
   \param val new value for the selected bit
   \ingroup lut
*/
void setFlag(uint32_t& flags, uint32_t bit, bool val);
///@}

////////////////////////////////////////////////////////////////////////////////////////////////////

/// \name STL Container Orderings and Hashes
///@{
/**
   Hash function for keys of type \ref uint_t.
   \ingroup lut
*/
template <class T> class HashUint
{
public:
    size_t
    operator()(T val) const
    {
        return (size_t)val;
    }
};

/**
   Order objects by their compare() method.
   \see utl::Object::compare
   \ingroup lut
*/
struct ObjectOrdering
{
    /** Evaluate to \c true iff lhs is less than rhs. */
    bool
    operator()(const utl::Object* lhs, const utl::Object* rhs) const
    {
        return (lhs->compare(*rhs) < 0);
    }
};

/**
   Order \c std::pair objects by their first objects (using \c < operator).
   \ingroup lut
*/
template <class X, class Y> struct PairFirstOrdering
{
    /** Evaluate to \c true iff lhs is less than rhs. */
    bool
    operator()(const std::pair<X, Y>* lhs, const std::pair<X, Y>* rhs) const
    {
        return (lhs->first < rhs->first);
    }
};

/**
   Order \c std::pair objects by their second objects (using \c < operator).
   \ingroup lut
*/
template <class X, class Y> struct PairSecondOrdering
{
    /** Evalute to \c true iff lhs is less than rhs. */
    bool
    operator()(const std::pair<X, Y>* lhs, const std::pair<X, Y>* rhs) const
    {
        return (lhs->second < rhs->second);
    }
};
///@}

////////////////////////////////////////////////////////////////////////////////////////////////////

/// \name Serialization
///@{
/**
   Serialize a std::string.
   \param str (in/out) \c std::string for serialization
   \param stream stream for serialization
   \param io \ref io_rd or \ref io_wr
   \ingroup lut
*/
void serialize(std::string& str, utl::Stream& stream, uint_t io);

/**
   Serialize a time_t value.
   \param tt (in/out) \c time_t value for serialization
   \param stream stream for serialization
   \param io \ref io_rd or \ref io_wr
   \ingroup lut
*/
void serialize(time_t& tt, utl::Stream& stream, uint_t io);

/**
   Serialize a \c std::map to/from the given stream.
   \param map object for serialization
   \param stream stream for serialization
   \param io \ref io_rd or \ref io_wr
   \ingroup lut
*/
template <class K, class T, class C>
void
serialize(std::map<K, T, C>& map, utl::Stream& stream, uint_t io)
{
    if (io == utl::io_rd)
    {
        map.clear();
        size_t items;
        utl::serialize(items, stream, io);
        for (uint_t i = 0; i != items; ++i)
        {
            K first = K();
            T second = T();
            utl::serialize(first, stream, io, utl::ser_default);
            utl::serialize(second, stream, io, utl::ser_default);
            typename std::map<K, T>::value_type pair(first, second);
            map.insert(pair);
        }
    }
    else
    {
        size_t items = map.size();
        utl::serialize(items, stream, io);
        for (auto& pair : map)
        {
            K first = pair.first;
            T second = pair.second;
            utl::serialize(first, stream, io, utl::ser_default);
            utl::serialize(second, stream, io, utl::ser_default);
        }
    }
}

/**
   Serialize a \c std::set to/from the given stream.
   \param set object for serialization
   \param stream stream for serialization
   \param io \ref io_rd or \ref io_wr
   \ingroup lut
*/
template <class T, class C>
void
serialize(std::set<T, C>& set, utl::Stream& stream, uint_t io)
{
    if (io == utl::io_rd)
    {
        set.clear();
        size_t items;
        utl::serialize(items, stream, io);
        for (size_t i = 0; i != items; ++i)
        {
            T object = T();
            utl::serialize(object, stream, io, utl::ser_default);
            set.insert(object);
        }
    }
    else
    {
        size_t items = set.size();
        utl::serialize(items, stream, io);
        for (T object : set)
        {
            utl::serialize(object, stream, io, utl::ser_default);
        }
    }
}

/**
   Serialize a \c std::vector to/from a stream.
   \param vector object to (de-)serialize
   \param stream stream for serialization
   \param io \ref io_rd or \ref io_wr
   \ingroup lut
*/
template <class T>
void
serialize(std::vector<T>& vector, utl::Stream& stream, uint_t io)
{
    if (io == utl::io_rd)
    {
        vector.clear();
        size_t items;
        utl::serialize(items, stream, io);
        vector.reserve(items);
        for (uint_t i = 0; i < items; ++i)
        {
            T object = T();
            utl::serialize(object, stream, io, utl::ser_default);
            vector.push_back(object);
        }
    }
    else
    {
        size_t items = vector.size();
        utl::serialize(items, stream, io);
        for (T object : vector)
        {
            utl::serialize(object, stream, io, utl::ser_default);
        }
    }
}

/**
   Serialize an object to/from the given stream.
   \param object object to serialize (may be null)
   \param stream stream for serialization
   \param io \ref io_rd or \ref io_wr
   \ingroup lut
*/
template <class T>
void
serializeNullable(T*& object, utl::Stream& stream, uint_t io)
{
    if (io == utl::io_rd)
    {
        serializeInNullable(object, stream);
    }
    else
    {
        serializeOutNullable(object, stream);
    }
}

/**
   Deserialize an object from the given stream.
   \param object (out) received object (may be null)
   \param is input stream for deserialization
   \ingroup lut
*/
template <class T>
void
serializeInNullable(T*& object, utl::Stream& is)
{
    bool objectExists;
    utl::serialize(objectExists, is, utl::io_rd);
    if (objectExists)
    {
        serialize(object, is, utl::io_rd);
    }
    else
    {
        object = nullptr;
    }
}

/**
   Serialize an object to the given stream.
   \param object object to serialize (may be null)
   \param os output stream for serialization
   \ingroup lut
*/
template <class T>
void
serializeOutNullable(T* object, utl::Stream& os)
{
    bool objectExists = (object != nullptr);
    utl::serialize(objectExists, os, utl::io_wr);
    if (objectExists)
    {
        serialize(object, os, utl::io_wr);
    }
}
///@}

////////////////////////////////////////////////////////////////////////////////////////////////////

/// \name Time
///@{
/**
   Get the number of seconds that correspond to a period_t value.
   \return number of seconds in given time period
   \param period time period
   \ingroup lut
*/
uint_t periodToSeconds(period_t period);

/**
   Convert a period_t value to a human-readable string.
   \return a string that specifies the given time period
   \param period time period
   \ingroup lut
*/
std::string periodToString(period_t period);

/**
   Convert a string to the corresponding period_t value.
   \return \ref period_t value for the time period specified by the given string
   \param str string that specifies a time period
   \ingroup lut
*/
period_t periodFromString(const std::string& str);

/**
   Get date for a given time.
   \return time_t value for beginning of the day at time t
   \param tt input time
   \ingroup lut
*/
time_t time_date(time_t tt);

/**
   Get number of seconds since start of day for given time.
   \return number of seconds since start of the day at time t
   \param tt input time
   \ingroup lut
*/
uint_t time_timeOfDay(time_t tt);

/**
   Get day-of-week (sunday=0) for given time.
   \return day-of-week for time tt
   \param tt input time
   \ingroup lut
*/
uint_t time_dayOfWeek(time_t tt);

/**
   Get a string representation of a time_t value.
   \return string representation of tt
   \param tt input time
   \ingroup lut
*/
std::string time_str(time_t tt);
///@}

////////////////////////////////////////////////////////////////////////////////////////////////////

/// \name Reference-Counting
///@{
/**
   Add a reference to all \ref RCobject derived objects in the container.
   \param cont operand
   \ingroup lut
*/
template <class Cont>
void
addRefCont(Cont& cont)
{
    for (auto obj : cont)
    {
        obj->addRef();
    }
}

/**
   Decrement the reference count of all objects in the given array.
   \param array array of pointers to \ref RCobject derived types
   \param ptr end pointer (one past the last pointer in array)
   \ingroup lut
*/
template <class T>
void
removeRefArray(T** array, T**& ptr)
{
    while (ptr > array)
    {
        T* object = *--ptr;
        object->removeRef();
    }
}

/**
   Decrement the reference count of all objects in the given container & clear it.
   \param cont operand container
   \ingroup lut
*/
template <class Cont>
void
removeRefCont(Cont& cont)
{
    for (auto obj : cont)
    {
        obj->removeRef();
    }
    cont.clear();
}

/**
   Decrement the reference count of the key values in the given map & clear it.
   \param map operand \c std::map
   \ingroup lut
*/
template <class Map>
void
removeRefMapFirst(Map& map)
{
    typename Map::iterator it;
    for (it = map.begin(); it != map.end(); ++it)
    {
        ((*it).first)->removeRef();
    }
    map.clear();
}
///@}

////////////////////////////////////////////////////////////////////////////////////////////////////

/// \name Copying
///@{
/**
   Add objects in the source container to the destination container (without cloning).
   \param dst container that receives objects
   \param src container that objects are copied from
   \ingroup lut
*/
template <class Cont>
void
addItems(Cont& dst, const Cont& src)
{
    for (auto obj : src)
    {
        dst.insert(obj);
    }
}

/**
   Create and return a copy of the given object.
   \return clone of obj (null if obj is null)
   \param obj object to clone (may be null)
   \ingroup lut
*/
template <class T>
T*
clone(const T* obj)
{
    if (obj == nullptr)
    {
        return nullptr;
    }
    else
    {
        return obj->clone();
    }
}

/**
   Create a managed copy of the given object.
   \return pointer to a new managed copy of *ptr
   \ingroup lut
*/
template <class T>
T*
mclone(T* ptr)
{
    if (ptr == nullptr)
    {
        return nullptr;
    }
    else
    {
        return ptr->mclone();
    }
}

/**
   Add copies of objects in the source \c std::map to dst.
   \param dst destination \c std::map
   \param src source \c std::map
   \ingroup lut
*/
template <class Map>
void
copyMap(Map& dst, const Map& src)
{
    deleteMapSecond(dst);
    for (auto pair : src)
    {
        dst.insert(std::make_pair(pair.first, pair.second->clone()));
    }
}

/**
   Add copies of objects in the source \c std::set to the destination \c std::set.
   \param dst destination \c std::set
   \param src source \c std::set
   \ingroup lut
*/
template <class Set>
void
copySet(Set& dst, const Set& src)
{
    deleteCont(dst);
    for (auto obj : src)
    {
        obj = obj->clone();
        dst.insert(obj);
    }
}

/**
   Add copies of objects in the source \c std::vector to the destination \c std::vector.
   \param dst destination \c std::vector
   \param src source \c std::vector
   \ingroup lut
*/
template <class Vector>
void
copyVector(Vector& dst, const Vector& src)
{
    deleteCont(dst);
    for (auto obj : src)
    {
        obj = obj->clone();
        dst.push_back(obj);
    }
}
///@}

////////////////////////////////////////////////////////////////////////////////////////////////////

/// \name Comparing
///@{
/**
   Determine whether two objects are equal.
   \return true if the objects are equal, false otherwise
   \param lhs left-hand-side object
   \param rhs right-hand-side object
   \ingroup lut
*/
template <class T>
bool
equals(const T* lhs, const T* rhs)
{
    if (lhs == rhs)
    {
        return true;
    }
    if ((lhs == nullptr) || (rhs == nullptr))
    {
        return false;
    }
    return lhs->equals(*rhs);
}

/**
   Compare the two given objects.
   \return 0 if lhs = rhs, < 0 if lhs < rhs, > 0 if lhs > rhs
   \ingroup lut
*/
template <class T>
int
compare(const T& lhs, const T& rhs)
{
    if (lhs < rhs)
    {
        return -1;
    }
    else if (lhs == rhs)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

/**
   Specialization of compare() for floating-point.
   \ingroup lut
*/
template <> int compare(const double& lhs, const double& rhs);

/**
   Compare two sorted collections (using utl::compare to compare their items).
   \return 0 if lhs = rhs, < 0 if lhs < rhs, > 0 if lhs > rhs
   \ingroup lut
*/
template <class Cont>
int
compareItems(const Cont& lhs, const Cont& rhs)
{
    int res = compare(lhs.size(), rhs.size());
    if (res != 0)
        return res;

    auto lhsEnd = lhs.end();
    for (auto lhsIt = lhs.begin(), rhsIt = rhs.begin(); lhsIt != lhsEnd; ++lhsIt, ++rhsIt)
    {
        auto& lhs = *lhsIt;
        auto& rhs = *rhsIt;
        int res = utl::compare(lhs, rhs);
        if (res != 0)
            return res;
    }

    return 0;
}
///@}

////////////////////////////////////////////////////////////////////////////////////////////////////

/// \name Array/Container Cleanup
///@{
/**
   Delete all objects in the given array.
   \param array array of object pointers
   \param ptr end pointer (one past the last pointer)
   \ingroup lut
*/
template <class T>
void
deleteArray(T** array, T**& ptr)
{
    while (ptr-- > array)
    {
        T* object = *ptr;
        delete object;
    }
}

/**
   Delete all objects in the given container, and clear the container.
   \param cont operand
   \ingroup lut
*/
template <class Cont>
void
deleteCont(Cont& cont)
{
    for (auto object : cont)
    {
        delete object;
    }
    cont.clear();
}

/**
   Delete the second object of each \c std::pair in the given \c std::map, and clear it.
   \param map operand
   \ingroup lut
*/
template <class Map>
void
deleteMapSecond(Map& map)
{
    for (auto pair : map)
    {
        delete pair.second;
    }
    map.clear();
}
///@}

////////////////////////////////////////////////////////////////////////////////////////////////////

/// \name Text Output
///@{
/**
   Create a text heading.
   \param title heading title
   \param ch repeat character (e.g. "=== title =======...")
   \param width heading width
   \ingroup lut
*/
std::string heading(const std::string& title, char ch, uint_t width);

/**
   Dump a human-readable representation of the objects in the given container to a stream.
   \ingroup lut
*/
template <class Cont>
void
dumpCont(Cont& cont, std::ostream& os)
{
    typename Cont::iterator it;
    for (it = cont.begin(); it != cont.end(); ++it)
    {
        typename Cont::value_type object = *it;
        object->dump(os);
    }
}
//@}

////////////////////////////////////////////////////////////////////////////////////////////////////

/// \name Miscellaneous
///@{
/**
   Create a new PRNG based on \c std::mt19937 (aka "Mersenne Twister").
   \return new instance of rng_t
   \ingroup lut
*/
inline rng_t*
make_rng()
{
#ifdef DEBUG
    return new rng_t(1042614900);
#else
    return new rng_t(1042614900);
#endif
}

/**
   A wrapper for \c std::set<T>::find() that returns null if no match is found.
   \return found object of type T (or null if not found)
   \param set \c std::set to search
   \param key search key
   \ingroup lut
*/
template <class T, class L>
T
setFind(std::set<T, L>& set, T key)
{
    typename std::set<T>::iterator it = set.find(key);
    if (it == set.end())
        return nullptr;
    return *it;
}

/**
   Resize an array that was previously allocated with malloc.
   \param ptr address of allocation
   \param oldSize old size (in bytes)
   \param newSize new size (in bytes)
   \ingroup lut
*/
void* realloc(void* ptr, size_t oldSize, size_t newSize);

/**
   Read a line of text from the given \c std::istream.
   \return line of text (as \c std::string)
   \param is input stream
   \ingroup lut
*/
std::string readLine(std::istream& is);

/**
   Return a copy of the given string converted to lowercase.
   \return a string equivalent to str except that uppercase characters are converted to lowercase
   \param str input string
   \ingroup lut
*/
std::string tolower(const std::string& str);
///@}

////////////////////////////////////////////////////////////////////////////////////////////////////

LUT_NS_END;
