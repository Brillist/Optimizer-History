#include "libclp.h"
#include <libutl/algorithms_inl.h>
#include "IntExpDomainCARit.h"
#include "IntExpDomainCAR.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef DEBUG
//#define DEBUG_UNIT
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(clp::IntExpDomainCAR);

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

IntExpDomainCAR::IntExpDomainCAR(Manager* mgr)
    : IntExpDomain(mgr)
{
    init();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

IntExpDomainCAR::IntExpDomainCAR(Manager* mgr, const int_uint_map_t& domain)
    : IntExpDomain(mgr)
{
    init(domain);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
IntExpDomainCAR::copy(const Object& rhs)
{
    auto& dcar = utl::cast<IntExpDomainCAR>(rhs);
    super::copy(dcar);
    _num = dcar._num;
    _bits = dcar._bits;
    _bitsLog2 = dcar._bitsLog2;
    _countsPerWord = dcar._countsPerWord;
    _countsPerWordLog2 = dcar._countsPerWordLog2;
    _countsPerWordMask = dcar._countsPerWordMask;
    _mask = dcar._mask;
    _valuesArray = dcar._valuesArray;
    _countsArray = dcar._countsArray;
    _values = _valuesArray.get();
    _counts = _countsArray.get();
    _stateDepth = 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

uint_t
IntExpDomainCAR::decrement(int val, uint_t num)
{
    ASSERTD(num > 0);
    uint_t idx =
        utl::binarySearch(_valuesArray.get(), 0, _num, val, utl::subtract<int>(), utl::find_any);
    ASSERTD(idx != uint_t_max);

    uint_t wordIdx = idx >> _countsPerWordLog2;
    uint_t countIdx = idx & _countsPerWordMask;
    uint_t maskShift = (countIdx << _bitsLog2);
    uint32_t mask = _mask << maskShift;

    // get old count
    uint32_t& word = _counts[wordIdx];
    uint_t count = (word & mask) >> maskShift;
    ASSERTD(count >= num);

    // remove val from domain?
    if (count == num)
    {
        remove(val);
        return 0;
    }

    // otherwise: (count > num) so just decrement it
    ASSERTD(count > num);
    word &= ~mask;
    count -= num;
    mask = count << maskShift;
    word |= mask;
    return count;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool
IntExpDomainCAR::has(int val) const
{
    uint_t idx =
        utl::binarySearch(_valuesArray.get(), 0, _num, val, utl::subtract<int>(), utl::find_any);
    if (idx == uint_t_max)
    {
        return false;
    }
    return (getCount(idx) != 0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

IntExpDomainIt*
IntExpDomainCAR::begin() const
{
    if (_size == 0)
        return end();
    IntExpDomainCARit* it = new IntExpDomainCARit(this, uint_t_max, int_t_min);
    it->next();
    return it;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

IntExpDomainIt*
IntExpDomainCAR::end() const
{
    IntExpDomainCARit* it = new IntExpDomainCARit(this, uint_t_max, int_t_max);
    return it;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int
IntExpDomainCAR::getPrev(int val) const
{
    if (val <= _min)
    {
        return int_t_min;
    }

    if (val > _max)
    {
        return _max;
    }

    uint_t idx =
        utl::binarySearch(_valuesArray.get(), 0, _num, val - 1, utl::subtract<int>(), utl::find_ip);
    if (_values[idx] >= val)
        --idx;
    while (getCount(idx) == 0)
        --idx;
    return _values[idx];
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int
IntExpDomainCAR::getNext(int val) const
{
    if (val < _min)
    {
        return _min;
    }

    if (val >= _max)
    {
        return int_t_max;
    }

    uint_t idx =
        utl::binarySearch(_valuesArray.get(), 0, _num, val + 1, utl::subtract<int>(), utl::find_ip);
    while (getCount(idx) == 0)
        ++idx;
    return _values[idx];
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
IntExpDomainCAR::_saveState()
{
    super::_saveState();
    _mgr->revSet(_stateDepth);
    _stateDepth = _mgr->depth();
    _mgr->revSet(_counts, _countsArray.size());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
IntExpDomainCAR::removeRange(int min, int max)
{
    ASSERTD(min <= max);

    min = utl::max(min, _min);
    max = utl::min(max, _max);

    // nothing to do?
    if ((min > max) || (min < _max) || (max < _min))
    {
        return;
    }

    saveState();

    // empty domain?
    if ((min <= _min) && (max >= _max))
    {
        _events |= ef_domain;
        _events |= ef_empty;
        _min = int_t_max;
        _max = int_t_min;
        return;
    }

    // find index of first value
    uint_t idx =
        utl::binarySearch(_valuesArray.get(), 0, _num, min, utl::subtract<int>(), utl::find_ip);

    // clear flags to remove values from the domain
    while ((_values[idx] <= max) && (idx < _num))
    {
        if (zeroCount(idx++))
        {
            _events |= ef_domain;
            --_size;
        }
    }

    // adjust _min, _max
    int oldMin = _min;
    int oldMax = _max;
    if (min <= _min)
    {
        _min = getNext(max);
    }
    if (max >= _max)
    {
        _max = getPrev(min);
    }

    // value event
    if (_min == _max)
    {
        _events |= ef_value;
    }

    // min/max/range events
    if (_min != oldMin)
    {
        _events |= ef_range;
        _events |= ef_min;
    }
    if (_max != oldMax)
    {
        _events |= ef_range;
        _events |= ef_max;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
IntExpDomainCAR::init()
{
    ABORT();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
IntExpDomainCAR::init(const int_uint_map_t& domain)
{
    ASSERTD(_mgr != nullptr);
    ASSERTD(domain.size() > 0);

    // _num = number of values in domain
    _num = _size = domain.size();

    // determine maximum count
    uint_t maxCount = 0;
    int_uint_map_t::const_iterator it;
    for (it = domain.begin(); it != domain.end(); ++it)
    {
        uint_t count = (*it).second;
        ASSERTD(count > 0);
        maxCount = utl::max(maxCount, count);
    }

    // set up for fast calculation based on number of bits per count
    // use 2, 4, 8, 16, or 32 bits per count (based on maxCount)
    _bits = 2;
    _bitsLog2 = 1;
    _countsPerWord = 16;
    _countsPerWordLog2 = 2;
    while (maxCount > ((1U << _bits) - 1))
    {
        _bits <<= 1;
        ++_bitsLog2;
        _countsPerWord >>= 1;
        ++_countsPerWordLog2;
    }
    _countsPerWordMask = (_countsPerWord - 1);
    _mask = (1U << _bits) - 1;

    // determine size of counts array
    uint_t countsArraySize = utl::roundUp(_num << _bitsLog2, 32U) / 32;

    // set array sizes
    _valuesArray.setIncrement(1);
    _countsArray.setIncrement(1);
    _valuesArray.setSize(_num);
    _countsArray.setSize(countsArraySize);

    _values = _valuesArray.get();
    _counts = _countsArray.get();
    _stateDepth = _mgr->depth();

    // copy domain into values array and set counts
    uint_t idx = 0;
    for (it = domain.begin(); it != domain.end(); ++it, ++idx)
    {
        int val = (*it).first;
        uint_t count = (*it).second;
        _values[idx] = val;
        setCount(idx, count);
    }

    // initialize _min, _max
    _min = _values[0];
    _max = _values[_num - 1];
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
IntExpDomainCAR::deInit()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////

uint_t
IntExpDomainCAR::getCount(uint_t idx) const
{
    uint_t wordIdx = idx >> _countsPerWordLog2;
    uint_t countIdx = idx & _countsPerWordMask;
    uint_t maskShift = (countIdx << _bitsLog2);
    uint32_t mask = _mask << maskShift;
    uint32_t& word = _counts[wordIdx];
    uint_t count = (word & mask) >> maskShift;
    return count;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

uint_t
IntExpDomainCAR::setCount(uint_t idx, uint_t count)
{
    ASSERTD(count <= _mask);
    uint_t wordIdx = idx >> _countsPerWordLog2;
    uint_t countIdx = idx & _countsPerWordMask;
    uint_t maskShift = (countIdx << _bitsLog2);
    uint32_t mask = _mask << maskShift;
    uint32_t& word = _counts[wordIdx];

    // clear old count
    uint_t oldCount = (word & mask) >> maskShift;
    word &= ~mask;

    // set new count
    mask = count;
    mask <<= maskShift;
    word |= mask;

    return oldCount;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_END;
