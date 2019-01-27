#include "libclp.h"
#include <libutl/AutoPtr.h>
#include "Manager.h"
#include "IntExpDomain.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef DEBUG
//#define DEBUG_UNIT
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL_ABC(clp::IntExpDomain);

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

void
IntExpDomain::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(IntExpDomain));
    const IntExpDomain& ied = (const IntExpDomain&)rhs;
    _mgr = ied._mgr;
    _events = 0;
    _min = ied._min;
    _max = ied._max;
    _size = ied._size;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

String
IntExpDomain::toString() const
{
    std::ostringstream ss;
    bool firstSpan = true;
    int spanMin, spanMax;
    spanMin = spanMax = int_t_min;
    for (;;)
    {
        int val = getNext(spanMax);
        bool extendSpan = (spanMin != int_t_min) && (val == (spanMax + 1));

        // complete a span?
        if ((spanMin > int_t_min) && !extendSpan)
        {
            if (!firstSpan)
            {
                ss << ",";
            }
            if (spanMin == spanMax)
            {
                ss << "[" << spanMin << "]";
            }
            else
            {
                ss << "[" << spanMin << ".." << spanMax << "]";
            }
            firstSpan = false;
        }

        // end of domain?
        if (val == int_t_max)
        {
            break;
        }

        // extend a span?
        if (extendSpan)
        {
            ++spanMax;
        }
        else
        {
            spanMin = spanMax = val;
        }
    }
    return ss.str().c_str();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
IntExpDomain::intersect(const int* array, uint_t size)
{
    int last = int_t_min;
    const int* arrayLim = array + size;
    for (const int* ptr = array; ptr != arrayLim; ++ptr)
    {
        int val = *ptr;
        if ((last + 1) < val)
        {
            removeRange(last + 1, val - 1);
        }
        last = val;
    }
    ASSERTD(last < int_t_max);
    removeRange(last + 1, int_t_max);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
IntExpDomain::intersect(const std::set<int>& intSet)
{
    int last = int_t_min;
    std::set<int>::const_iterator it;
    for (it = intSet.begin(); it != intSet.end(); ++it)
    {
        int val = *it;
        if ((last + 1) < val)
        {
            removeRange(last + 1, val - 1);
        }
        last = val;
    }
    ASSERTD(last < int_t_max);
    removeRange(last + 1, int_t_max);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
IntExpDomain::intersect(const IntExpDomain* rhs)
{
    int last = int_t_min;
    AutoPtr<IntExpDomainIt> itPtr;
    IntExpDomainIt* it;
    for (itPtr = it = rhs->begin(); !it->atEnd(); it->next())
    {
        int val = **it;
        removeRange(last + 1, val - 1);
        last = val;
    }

    // values > _last are not in rhs and therefore removed from lhs

    ASSERTD(last < int_t_max);
    removeRange(last + 1, int_t_max);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
IntExpDomain::_saveState()
{
    _mgr->revSet((uint_t*)&_min, 3);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
IntExpDomain::init(Manager* mgr)
{
    _mgr = mgr;
    _events = 0;
    _min = int_t_max;
    _max = int_t_min;
    _size = 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_END;
