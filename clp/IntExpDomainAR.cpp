#include "libclp.h"
#include <libutl/algorithms_inl.h>
#include "IntExpDomainARit.h"
#include "IntExpDomainAR.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef DEBUG
//#define DEBUG_UNIT
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(clp::IntExpDomainAR);

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

IntExpDomainAR::IntExpDomainAR(Manager* mgr)
    : IntExpDomain(mgr)
{
    init();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

IntExpDomainAR::IntExpDomainAR(Manager* mgr, const int_set_t& domain, bool empty)
    : IntExpDomain(mgr)
{
    init(domain, empty);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

IntExpDomainAR::IntExpDomainAR(Manager* mgr, const uint_set_t& p_domain, bool empty)
    : IntExpDomain(mgr)
{
    // convert set<uint_t> -> set<int>
    int_set_t domain;
    uint_set_t::iterator it;
    for (it = p_domain.begin(); it != p_domain.end(); ++it)
    {
        uint_t val = *it;
        domain.insert(val);
    }

    init(domain, empty);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

IntExpDomainAR::IntExpDomainAR(Manager* mgr, uint_t num, int* values, bool valuesOwner, bool empty)
    : IntExpDomain(mgr)
{
    init(num, values, valuesOwner, empty);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
IntExpDomainAR::copy(const Object& rhs)
{
    auto& dar = utl::cast<IntExpDomainAR>(rhs);
    super::copy(dar);
    _num = dar._num;
    if (_valuesOwner)
        delete[] _values;
    _valuesOwner = dar._valuesOwner;
    delete[] _flags;
    _flagsSize = dar._flagsSize;

    if (_num == 0)
    {
        _values = nullptr;
        _flags = nullptr;
    }
    else
    {
        if (_valuesOwner)
        {
            _values = new int[_num];
            memcpy(_values, dar._values, _num * sizeof(int));
        }
        else
        {
            _values = dar._values;
        }

        _flags = new uint32_t[_flagsSize];
        memcpy(_flags, dar._flags, _flagsSize * 4);
    }
    if (_mgr == nullptr)
        _stateDepth = 0;
    else
        _stateDepth = _mgr->depth();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
IntExpDomainAR::copyFlags(const IntExpDomainAR* rhs)
{
    ASSERTD(_num == rhs->_num);
    _min = rhs->_min;
    _max = rhs->_max;
    _size = rhs->_size;
    if (_flagsSize == 1)
    {
        *_flags = *(rhs->_flags);
    }
    else
    {
        memcpy(_flags, rhs->_flags, _flagsSize * 4);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool
IntExpDomainAR::flagsEqual(const IntExpDomainAR* rhs) const
{
    ASSERTD(_flagsSize == rhs->_flagsSize);
    if (_size != rhs->_size)
        return false;
    if (_flagsSize == 1)
        return (*_flags == *rhs->_flags);
    return (memcmp(_flags, rhs->_flags, _flagsSize * 4) == 0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool
IntExpDomainAR::has(int val) const
{
    if ((val < _min) || (val > _max))
    {
        return false;
    }

    uint_t idx = findValueIdx(val);
    return (idx != uint_t_max) && (_values[idx] == val) && getFlag(idx);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

IntExpDomainIt*
IntExpDomainAR::begin() const
{
    if (_size == 0)
        return end();
    auto it = new IntExpDomainARit(this, uint_t_max, int_t_min);
    it->next();
    return it;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

IntExpDomainIt*
IntExpDomainAR::end() const
{
    auto it = new IntExpDomainARit(this, uint_t_max, int_t_max);
    return it;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int
IntExpDomainAR::getPrev(int val) const
{
    if (val <= _min)
    {
        return int_t_min;
    }

    uint_t idx = findValueIdx(val);
    if (val == _values[idx])
        --idx;
    idx = findBackward(idx);
    return _values[idx];
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int
IntExpDomainAR::getNext(int val) const
{
    if (val >= _max)
    {
        return int_t_max;
    }

    uint_t idx = findValueIdx(val);
    if (val == _values[idx])
        ++idx;
    idx = findForward(idx);
    return _values[idx];
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
IntExpDomainAR::_saveState()
{
    super::_saveState();
    _mgr->revSet(_stateDepth);
    _stateDepth = _mgr->depth();
    _mgr->revSet(_flags, _flagsSize);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
IntExpDomainAR::addRange(int min, int max)
{
    uint_t oldSize = _size;
    min = utl::max(min, _values[0]);
    max = utl::min(max, _values[_num - 1]);

    // nothing to do?
    if (min > max)
        return;

    saveState();

    // added values are in [minIdx,maxIdx]
    uint_t minIdx, maxIdx;

    // find index of first value
    minIdx = findValueIdx(min);

    // set flags to add values to the domain
    uint_t idx;
    for (idx = minIdx; (idx < _num) && (_values[idx] <= max); ++idx)
    {
        if (setFlag(idx))
        {
            if (_size == oldSize)
                minIdx = idx;
            maxIdx = idx;
            _events |= ef_domain;
            ++_size;
        }
    }

    // nothing happened?
    if (_size == oldSize)
    {
        return;
    }

    // note: values in [minIdx,maxIdx] have been added

    // adjust _min, record event
    if (_values[minIdx] < _min)
    {
        _min = _values[minIdx];
        _events |= ef_range;
        _events |= ef_min;
    }

    // adjust _max, record event
    if (_values[maxIdx] > _max)
    {
        _max = _values[maxIdx];
        _events |= ef_range;
        _events |= ef_max;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
IntExpDomainAR::removeRange(int min, int max)
{
    uint_t oldSize = _size;
    min = utl::max(min, _min);
    max = utl::min(max, _max);

    // nothing to do?
    if ((min > max) || (min > _max) || (max < _min))
    {
        return;
    }

    saveState();

    // empty domain?
    if ((min <= _min) && (max >= _max))
    {
        _events |= ef_empty;
        _size = 0;
        _min = int_t_max;
        _max = int_t_min;
        return;
    }

    // find index of first value >= min
    uint_t idx, minIdx;
    for (idx = findValueIdx(min); (idx < _num) && (_values[idx] <= max); ++idx)
    {
        if (clearFlag(idx))
        {
            if (_size == oldSize)
                minIdx = idx;
            _events |= ef_domain;
            --_size;
        }
    }

    // nothing happened?
    if (_size == oldSize)
    {
        return;
    }

    // adjust _min, record event
    if (min <= _min)
    {
        // note: (max < _max) because domain is not empty
        ASSERTD(idx < _num);
        idx = findForward(idx);
        _min = _values[idx];
        _events |= ef_range;
        _events |= ef_min;
    }

    // adjust _max, record event
    if (max >= _max)
    {
        // note: (min > _min) because domain is not empty
        ASSERTD(minIdx > 0);
        idx = findBackward(minIdx - 1);
        _max = _values[idx];
        _events |= ef_range;
        _events |= ef_max;
    }

    // value event
    if (_min == _max)
    {
        _events |= ef_value;
    }

    ASSERTD(_min <= _max);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
IntExpDomainAR::init()
{
    _num = 0;
    _valuesOwner = false;
    _values = nullptr;
    _flagsSize = 0;
    _stateDepth = 0;
    _flags = nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
IntExpDomainAR::init(const std::set<int>& domain, bool empty)
{
    auto values = new int[domain.size()];
    std::set<int>::const_iterator it;
    auto ptr = values;
    for (auto val : domain)
    {
        *ptr++ = val;
    }
    init(domain.size(), values, true, empty);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
IntExpDomainAR::init(uint_t num, int* values, bool valuesOwner, bool empty)
{
    ASSERTD(_mgr != nullptr);
    ASSERTD(num > 0);

    _num = num;
    _stateDepth = _mgr->depth();

    // _flags[]
    _flagsSize = utl::roundUp(_num, 32U) / 32;
    _flags = new uint32_t[_flagsSize];

    // _values[]
    _valuesOwner = valuesOwner;
    _values = values;

    // _size, _min, _max, _flags[]
    if (empty)
    {
        _size = 0;
        _min = int_t_max;
        _max = int_t_min;
        if (_flagsSize == 1)
            *_flags = 0;
        else
            memset(_flags, 0x00, _flagsSize * sizeof(uint32_t));
    }
    else
    {
        _size = _num;
        _min = _values[0];
        _max = _values[_num - 1];
        if (_flagsSize == 1)
            *_flags = uint32_t_max;
        else
            memset(_flags, 0xff, _flagsSize * sizeof(uint32_t));
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
IntExpDomainAR::deInit()
{
    if (_valuesOwner)
        delete[] _values;
    delete[] _flags;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

uint_t
IntExpDomainAR::findValueIdx(int val) const
{
    if (val > _values[_num - 1])
        return uint_t_max;

    // |values| <= 32  ==>  linear search is faster
    if (_num <= 32)
    {
        uint_t numDiv2 = _num >> 1;
        int* midPtr = _values + numDiv2;
        int* valuePtr = (val >= *midPtr) ? midPtr : _values;

        while (*valuePtr < val)
            ++valuePtr;
        return (valuePtr - _values);
    }

    uint_t idx = utl::binarySearch(_values, 0, _num, val, utl::subtract<int>(), utl::find_ip);

    return idx;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

uint_t
IntExpDomainAR::findForward(uint_t idx) const
{
    if (idx >= _num)
        return uint_t_max;
    if (_values[idx] > _max)
        return uint_t_max;

    uint_t* flagsPtr = _flags + (idx >> 5);
    uint_t flags = *flagsPtr;
    uint_t flagsMask = 0x80000000U >> (idx & 0x1f);
    while ((flags & flagsMask) == 0)
    {
        ++idx;
        flagsMask >>= 1;
        if (flagsMask == 0)
        {
            flags = *++flagsPtr;
            flagsMask = 0x80000000U;
        }
    }
    return idx;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

uint_t
IntExpDomainAR::findBackward(uint_t idx) const
{
    if (idx >= _num)
        idx = _num - 1;
    if (_values[idx] < _min)
        return uint_t_max;

    uint_t* flagsPtr = _flags + (idx >> 5);
    uint_t flags = *flagsPtr;
    uint_t flagsMask = 0x80000000U >> (idx & 0x1f);
    while ((flags & flagsMask) == 0)
    {
        --idx;
        flagsMask <<= 1;
        if (flagsMask == 0)
        {
            flags = *--flagsPtr;
            flagsMask = 1U;
        }
    }
    return idx;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool
IntExpDomainAR::getFlag(uint_t idx) const
{
    ASSERTD(idx < _num);
    uint_t word = idx >> 5;
    uint_t bit = idx & 0x1f;
    uint32_t flags = _flags[word];
    uint32_t mask = (0x80000000U >> bit);
    return ((flags & mask) != 0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool
IntExpDomainAR::setFlag(uint_t idx)
{
    ASSERTD(idx < _num);
    uint_t word = idx >> 5;
    uint_t bit = idx & 0x1f;
    uint32_t mask = (0x80000000U >> bit);
    if ((_flags[word] & mask) != 0)
        return false;
    _flags[word] |= mask;
    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool
IntExpDomainAR::clearFlag(uint_t idx)
{
    ASSERTD(idx < _num);
    uint_t word = idx >> 5;
    uint_t bit = idx & 0x1f;
    uint32_t mask = (0x80000000U >> bit);
    if ((_flags[word] & mask) == 0)
        return false;
    _flags[word] &= ~mask;
    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_END;
