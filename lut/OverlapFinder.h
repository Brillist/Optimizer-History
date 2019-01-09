#ifndef LUT_OVERLAPFINDER_H
#define LUT_OVERLAPFINDER_H

//////////////////////////////////////////////////////////////////////////////

#include <libutl/BufferedFDstream.h>
#include <libutl/Span.h>

//////////////////////////////////////////////////////////////////////////////

LUT_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////
/// OverlapDeltas ////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

/**
   Added/removed objects from previous overlap-set.

   \author Adam McKee
*/

//////////////////////////////////////////////////////////////////////////////

template <class T>
class OverlapDeltas : public utl::Object
{
    UTL_CLASS_DECL_TPL(OverlapDeltas, T);
public:
    typedef std_hash_set< T, HashUint<T> > set_t;
public:
    /** Constructor. */
    OverlapDeltas(utl::uint_t t)
    { init(); _t = t; }

    /** Copy another instance. */
    virtual void copy(const utl::Object& rhs);

    /** Dump a human-readable representation to the given stream. */
    virtual void dump(std::ostream& os) const;

    /** Get time. */
    utl::uint_t t() const
    { return _t; }

    /** Get time. */
    utl::uint_t& t()
    { return _t; }

    /** Empty? */
    bool empty() const
    { return (_addObjects->empty() && _remObjects->empty()); }

    /** Get add-objects. */
    const set_t& addObjects() const
    { return *_addObjects; }

    /** Get rem-objects. */
    const set_t& remObjects() const
    { return *_remObjects; }

    /** Get add-objects. */
    set_t& addObjects();

    /** Get rem-objects. */
    set_t& remObjects();
private:
    void init()
    { _t = 0; _addObjects = _remObjects = &_emptySet; }
    void deInit();
    void dump(std::ostream& os, const set_t& set) const;
private:
    utl::uint_t _t;
    set_t* _addObjects;
    set_t* _remObjects;
    static set_t _emptySet;
};

//////////////////////////////////////////////////////////////////////////////

template <class T>
void
OverlapDeltas<T>::copy(const utl::Object& rhs)
{
    ASSERTD(rhs.isA(OverlapDeltas<T>));
    const OverlapDeltas<T>& od = (const OverlapDeltas<T>&)rhs;
    _t = od._t;
    _addObjects = od._addObjects;
    _remObjects = od._remObjects;
}

//////////////////////////////////////////////////////////////////////////////

template <class T>
void
OverlapDeltas<T>::dump(std::ostream& os) const
{
    os << "t: " << _t << std::endl;
    os << "add: "; dump(os, *_addObjects);
    os << "rem: "; dump(os, *_remObjects);
}

//////////////////////////////////////////////////////////////////////////////

template <class T>
void
OverlapDeltas<T>::dump(std::ostream& os, const set_t& objects) const
{
    typename set_t::const_iterator it;
    for (it = objects.begin(); it != objects.end(); ++it)
    {
        if (it != objects.begin())
        {
            os << ",";
        }
        const T& obj = *it;
        os << obj;
    }
    os << std::endl;
}

//////////////////////////////////////////////////////////////////////////////

template <class T>
typename OverlapDeltas<T>::set_t&
OverlapDeltas<T>::addObjects()
{
    if (_addObjects == &_emptySet)
    {
        _addObjects = new set_t();
    }
    return *_addObjects;
}

//////////////////////////////////////////////////////////////////////////////

template <class T>
typename OverlapDeltas<T>::set_t&
OverlapDeltas<T>::remObjects()
{
    if (_remObjects == &_emptySet)
    {
        _remObjects = new set_t();
    }
    return *_remObjects;
}

//////////////////////////////////////////////////////////////////////////////

template <class T>
void
OverlapDeltas<T>::deInit()
{
    if (_addObjects != &_emptySet)
    {
        delete _addObjects;
    }
    if (_remObjects != &_emptySet)
    {
        delete _remObjects;
    }
}

//////////////////////////////////////////////////////////////////////////////

template <class T>
typename OverlapDeltas<T>::set_t OverlapDeltas<T>::_emptySet;

//////////////////////////////////////////////////////////////////////////////
/// OverlapDeltasOrdering ////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

