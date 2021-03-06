#include "libclp.h"
#include "IntExpDomainCAR.h"
#include "IntExpDomainCARit.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(clp::IntExpDomainCARit);

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

void
IntExpDomainCARit::copy(const utl::Object& rhs)
{
    auto& it = utl::cast<IntExpDomainCARit>(rhs);
    _domain = it._domain;
    _idx = it._idx;
    _val = it._val;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
IntExpDomainCARit::next()
{
    if (_val >= _domain->max())
    {
        _idx = uint_t_max;
        _val = int_t_max;
        return;
    }

    if (_val < _domain->min())
    {
        for (_idx = 0; _domain->getCount(_idx) == 0; ++_idx)
            ;
        _val = _domain->_values[_idx];
        ASSERTD(_val == _domain->min());
        return;
    }

    // note: _domain.min <= _val < _domain.max

    while (_domain->getCount(++_idx) == 0)
        ;
    _val = _domain->_values[_idx];
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
IntExpDomainCARit::prev()
{
    if (_val <= _domain->min())
    {
        _idx = uint_t_max;
        _val = int_t_min;
        return;
    }

    if (_val > _domain->max())
    {
        for (_idx = _domain->_num - 1; _domain->getCount(_idx) == 0; --_idx)
            ;
        _val = _domain->_values[_idx];
        ASSERTD(_val == _domain->max());
        return;
    }

    // note: _domain.min <= _val < _domain.max

    while (_domain->getCount(--_idx) == 0)
        ;
    _val = _domain->_values[_idx];
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_END;
