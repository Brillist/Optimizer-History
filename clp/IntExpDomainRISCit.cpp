#include "libclp.h"
#include "IntExpDomainRISC.h"
#include "IntExpDomainRISCit.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(clp::IntExpDomainRISCit, utl::Object);

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

void
IntExpDomainRISCit::copy(const utl::Object& rhs)
{
    ASSERTD(rhs.isA(IntExpDomainRISCit));
    const IntExpDomainRISCit& dit = (const IntExpDomainRISCit&)rhs;
    _domain = dit._domain;
    _span = dit._span;
    _val = dit._val;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
IntExpDomainRISCit::next()
{
    if (_val >= _domain->max())
    {
        _span = _domain->tail();
        _val = int_t_max;
        return;
    }

    if (_val < _domain->min())
    {
        _span = _domain->head()->next()->next();
        _val = _span->min();
        ASSERTD(_val == _domain->min());
        return;
    }

    // note: _domain.min <= _val < _domain.max

    ++_val;
    if (_val > _span->max())
    {
        _span = _span->next()->next();
        _val = _span->min();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
IntExpDomainRISCit::prev()
{
    if (_val <= _domain->min())
    {
        _span = _domain->head();
        _val = int_t_min;
        return;
    }

    if (_val > _domain->max())
    {
        _span = _domain->tail()->prev()->prev();
        _val = _span->max();
        ASSERTD(_val == _domain->max());
        return;
    }

    // note: _domain.min < _val <= _domain.max

    --_val;
    if (_val < _span->min())
    {
        _span = _span->prev()->prev();
        _val = _span->max();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_END;