/**
   Order OverlapDeltas by time.

   \see OverlapDeltas::t
   \author Adam McKee
*/

//////////////////////////////////////////////////////////////////////////////

template <class T>
struct OverlapDeltasOrdering
    : public
      std::binary_function<
         OverlapDeltas<T>*,
         OverlapDeltas<T>*,
         bool>
{
    /** Compare two nodes. */
    bool operator()(
        const OverlapDeltas<T>* lhs,
        const OverlapDeltas<T>* rhs) const
    {
        return (lhs->t() < rhs->t());
    }
};

//////////////////////////////////////////////////////////////////////////////
/// OverlapFinder ////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

/**
   Find spans that overlap with a given span.

   \author Adam McKee
*/

//////////////////////////////////////////////////////////////////////////////

template <class T>
class OverlapFinder : public utl::Object
{
    UTL_CLASS_DECL_TPL(OverlapFinder, T);
    UTL_CLASS_NO_COPY;
public:
    /** Constructor. */
    OverlapFinder(utl::uint_t interval, utl::uint_t numIntervals)
    { init(interval, numIntervals); }

    /** Initialize. */
    void initialize(utl::uint_t interval, utl::uint_t numIntervals);

    /** Copy another instance. */
    virtual void copy(const utl::Object& rhs);

    /** Dump a human-readable representation to the given stream. */
    virtual void dump(std::ostream& os) const;

    /** What objects overlap with the given span? */
    utl::uint_t find(
        utl::uint_t min,
        utl::uint_t max,
        T*& array,
        utl::uint_t& arraySize) const;

    /** Add a span. */
    void add(const T& object, int min, int max);

    /** Remove a span. */
    void remove(const T& object, int min, int max);

    /** Move a previously added span. */
    void move(
        const T& object,
        int oldMin, int oldMax,
        int newMin, int newMax);

    /** Merge two spans into a single span. */
    void merge(
        const T& object,
        int lhsMin, int lhsMax,
        int rhsMin, int rhsMax,
        int newMin, int newMax);

    /** Split a span into two spans. */
    void split(
        const T& object,
        int oldMin, int oldMax,
        int lhsMin, int lhsMax,
        int rhsMin, int rhsMax);
private:
    typedef std_hash_set< T, HashUint<T> > set_t;
    typedef
        std::set< OverlapDeltas<T>*,
        OverlapDeltasOrdering<T> > od_set_t;
private:
    void init()
    { _interval = 0; _numIntervals = 0; _overlapSets = nullptr; }

    void init(utl::uint_t interval, utl::uint_t numIntervals);

    void deInit();

    void findOverlapSet(
        set_t& os,
        typename od_set_t::const_iterator& deltaIt,
        utl::uint_t t) const;

    typename od_set_t::const_iterator deltaFind(utl::uint_t t) const
    { _searchOD.t() = t; return _deltas.find(&_searchOD); }

    typename od_set_t::const_iterator deltaFindLB(utl::uint_t t) const
    { _searchOD.t() = t; return _deltas.lower_bound(&_searchOD); }

    void addFound(
        const set_t& foundObjects,
        utl::uint_t& idx,
        T*& array,
        utl::uint_t& arraySize) const;

    void addMin(const T& object, utl::uint_t min);

    void addMax(const T& object, utl::uint_t max);

    void remMin(const T& object, utl::uint_t min);

    void remMax(const T& object, utl::uint_t max);

    void addOverlap(
        const T& object,
        const utl::Span<int>& span,
        const utl::Span<int>** notSpans,
        const utl::Span<int>** notSpansLim);

    void addOverlap(const T& object, utl::uint_t begin, utl::uint_t end);

    void remOverlap(
        const T& object,
        const utl::Span<int>& span,
        const utl::Span<int>** notSpans,
        const utl::Span<int>** notSpansLim);

    void remOverlap(const T& object, utl::uint_t begin, utl::uint_t end);
private:
    utl::uint_t _interval;
    utl::uint_t _numIntervals;
    set_t** _overlapSets;
    od_set_t _deltas;
    mutable OverlapDeltas<T> _searchOD;
};

//////////////////////////////////////////////////////////////////////////////

