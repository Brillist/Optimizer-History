#include "libclp.h"
#include "Manager.h"
#include "IntExpArray.h"

//////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;

//////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(clp::IntExpArray, utl::Object);

//////////////////////////////////////////////////////////////////////////////

CLP_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////

IntExpArray::IntExpArray(
    IntExp* v0,
    IntExp* v1)
{
    init();
    add(v0);
    add(v1);
}

//////////////////////////////////////////////////////////////////////////////

IntExpArray::IntExpArray(
    IntExp* v0,
    IntExp* v1,
    IntExp* v2)
{
    init();
    add(v0);
    add(v1);
    add(v2);
}

//////////////////////////////////////////////////////////////////////////////

IntExpArray::IntExpArray(
    IntExp* v0,
    IntExp* v1,
    IntExp* v2,
    IntExp* v3)
{
    init();
    add(v0);
    add(v1);
    add(v2);
    add(v3);
}

//////////////////////////////////////////////////////////////////////////////

IntExpArray::IntExpArray(
    IntExp* v0,
    IntExp* v1,
    IntExp* v2,
    IntExp* v3,
    IntExp* v4)
{
    init();
    add(v0);
    add(v1);
    add(v2);
    add(v3);
    add(v4);
}

//////////////////////////////////////////////////////////////////////////////

void
IntExpArray::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(IntExpArray));
    const IntExpArray& iea = (const IntExpArray&)rhs;
    _exps = iea._exps;
}

//////////////////////////////////////////////////////////////////////////////

void
IntExpArray::setManaged(bool managed)
{
    intexp_vector_t::const_iterator it;
    for (it = _exps.begin(); it != _exps.end(); ++it)
    {
        IntExp* var = *it;
        var->setManaged(true);
    }
}

//////////////////////////////////////////////////////////////////////////////

int
IntExpArray::minMin() const
{
    int res = int_t_max;
    intexp_vector_t::const_iterator it;
    for (it = _exps.begin(); it != _exps.end(); ++it)
    {
        IntExp* var = *it;
        res = utl::min(res, var->min());
    }
    return res;
}

//////////////////////////////////////////////////////////////////////////////

int
IntExpArray::maxMin() const
{
    int res = int_t_min;
    intexp_vector_t::const_iterator it;
    for (it = _exps.begin(); it != _exps.end(); ++it)
    {
        IntExp* var = *it;
        res = utl::max(res, var->min());
    }
    return res;
}

//////////////////////////////////////////////////////////////////////////////

int
IntExpArray::minMax() const
{
    int res = int_t_max;
    intexp_vector_t::const_iterator it;
    for (it = _exps.begin(); it != _exps.end(); ++it)
    {
        IntExp* var = *it;
        res = utl::min(res, var->max());
    }
    return res;
}

//////////////////////////////////////////////////////////////////////////////

int
IntExpArray::maxMax() const
{
    int res = int_t_min;
    intexp_vector_t::const_iterator it;
    for (it = _exps.begin(); it != _exps.end(); ++it)
    {
        IntExp* var = *it;
        res = utl::max(res, var->max());
    }
    return res;
}

//////////////////////////////////////////////////////////////////////////////

CLP_NS_END;
