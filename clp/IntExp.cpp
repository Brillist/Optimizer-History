#include "libclp.h"
#include "Bound.h"
#include "IntExp.h"
#include "IntExpDomainRISC.h"
#include "Manager.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL_ABC(clp::IntExp);

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

IntExp::IntExp(Manager* mgr, IntExpDomain* domain)
{
    init();
    _mgr = nullptr;
    setManager(mgr);
    _domain = domain;
    if (_domain == nullptr)
    {
        _domain = new IntExpDomainRISC(_mgr);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
IntExp::copy(const Object& rhs)
{
    auto& rhsExp = utl::cast<IntExp>(rhs);
    _domain = lut::clone(rhsExp._domain);
    setManager(rhsExp.manager());
    _name = rhsExp._name;
    _failOnEmpty = false;
    _object = rhsExp._object;
    ASSERTD(_valueBounds.empty());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

String
IntExp::toString() const
{
    return _domain->toString();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
IntExp::backtrack()
{
    ABORT();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

IntExp*
IntExp::mclone()
{
    IntExp* exp;
    if (this->managed())
    {
        exp = this;
        exp->mcopy();
    }
    else
    {
        exp = clone();
        exp->mcopy();
        ASSERTD(_mgr != nullptr);
        _mgr->add(exp);
    }
    return exp;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
IntExp::mcopy()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool
IntExp::managed() const
{
    return _managed;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

const IntExpDomainRISC*
IntExp::domainRISC() const
{
    return utl::cast<IntExpDomainRISC>(_domain);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
IntExp::setManager(Manager* mgr)
{
    if (mgr == _mgr)
        return;
    ASSERTD(_mgr == nullptr);
    _mgr = mgr;
    _intersectExps.initialize(_mgr);
    _domainBounds.initialize(_mgr);
    _valueBounds.initialize(_mgr);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool
IntExp::add(int val)
{
    _domain->add(val, val);
    if (_domain->anyEvent())
    {
        raiseEvents();
        return true;
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

uint_t
IntExp::add(int min, int max)
{
    uint_t oldSize = _domain->size();
    _domain->add(min, max);
    if (_domain->anyEvent())
    {
        raiseEvents();
        return (_domain->size() - oldSize);
    }
    return 0U;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
IntExp::setDeferRemoves(bool deferRemoves)
{
    if (_stateDepth < _mgr->depth())
    {
        _mgr->revSet(&_stateDepth, 2);
        _stateDepth = _mgr->depth();
    }
    _deferRemoves = deferRemoves;
    if (_deferRemoves)
    {
        _deferredRemovesPtr = _deferredRemoves;
    }
    else
    {
        // remove the deferred stuff
        int* it = _deferredRemoves;
        while (it != _deferredRemovesPtr)
        {
            int min = *it++;
            int max = *it++;
            remove(min, max);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
IntExp::setRange(int min, int max)
{
    setMin(min);
    setMax(max);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
IntExp::setMin(int min)
{
    if (min <= _domain->min())
        return;
    _domain->remove(utl::int_t_min, min - 1);
    if (_domain->anyEvent())
        raiseEvents();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
IntExp::setMax(int max)
{
    if (max >= _domain->max())
        return;
    _domain->remove(max + 1, utl::int_t_max);
    if (_domain->anyEvent())
        raiseEvents();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
IntExp::intersect(const IntExp* expr)
{
    _domain->intersect(expr->_domain);
    raiseEvents();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
IntExp::intersect(const int* array, uint_t size)
{
    _domain->intersect(array, size);
    raiseEvents();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
IntExp::intersect(const uint_t* array, uint_t size)
{
    _domain->intersect(array, size);
    raiseEvents();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
IntExp::intersect(const std::set<int>& intSet)
{
    _domain->intersect(intSet);
    raiseEvents();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool
IntExp::remove(int val)
{
    return (remove(val, val) > 0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

uint_t
IntExp::remove(int min, int max)
{
    if (_deferRemoves)
    {
        if (_deferredRemovesPtr == _deferredRemovesLim)
        {
            utl::arrayGrow(_deferredRemoves, _deferredRemovesPtr, _deferredRemovesLim,
                           utl::max((size_t)256, _deferredRemovesSize * 2));
            _deferredRemovesSize = _deferredRemovesLim - _deferredRemoves;
        }

        // check for continuity with previous removal
        if (_deferredRemovesPtr > _deferredRemoves)
        {
            int& prevRemoveMin = *(_deferredRemovesPtr - 2);
            int& prevRemoveMax = *(_deferredRemovesPtr - 1);
            if (min == (prevRemoveMax + 1))
            {
                prevRemoveMax = max;
                return 0U;
            }
            else if (max == (prevRemoveMin - 1))
            {
                prevRemoveMin = min;
                return 0U;
            }
        }

        *_deferredRemovesPtr++ = min;
        *_deferredRemovesPtr++ = max;
        return 0U;
    }

    uint_t oldSize = _domain->size();
    _domain->remove(min, max);
    if (_domain->anyEvent())
    {
        // update intersect-exps
        for (auto intersectExp : _intersectExps)
        {
            intersectExp->remove(min, max);
        }

        raiseEvents();
        return (oldSize - _domain->size());
    }
    return 0U;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
IntExp::raiseEvents()
{
    // no event => do nothing
    if (!_domain->anyEvent())
    {
        return;
    }

    // domain is now empty and we're supposed to fail?
    if (_domain->emptyEvent() && _failOnEmpty)
    {
        // clear events and fail
        _domain->clearEvents();
        throw FailEx();
    }

    try
    {
        // domain event?
        if (_domain->domainEvent())
        {
            // invalidate domain-bounds
            for (auto cb : _domainBounds)
            {
                cb->invalidate();
            }

            // constrained to a single value?
            if (_domain->valueEvent())
            {
                // invalidate value-bounds
                for (auto cb : _valueBounds)
                {
                    cb->invalidate();
                }
            }
        }

        _domain->clearEvents();
    }
    catch (...)
    {
        _domain->clearEvents();
        throw;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
IntExp::init()
{
    _domain = nullptr;
    _mgr = nullptr;
    _managed = false;
    _failOnEmpty = true;
    _object = nullptr;
    _stateDepth = 0;
    _deferRemoves = false;
    _deferredRemoves = _deferredRemovesPtr = _deferredRemovesLim = nullptr;
    _deferredRemovesSize = 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
IntExp::deInit()
{
    delete _domain;
    delete[] _deferredRemoves;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_END;