template <class T>
void
OverlapFinder<T>::initialize(utl::uint_t interval, utl::uint_t numIntervals)
{
    // already initialized?
    if (_interval != 0)
    {
        return;
    }

    init(interval, numIntervals);
}

//////////////////////////////////////////////////////////////////////////////

template <class T>
void
OverlapFinder<T>::dump(std::ostream& os) const
{
    typename od_set_t::const_iterator it;
    for (it = _deltas.begin(); it != _deltas.end(); ++it)
    {
        OverlapDeltas<T>* node = *it;
        node->dump(os);
        os << "-----------------------------------------------" << std::endl;
    }
}

//////////////////////////////////////////////////////////////////////////////

template <class T>
utl::uint_t
OverlapFinder<T>::find(
    utl::uint_t min,
    utl::uint_t max,
    T*& array,
    utl::uint_t& arraySize) const
{
    return 0;
    if ((min > max) || (_interval == 0))
    {
        return 0;
    }

    // find overlap-set @ min
    set_t os;
    typename od_set_t::const_iterator deltaIt;
    findOverlapSet(os, deltaIt, min);

    // add overlap-set to found array
    utl::uint_t foundIdx = 0;
    addFound(os, foundIdx, array, arraySize);

    // add objects s.t. (min < obj.min <= max)
    typename od_set_t::const_iterator deltasEnd = _deltas.end();
    for (; deltaIt != deltasEnd; ++deltaIt)
    {
        OverlapDeltas<T>* deltas = *deltaIt;
        utl::uint_t dt = deltas->t();
        if (dt > max) break;
        addFound(deltas->addObjects(), foundIdx, array, arraySize);
    }

    return foundIdx;
}

//////////////////////////////////////////////////////////////////////////////

template <class T>
void
OverlapFinder<T>::add(const T& object, int min, int max)
{
    return;
    ASSERTD(min <= max);
    addMin(object, min);
    addMax(object, max);
    addOverlap(object, min, max + 1);
}

//////////////////////////////////////////////////////////////////////////////

template <class T>
void
OverlapFinder<T>::remove(const T& object, int min, int max)
{
    return;
    ASSERTD(min <= max);
    remMin(object, min);
    remMax(object, max);
    remOverlap(object, min, max + 1);
}

//////////////////////////////////////////////////////////////////////////////

