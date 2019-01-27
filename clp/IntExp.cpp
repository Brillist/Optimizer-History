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
    ASSERTD(rhs.isA(IntExp));
    const IntExp& rhsExp = (const IntExp&)rhs;
    _domain = lut::clone(rhsExp._domain);
    setManager(rhsExp.manager());
    _name = rhsExp._name;
    _failOnEmpty = false;
    _object = rhsExp._object;
    ASSERTD(_valueBounds.empty());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

IntExp*
IntExp::mclone()
{
    IntExp* exp;
    if (isManaged())
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

String
IntExp::toString() const
{
    return _domain->toString();
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

const IntExpDomainRISC*
IntExp::domainRISC() const
{
    const IntExpDomainRISC* risc = dynamic_cast<const IntExpDomainRISC*>(_domain);
    ASSERTD(risc != nullptr);
    return risc;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

const IntSpan*
IntExp::head() const
{
    const IntExpDomainRISC* domain = dynamic_cast<const IntExpDomainRISC*>(_domain);
    ASSERTD(domain != nullptr);
    return domain->head();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

const IntSpan*
IntExp::tail() const
{
    const IntExpDomainRISC* domain = dynamic_cast<const IntExpDomainRISC*>(_domain);
    ASSERTD(domain != nullptr);
    return domain->tail();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
IntExp::set(int min, int max)
{
    min = utl::max(min, int_t_min + 1);
    max = utl::min(max, int_t_max - 1);
    _domain->remove(int_t_min, min - 1);
    _domain->remove(max + 1, int_t_max);
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
IntExp::deferRemoves(bool b)
{
    if (_stateDepth < _mgr->depth())
    {
        _mgr->revSet(&_stateDepth, 2);
        _stateDepth = _mgr->depth();
    }
    _deferRemoves = b;
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
        if (_intersectExps.size() > 0)
        {
            intexp_array_t::iterator it, lim = _intersectExps.end();
            for (it = _intersectExps.begin(); it != lim; ++it)
            {
                IntExp* intersectExp = *it;
                intersectExp->remove(min, max);
            }
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

    if (_domain->emptyEvent() && _failOnEmpty)
    {
        _domain->clearEvents();
        throw FailEx();
    }

    try
    {
        //////////////////////////////////////////////
        // domain/value event //
        //////////////////////////////////////////////

        if (_domain->domainEvent())
        {
            cb_set_t::iterator it, endIt = _domainBounds.end();
            for (it = _domainBounds.begin(); it != endIt; ++it)
            {
                ConstrainedBound* cb = *it;
                cb->invalidate();
            }

            if (_domain->valueEvent())
            {
                endIt = _valueBounds.end();
                for (it = _valueBounds.begin(); it != endIt; ++it)
                {
                    ConstrainedBound* cb = *it;
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
