#include "libclp.h"
#include "IntExpDomainAR.h"
#include "IntExpDomainARit.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(clp::IntExpDomainARit);

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

void
IntExpDomainARit::copy(const utl::Object& rhs)
{
    auto& dit = utl::cast<IntExpDomainARit>(rhs);
    _domain = dit._domain;
    _idx = dit._idx;
    _val = dit._val;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
IntExpDomainARit::next()
{
    if (_val >= _domain->max())
    {
        _idx = uint_t_max;
        _val = int_t_max;
        return;
    }

    if (_val < _domain->min())
    {
        _idx = _domain->findValueIdx(_domain->min());
        _val = _domain->min();
        return;
    }

    // note: _domain.min <= _val < _domain.max

    _idx = _domain->findForward(++_idx);
    _val = _domain->_values[_idx];
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
IntExpDomainARit::prev()
{
    if (_val <= _domain->min())
    {
        _idx = uint_t_max;
        _val = int_t_min;
        return;
    }

    if (_val > _domain->max())
    {
        _idx = _domain->findValueIdx(_domain->max());
        _val = _domain->max();
        return;
    }

    // note: _domain.min <= _val < _domain.max

    _idx = _domain->findBackward(--_idx);
    _val = _domain->_values[_idx];
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_END;