template <class T>
void
OverlapFinder<T>::move(
    const T& object,
    int oldMin, int oldMax,
    int newMin, int newMax)
{
    return;
    //DEBUG_CODE
    /*static int numMoves = 0;
    static int64_t minDist = 0;
    static int64_t maxDist = 0;
    minDist += (newMin - oldMin);
    maxDist += (newMax - oldMax);
    ++numMoves;
    if ((numMoves % 100000) == 0)
    {
        utl::cout
            << "average minDist: "
            << (double)minDist / numMoves
            << utl::endl;
        utl::cout
            << "average maxDist: "
            << (double)maxDist / numMoves
            << utl::endl;
    }*/

    ASSERTD(oldMin <= oldMax);
    ASSERTD(newMin <= newMax);

    //DEBUG_CODE
    /*remove(object, oldMin, oldMax);
    add(object, newMin, newMax);
    return;*/

    if (oldMin != newMin)
    {
        remMin(object, oldMin);
    }
    if (oldMax != newMax)
    {
        remMax(object, oldMax);
    }

    if (oldMin != newMin)
    {
        addMin(object, newMin);
    }
    if (oldMax != newMax)
    {
        addMax(object, newMax);
    }

    // find overlap of old/new spans
    utl::Span<int> oldSpan(oldMin, oldMax + 1);
    utl::Span<int> newSpan(newMin, newMax + 1);
    utl::Span<int> overlap = oldSpan.overlap(newSpan);

    // remove part(s) of oldSpan that don't overlap with newSpan
    utl::Span<int> remSpan = oldSpan;
    utl::Span<int> rhsSpan = remSpan.remove(overlap);
    if (!remSpan.isNil())
    {
        remOverlap(object, remSpan.getBegin(), remSpan.getEnd());
        if (!rhsSpan.isNil())
        {
            remOverlap(object, rhsSpan.getBegin(), rhsSpan.getEnd());
        }
    }

    // add part(s) of newSpan that don't overlap with oldSpan
    utl::Span<int> addSpan = newSpan;
    rhsSpan = addSpan.remove(overlap);
    if (!addSpan.isNil())
    {
        addOverlap(object, addSpan.getBegin(), addSpan.getEnd());
        if (!rhsSpan.isNil())
        {
            addOverlap(object, rhsSpan.getBegin(), rhsSpan.getEnd());
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

template <class T>
void
OverlapFinder<T>::merge(
   const T& object,
   int lhsMin, int lhsMax,
   int rhsMin, int rhsMax,
   int newMin, int newMax)
{
    return;
    ASSERTD(lhsMin <= lhsMax);
    ASSERTD(rhsMin <= rhsMax);
    ASSERTD(newMin <= newMax);

    remMin(object, lhsMin);
    remMax(object, lhsMax);
    remMin(object, rhsMin);
    remMax(object, rhsMax);
    addMin(object, newMin);
    addMax(object, newMax);

    utl::Span<int> lhsSpan(lhsMin, lhsMax + 1);
    utl::Span<int> rhsSpan(rhsMin, rhsMax + 1);
    utl::Span<int> newSpan(newMin, newMax + 1);

    const utl::Span<int>* spans[2];
    const utl::Span<int>** spansLim = spans + 1;
    spans[0] = &newSpan;

    // remove part of lhsSpan that doesn't overlap with newSpan
    remOverlap(object, lhsSpan, spans, spansLim);

    // remove part of rhsSpan that doesn't overlap with newSpan
    remOverlap(object, rhsSpan, spans, spansLim);

    spans[0] = &lhsSpan;
    spans[1] = &rhsSpan;
    spansLim = spans + 2;

    // add part of newSpan that doesn't overlap with lhsSpan or rhsSpan
    addOverlap(object, newSpan, spans, spansLim);
}

//////////////////////////////////////////////////////////////////////////////

template <class T>
void
OverlapFinder<T>::split(
   const T& object,
   int oldMin, int oldMax,
   int lhsMin, int lhsMax,
   int rhsMin, int rhsMax)
{
    return;
   ASSERTD(oldMin <= oldMax);
   ASSERTD(lhsMin <= lhsMax);
   ASSERTD(rhsMin <= rhsMax);

   remMin(object, oldMin);
   remMax(object, oldMax);
   addMin(object, lhsMin);
   addMax(object, lhsMax);
   addMin(object, rhsMin);
   addMax(object, rhsMax);

   utl::Span<int> oldSpan(oldMin, oldMax + 1);
   utl::Span<int> lhsSpan(lhsMin, lhsMax + 1);
   utl::Span<int> rhsSpan(rhsMin, rhsMax + 1);

   const utl::Span<int>* spans[2];
   const utl::Span<int>** spansLim = spans + 2;
   spans[0] = &lhsSpan;
   spans[1] = &rhsSpan;

   // remove part of oldSpan that doesn't overlap with lhsSpan or rhsSpan
   remOverlap(object, oldSpan, spans, spansLim);

   spans[0] = &oldSpan;
   spansLim = spans + 1;

   // add part of lhsSpan that doesn't overlap with oldSpan
   addOverlap(object, lhsSpan, spans, spansLim);

   // add part of rhsSpan that doesn't overlap with oldSpan
   addOverlap(object, rhsSpan, spans, spansLim);
}

//////////////////////////////////////////////////////////////////////////////

template <class T>
void
OverlapFinder<T>::init(utl::uint_t interval, utl::uint_t numIntervals)
{
    ASSERTD(interval > 0);
    _interval = interval;
    _numIntervals = numIntervals;
    _overlapSets = new set_t*[_numIntervals];
    memset(_overlapSets, 0, _numIntervals * sizeof(set_t*));
}

//////////////////////////////////////////////////////////////////////////////

template <class T>
void
OverlapFinder<T>::deInit()
{
    set_t** overlapSetsEnd = _overlapSets + _numIntervals;
    deleteArray(_overlapSets, overlapSetsEnd);
    delete [] _overlapSets;

    deleteCont(_deltas);
}

//////////////////////////////////////////////////////////////////////////////

template <class T>
void
OverlapFinder<T>::findOverlapSet(
    set_t& os,
    typename od_set_t::const_iterator& deltaIt,
    utl::uint_t t) const
{
    // os_t : time of last overlap-set
    utl::uint_t os_t = t - (t % _interval);
    // os_idx : index of last overlap-set
    utl::uint_t os_idx = t / _interval;

    // copy an existing overlap-set
    if (_overlapSets[os_idx] != nullptr)
    {
        os = *_overlapSets[os_idx];
    }

    deltaIt = deltaFindLB(os_t + 1);

    // t is a multiple of _interval => we are done
    if (t == os_t)
    {
        return;
    }

    // process deltas
    typename od_set_t::const_iterator deltasEnd = _deltas.end();
    for (; deltaIt != deltasEnd; ++deltaIt)
    {
        const OverlapDeltas<T>* deltas = (*deltaIt);
        utl::uint_t dt = deltas->t();
        if (dt > t) break;

        // remove objects s.t. (obj.max < t)
        const set_t& remSet = deltas->remObjects();
        typename set_t::const_iterator remSetIt;
        typename set_t::const_iterator remSetEnd = remSet.end();
        for (
            remSetIt = remSet.begin();
            remSetIt != remSetEnd;
            ++remSetIt)
        {
            T remObj = *remSetIt;
            os.erase(remObj);
        }

        // add objects s.t. (obj.min < t)
        const set_t& addSet = deltas->addObjects();
        typename set_t::const_iterator addSetIt;
        typename set_t::const_iterator addSetEnd = addSet.end();
        for (
            addSetIt = addSet.begin();
            addSetIt != addSetEnd;
            ++addSetIt)
        {
            T addObj = *addSetIt;
            os.insert(addObj);
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

template <class T>
void
OverlapFinder<T>::addFound(
    const set_t& foundObjects,
    utl::uint_t& idx,
    T*& array,
    utl::uint_t& arraySize) const
{
    utl::uint_t minSize = (idx + foundObjects.size());
    if (arraySize < minSize)
    {
        utl::arrayGrow(array, arraySize, minSize);
    }
    typename set_t::const_iterator objIt;
    typename set_t::const_iterator objEndIt = foundObjects.end();
    for (objIt = foundObjects.begin(); objIt != objEndIt; ++objIt)
    {
        const T& object = *objIt;
        array[idx++] = object;
    }
}

//////////////////////////////////////////////////////////////////////////////

template <class T>
void
OverlapFinder<T>::addMin(const T& object, utl::uint_t min)
{
    typename od_set_t::iterator it = deltaFind(min);
    OverlapDeltas<T>* deltas;
    if (it == _deltas.end())
    {
        deltas = new OverlapDeltas<T>(min);
        _deltas.insert(deltas);
    }
    else
    {
        deltas = *it;
    }
    deltas->addObjects().insert(object);
}

//////////////////////////////////////////////////////////////////////////////

template <class T>
void
OverlapFinder<T>::addMax(const T& object, utl::uint_t max)
{
    typename od_set_t::iterator it = deltaFind(max + 1);
    OverlapDeltas<T>* deltas;
    if (it == _deltas.end())
    {
        deltas = new OverlapDeltas<T>(max + 1);
        _deltas.insert(deltas);
    }
    else
    {
        deltas = *it;
    }
    deltas->remObjects().insert(object);
}

//////////////////////////////////////////////////////////////////////////////

template <class T>
void
OverlapFinder<T>::remMin(const T& object, utl::uint_t min)
{
    typename od_set_t::iterator it = deltaFind(min);
    ASSERTD(it != _deltas.end());
    OverlapDeltas<T>* deltas = *it;
    deltas->addObjects().erase(object);
    if (deltas->empty())
    {
        delete deltas;
        _deltas.erase(it);
    }
}

//////////////////////////////////////////////////////////////////////////////

template <class T>
void
OverlapFinder<T>::remMax(const T& object, utl::uint_t max)
{
    typename od_set_t::iterator it = deltaFind(max + 1);
    ASSERTD(it != _deltas.end());
    OverlapDeltas<T>* deltas = *it;
    deltas->remObjects().erase(object);
    if (deltas->empty())
    {
        delete deltas;
        _deltas.erase(it);
    }
}

//////////////////////////////////////////////////////////////////////////////

template <class T>
void
OverlapFinder<T>::addOverlap(
    const T& object,
    const utl::Span<int>& span,
    const utl::Span<int>** notSpans,
    const utl::Span<int>** notSpansLim)
{
    const utl::Span<int>** notPtr = notSpans;
    const utl::Span<int>* notSpan = (notPtr == notSpansLim) ? nullptr : *notPtr;
    int t = span.getBegin();
    while (t < span.getEnd())
    {
        while ((notSpan != nullptr) && (notSpan->getEnd() <= t))
        {
            ++notPtr;
            notSpan = (notPtr == notSpansLim) ? nullptr : *notPtr;
        }
        if (notSpan == nullptr)
        {
            addOverlap(object, t, span.getEnd());
            return;
        }
        // note: (notSpan.end > t)
        if (notSpan->getBegin() <= t)
        {
            t = notSpan->getEnd();
            continue;
        }
        // note: (t < notSpan.begin < notSpan.end)
        int nextT = utl::min(span.getEnd(), notSpan->getBegin());
        addOverlap(object, t, nextT);
        t = nextT;
    }
}

//////////////////////////////////////////////////////////////////////////////

template <class T>
void
OverlapFinder<T>::addOverlap(
    const T& object,
    utl::uint_t begin,
    utl::uint_t end)
{
    if (begin >= end) return;

    // find index of first and last overlap
    utl::uint_t osIdxBegin = begin / _interval;
    if ((begin % _interval) != 0)
    {
        ++osIdxBegin;
    }
    utl::uint_t osIdxEnd = (end - 1) / _interval;
    ASSERTD(osIdxEnd < _numIntervals);

    // add object to overlap-sets that intersect [begin,end)
    for (utl::uint_t osIdx = osIdxBegin; osIdx <= osIdxEnd; ++osIdx)
    {
        set_t*& os = _overlapSets[osIdx];
        if (os == nullptr)
        {
            os = new set_t();
        }
        os->insert(object);
    }
}

//////////////////////////////////////////////////////////////////////////////

template <class T>
void
OverlapFinder<T>::remOverlap(
    const T& object,
    const utl::Span<int>& span,
    const utl::Span<int>** notSpans,
    const utl::Span<int>** notSpansLim)
{
    const utl::Span<int>** notPtr = notSpans;
    const utl::Span<int>* notSpan = (notPtr == notSpansLim) ? nullptr : *notPtr;
    int t = span.getBegin();
    while (t < span.getEnd())
    {
        while ((notSpan != nullptr) && (notSpan->getEnd() <= t))
        {
            ++notPtr;
            notSpan = (notPtr == notSpansLim) ? nullptr : *notPtr;
        }
        if (notSpan == nullptr)
        {
            remOverlap(object, t, span.getEnd());
            return;
        }
        // note: (notSpan.end > t)
        if (notSpan->getBegin() <= t)
        {
            t = notSpan->getEnd();
            continue;
        }
        // note: (t < notSpan.begin < notSpan.end)
        int nextT = utl::min(span.getEnd(), notSpan->getBegin());
        remOverlap(object, t, nextT);
        t = nextT;
    }
}

//////////////////////////////////////////////////////////////////////////////

template <class T>
void
OverlapFinder<T>::remOverlap(
    const T& object,
    utl::uint_t begin,
    utl::uint_t end)
{
    if (begin >= end) return;

    // find index of first and last overlap
    utl::uint_t osIdxBegin = begin / _interval;
    if ((begin % _interval) != 0)
    {
        ++osIdxBegin;
    }
    utl::uint_t osIdxEnd = (end - 1) / _interval;
    ASSERTD(osIdxEnd < _numIntervals);

    // remove object from overlap-sets that intersect [begin,end)
    for (utl::uint_t osIdx = osIdxBegin; osIdx <= osIdxEnd; ++osIdx)
    {
        set_t* os = _overlapSets[osIdx];
        ASSERTD(os != nullptr);
        os->erase(object);
    }
}

//////////////////////////////////////////////////////////////////////////////

LUT_NS_END;

//////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL_TPL(lut::OverlapDeltas, T, utl::Object);
UTL_CLASS_IMPL_TPL(lut::OverlapFinder, T, utl::Object);

//////////////////////////////////////////////////////////////////////////////

#endif
