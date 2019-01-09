#ifndef LUT_UTIL_H
#define LUT_UTIL_H

//////////////////////////////////////////////////////////////////////////////

#include <libutl/algorithms_inl.h>
#include <libutl/RandNumGen.h>
#include <lut/RCobject.h>

//////////////////////////////////////////////////////////////////////////////

LUT_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////

template <class T>
void serializeNullable(T* object, utl::Stream& stream, utl::uint_t io)
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

//////////////////////////////////////////////////////////////////////////////

template <class T>
void serializeInNullable(T*& object, utl::Stream& is)
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

//////////////////////////////////////////////////////////////////////////////

template <class T>
void serializeOutNullable(T* object, utl::Stream& os)
{
    bool objectExists = (object != nullptr);
    utl::serialize(objectExists, os, utl::io_wr);
    if (objectExists)
    {
        serialize(object, os, utl::io_wr);
    }
}

//////////////////////////////////////////////////////////////////////////////

template < class ST1, class ST2, class F, class S >
void serialize(std::map<F,S>& map, utl::Stream& stream, utl::uint_t io)
{
    if (io == utl::io_rd)
    {
        map.clear();
        utl::uint_t items;
        utl::serialize(items, stream, io);
        for (utl::uint_t i = 0; i < items; ++i)
        {
            ST1 first = 0;
            utl::serialize(first, stream, io, utl::ser_default);

/*             F first; */
/*             utl::serialize(first, stream, io, utl::ser_default); */

            ST2 second = 0;
            utl::serialize(second, stream, io, utl::ser_default);
            typename std::map<F,S>::value_type pair((F&)first, (S&)second);
/*             typename std::map<F,S>::value_type pair(first, (S&)second); */
            map.insert(pair);
        }
    }
    else
    {
        utl::uint_t items = map.size();
        utl::serialize(items, stream, io);
        typename std::map<F,S>::iterator it;
        for (it = map.begin(); it != map.end(); ++it)
        {
/*             F first = (*it).first; */
            ST1 first = (*it).first;
            utl::serialize(first, stream, io, utl::ser_default);
            ST2 second = (*it).second;
            utl::serialize(second, stream, io, utl::ser_default);
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

template < class ST, class T, class L >
void serialize(std::set<T,L>& set, utl::Stream& stream, utl::uint_t io)
{
    if (io == utl::io_rd)
    {
        set.clear();
        utl::uint_t items;
        utl::serialize(items, stream, io);
        for (utl::uint_t i = 0; i < items; ++i)
        {
            ST object = 0;
            utl::serialize(
                object,
                stream,
                io,
                utl::ser_default);
            set.insert((T&)object);
        }
    }
    else
    {
        utl::uint_t items = set.size();
        utl::serialize(items, stream, io);
        typename std::set<T,L>::iterator it;
        for (it = set.begin(); it != set.end(); ++it)
        {
            ST object = *it;
            utl::serialize(
                object,
                stream,
                io,
                utl::ser_default);
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

template <class ST, class T>
void serialize(std::vector<T>& vector, utl::Stream& stream, utl::uint_t io)
{
    if (io == utl::io_rd)
    {
        vector.clear();
        utl::uint_t items;
        utl::serialize(items, stream, io);
        vector.reserve(items);
        for (utl::uint_t i = 0; i < items; ++i)
        {
            ST object = 0;
            utl::serialize(
                object,
                stream,
                io,
                utl::ser_default);
            vector.push_back((T&)object);
        }
    }
    else
    {
        utl::uint_t items = vector.size();
        utl::serialize(items, stream, io);
        typename std::vector<T>::iterator it;
        for (it = vector.begin(); it != vector.end(); ++it)
        {
            ST object = *it;
            utl::serialize(
                object,
                stream,
                io,
                utl::ser_default);
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

void serialize(std::string& str, utl::Stream& stream, utl::uint_t io);

//////////////////////////////////////////////////////////////////////////////

void serialize(time_t& t, utl::Stream& stream, utl::uint_t io);

//////////////////////////////////////////////////////////////////////////////

class FlagToggle
{
public:
    FlagToggle(bool& b)
        : _b(b) { _b = !_b; }

        ~FlagToggle()
        { _b = !_b; }
private:
        bool& _b;
};

//////////////////////////////////////////////////////////////////////////////

template <class T>
class HashUint
{
public:
    size_t operator()(T val) const
    { return (size_t)val; }
};

//////////////////////////////////////////////////////////////////////////////

std::string heading(const std::string& title, char ch, utl::uint_t width);

//////////////////////////////////////////////////////////////////////////////

std::string getTimeString(time_t t);

//////////////////////////////////////////////////////////////////////////////

/**
   Convert a string to the corresponding period_t value.
*/
period_t stringToPeriod(const std::string& str);

//////////////////////////////////////////////////////////////////////////////

/**
   Convert a period_t value to a human-readable string.
*/
std::string periodToString(period_t period);

//////////////////////////////////////////////////////////////////////////////

/**
   Get the number of seconds that correspond to a period_t value.
*/
utl::uint_t periodToSeconds(period_t period);

//////////////////////////////////////////////////////////////////////////////

/**
   Grow an array that was previously allocated with malloc.
*/
void* realloc(void* ptr, utl::uint_t oldSize, utl::uint_t newSize);

//////////////////////////////////////////////////////////////////////////////

/**
   Get date for a given time.
*/
time_t
time_date(time_t t);

//////////////////////////////////////////////////////////////////////////////

/**
   Get number of seconds since start of day for given time.
*/
utl::uint_t
time_timeOfDay(time_t t);

//////////////////////////////////////////////////////////////////////////////

/**
   Get day-of-week (sunday=0) for given time.
*/
utl::uint_t
time_dayOfWeek(time_t t);

//////////////////////////////////////////////////////////////////////////////

/**
   Get a string representation of a date/time.
*/
std::string
time_str(time_t t);

//////////////////////////////////////////////////////////////////////////////

/**
   Get the flag at the given bit-position.
*/
bool
getFlag(utl::uint32_t flags, utl::uint32_t bit);

//////////////////////////////////////////////////////////////////////////////

/**
   Set the flag at the given bit-position.
*/
void
setFlag(utl::uint32_t& flags, utl::uint32_t bit, bool val);

//////////////////////////////////////////////////////////////////////////////

/**
   Create and return a copy of the given object.
   \author Adam McKee
*/
template <class T>
T*
clone(const T* ptr)
{
    if (ptr == nullptr)
    {
        return nullptr;
    }
    else
    {
        return ptr->clone();
    }
}

//////////////////////////////////////////////////////////////////////////////

/**
   Create a managed copy of the given object.
   \author Adam McKee
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

//////////////////////////////////////////////////////////////////////////////

/**
   Compare the two given objects.
   \return 0 if lhs = rhs, < 0 if lhs < rhs, > 0 if lhs > rhs
   \author Adam McKee
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

//////////////////////////////////////////////////////////////////////////////

/**
   Specialization of compare() for floating-point.
   \author Adam McKee
*/
template <>
int
compare(const double& lhs, const double& rhs);

//////////////////////////////////////////////////////////////////////////////

/**
   STL less-than ordering based on compare() method of objects.
   \author Adam McKee
*/
template <class Object>
struct ObjectSTLordering
{
    /** Compare two objects. */
    bool operator()(const Object* lhs, const Object* rhs) const
    {
        return (lhs->compare(*rhs) < 0);
    }
};

//////////////////////////////////////////////////////////////////////////////

/**
   Compare the two sorted collections.
   \return 0 if lhs = rhs, < 0 if lhs < rhs, > 0 if lhs > rhs
   \author Adam McKee
*/
template <class Cont>
int
compareContValues(const Cont& lhs, const Cont& rhs)
{
    int res = compare(lhs.size(), rhs.size());
    if (res != 0) return res;

    typename Cont::const_iterator lhsIt;
    typename Cont::const_iterator rhsIt;
    typename Cont::const_iterator lhsEnd = lhs.end();
    for (lhsIt = lhs.begin(), rhsIt = rhs.begin();
         lhsIt != lhsEnd;
         ++lhsIt, ++rhsIt)
    {
        const typename Cont::value_type& lhsVal = *lhsIt;
        const typename Cont::value_type& rhsVal = *rhsIt;
        int res = compare(lhsVal, rhsVal);
        if (res != 0) return res;
    }

    return 0;
}

//////////////////////////////////////////////////////////////////////////////

/**
   Compare objects in two sorted collections (using their compare() method).
   \return 0 if lhs = rhs, < 0 if lhs < rhs, > 0 if lhs > rhs
   \author Adam McKee
*/
template <class Cont>
int
compareContObjects(const Cont& lhs, const Cont& rhs)
{
    int res = compare(lhs.size(), rhs.size());
    if (res != 0) return res;

    typename Cont::const_iterator lhsIt;
    typename Cont::const_iterator rhsIt;
    typename Cont::const_iterator lhsEnd = lhs.end();
    for (lhsIt = lhs.begin(), rhsIt = rhs.begin();
         lhsIt != lhsEnd;
         ++lhsIt, ++rhsIt)
    {
        const typename Cont::value_type& lhsObject = *lhsIt;
        const typename Cont::value_type& rhsObject = *rhsIt;
        int res = lhsObject->compare(*rhsObject);
        if (res != 0) return res;
    }

    return 0;
}

//////////////////////////////////////////////////////////////////////////////

/**
   Delete all objects in the given container, and clear the container.
   \author Adam McKee
*/
template <class Cont>
void deleteCont(Cont& cont)
{
    typename Cont::iterator it;
    for (it = cont.begin(); it != cont.end(); ++it)
    {
        typename Cont::value_type object = *it;
        delete object;
    }
    cont.clear();
}

//////////////////////////////////////////////////////////////////////////////

/**
   Delete all objects in the given container, and clear the container.
   \author Adam McKee
*/
template <class K, class V>
    void deleteMapSecond(std_hash_map<K,V>& map)
{
    typename std_hash_map<K,V>::iterator it;
    for (it = map.begin(); it != map.end(); ++it)
    {
        typename std_hash_map<K,V>::value_type object = *it;
        delete object.second;
    }
    map.clear();
}

//////////////////////////////////////////////////////////////////////////////

/**
   Delete all objects in the given container, and clear the container.
   \author Adam McKee
*/
template <class K, class V, class Ord>
    void deleteMapSecond(std::map<K,V,Ord>& map)
{
    typename std::map<K,V,Ord>::iterator it;
    for (it = map.begin(); it != map.end(); ++it)
    {
        typename std::map<K,V,Ord>::value_type object = *it;
        delete object.second;
    }
    map.clear();
}

//////////////////////////////////////////////////////////////////////////////

/*template <class Map>
  void deleteMapSecond(Map& map)
  {
  typename Map::iterator it;
  for (it = map.begin(); it != map.end(); ++it)
  {
  typename Map::value_type object = *it;
  delete object.second;
  }
  map.clear();
  }*/

//////////////////////////////////////////////////////////////////////////////

/**
   Delete all objects in the given array.
   \author Adam McKee
*/
template <class T>
void deleteArray(T** array, T**& ptr)
{
    while (ptr-- > array)
    {
        T* object = *ptr;
        delete object;
    }
}

//////////////////////////////////////////////////////////////////////////////

/**
   Determine whether two objects are equal.
   \return true if the objects are equal, false otherwise
   \param lhs left-hand-side object
   \param rhs right-hand-side object
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

    bool res = lhs->equals(*rhs);
    return res;
}

//////////////////////////////////////////////////////////////////////////////

/**
   Erase the object referred to by the given iterator,
   and increment the iterator.
   \author Adam McKee
*/
template <class Cont, class Iter>
    void erase(Cont& cont, Iter& it)
{
    cont.erase(it++);
}

//////////////////////////////////////////////////////////////////////////////

/**
   Add copies of objects in the src vector to the dst vector.
   \see clone
   \author Adam McKee
*/
template <class T>
void copyVector(std::vector<T>& dst, const std::vector<T>& src)
{
    deleteCont(dst);
    typename std::vector<T>::const_iterator it;
    for (it = src.begin(); it != src.end(); ++it)
    {
        T object = *it;
        dst.push_back(object->clone());
    }
}

//////////////////////////////////////////////////////////////////////////////

/**
   Add copies of objects in the src-set to the dst-set.
   \see clone
   \author Adam McKee
*/
template <class Set>
void copySet(Set& dst, const Set& src)
{
    deleteCont(dst);
    typename Set::const_iterator it;
    for (it = src.begin(); it != src.end(); ++it)
    {
        typename Set::value_type object = *it;
        object = object->clone();
        dst.insert(object);
    }
}

//////////////////////////////////////////////////////////////////////////////

template <class T, class C>
    void copyCollection(std::vector<T>& vect, const C& col)
{
    vect.clear();
    typename C::iterator it;
    for (it = col.begin(); it != col.end(); ++it)
    {
        T object = (T)*it;
        vect.push_back(object);
    }
}

//////////////////////////////////////////////////////////////////////////////

/**
   Copy items from one collection into another.
   \author Adam McKee
*/
template <class Cont>
void addItems(Cont& dst, const Cont& src)
{
    typename Cont::const_iterator it;
    for (it = src.begin(); it != src.end(); ++it)
    {
        typename Cont::value_type object = *it;
        dst.insert(object);
    }
}

//////////////////////////////////////////////////////////////////////////////

/**
   Copy all objects in the given container.
   \author Howard Lee
*/
template <class Map>
void copyMap(Map& dst, const Map& src)
{
    deleteMapSecond(dst);
    typename Map::const_iterator it;
    for (it = src.begin(); it != src.end(); ++it)
    {
        typename Map::value_type object = *it;
        dst.insert(std::make_pair(object.first,object.second->clone()));
    }
}

//////////////////////////////////////////////////////////////////////////////

/**
   Find a unique key in the given map.
   \author Adam McKee
*/
template <class Map>
utl::uint_t uniqueKey(const Map& map)
{
    utl::uint_t v = 1;
    typename Map::const_iterator it;
    for (it = map.begin(); it != map.end(); ++it)
    {
        utl::uint_t itKey = (*it).first;
        if (itKey == v) return v;
        v = (itKey + 1);
    }
    return v;
}

//////////////////////////////////////////////////////////////////////////////

template <class T>
void
addRef(T* object)
{
    RCobject* rcobject = dynamic_cast<RCobject*>(object);
    if (rcobject == nullptr)
    {
        return;
    }
    rcobject->addRef();
}

//////////////////////////////////////////////////////////////////////////////

template <class T>
void
removeRef(T* object)
{
    RCobject* rcobject = dynamic_cast<RCobject*>(object);
    if (rcobject == nullptr)
    {
        return;
    }
    rcobject->removeRef();
}

//////////////////////////////////////////////////////////////////////////////

/**
   Decrement the reference count of all objects in the given container,
   and clear the container.
   \author Adam McKee
*/
template <class Cont>
void removeRefCont(Cont& cont)
{
    typename Cont::iterator it;
    for (it = cont.begin(); it != cont.end(); ++it)
    {
        typename Cont::value_type object = *it;
        removeRef(object);
    }
    cont.clear();
}

//////////////////////////////////////////////////////////////////////////////

/**
   Decrement the reference count of all objects in the given array.
   \author Adam McKee
*/
template <class T>
void removeRefArray(T** array, T**& ptr)
{
    while (ptr-- > array)
    {
        T* object = *ptr;
        removeRef(object);
    }
}

//////////////////////////////////////////////////////////////////////////////

/**
   Decrement the reference count of the key values in the given map,
   and clear the map.
   \author Adam McKee
*/
template <class Map>
void removeRefMapFirst(Map& map)
{
    typename Map::iterator it;
    for (it = map.begin(); it != map.end(); ++it)
    {
        removeRef((*it).first);
    }
    map.clear();
}

//////////////////////////////////////////////////////////////////////////////

/**
   Dump a human-readable representation of the objects in the given container
   to the given output stream.
   \author Adam McKee
*/
template <class Cont>
void dumpCont(Cont& cont, std::ostream& os)
{
    typename Cont::iterator it;
    for (it = cont.begin(); it != cont.end(); ++it)
    {
        typename Cont::value_type object = *it;
        object->dump(os);
    }
}

//////////////////////////////////////////////////////////////////////////////

/**
   Randomly shuffle the objects in the given vector,
   using the specified PRNG.
   \author Adam McKee
*/
template <class T>
void shuffle(std::vector<T>& vect, utl::RandNumGen& rng)
{
    utl::uint_t size = vect.size();
    // randomly shuffle
    for (utl::uint_t i = 1; i < size; i++)
    {
        utl::uint_t randOffset = rng.evali(i + 1);
        std::swap(vect[randOffset], vect[i]);
    }
}

//////////////////////////////////////////////////////////////////////////////

/**
   Randomly shuffle the objects in the given array,
   using the specified PRNG.
   \author Adam McKee
*/
template <class T>
void shuffle(T* array, utl::uint_t size, utl::RandNumGen& rng)
{
    // randomly shuffle
    for (utl::uint_t i = 1; i < size; i++)
    {
        utl::uint_t randOffset = rng.evali(i + 1);
        std::swap(array[randOffset], array[i]);
    }
}

//////////////////////////////////////////////////////////////////////////////

/**
   Read a line of input from the given stream.
   \author Adam McKee
*/
std::string readLine(std::istream& is);

//////////////////////////////////////////////////////////////////////////////

/**
   Return a copy of the given string, with all alphabetic characters
   converted to lower-case.
   \author Adam McKee
*/
std::string tolower(const std::string& str);

//////////////////////////////////////////////////////////////////////////////

/**
   Ordering via utl::Object::compare().
   \author Adam McKee
*/

struct UTLordering
    : public std::binary_function<utl::Object*, utl::Object*, bool>
{
    bool operator()(
        const utl::Object* lhs,
        const utl::Object* rhs) const
    {
        return (lhs->compare(*rhs) < 0);
    }
};

//////////////////////////////////////////////////////////////////////////////

/**
   Ordering by first element of a pair.
   \author Adam McKee
*/
template <class X, class Y>
struct PairFirstOrdering
    : public std::binary_function<std::pair<X,Y>*,std::pair<X,Y>*,bool>
{
    bool operator()(
        const std::pair<X,Y>* lhs,
        const std::pair<X,Y>* rhs) const
    {
        return (lhs->first < rhs->first);
    }
};

//////////////////////////////////////////////////////////////////////////////

/**
   Ordering by second element of a pair.
   \author Adam McKee
*/
template <class X, class Y>
struct PairSecondOrdering
    : public std::binary_function<std::pair<X,Y>*,std::pair<X,Y>*,bool>
{
    bool operator()(
        const std::pair<X,Y>* lhs,
        const std::pair<X,Y>* rhs) const
    {
        return (lhs->second < rhs->second);
    }
};

//////////////////////////////////////////////////////////////////////////////

template < class T, class L >
T setFind(std::set<T,L>& set, T t)
{
    typename std::set<T>::iterator it
        = set.find(t);
    if (it == set.end()) return nullptr;
    return *it;
}

//////////////////////////////////////////////////////////////////////////////

#define forEachIt(colType, col, itemType, item)                         \
    {                                                                   \
        colType::iterator it__ = col.begin();                           \
        for (it__ = col.begin(); ; ++it__)                              \
        {                                                               \
            utl::Object* obj__ = *it__;                                 \
            if (obj__ == nullptr) break;                                \
            ASSERTD(obj__->isA(itemType));                              \
            itemType& item = (itemType&)*obj__;

//////////////////////////////////////////////////////////////////////////////

#define endForEach                                                      \
        }                                                               \
    }

//////////////////////////////////////////////////////////////////////////////

LUT_NS_END;

//////////////////////////////////////////////////////////////////////////////

#endif
