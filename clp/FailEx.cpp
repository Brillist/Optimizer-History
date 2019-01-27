#include "libclp.h"
#include "FailEx.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(clp::FailEx);

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

FailEx::FailEx(String* str, uint_t label)
    : utl::Exception("inconsistency", nullptr)
{
    init(str, label);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
FailEx::copy(const utl::Object& rhs)
{
    ASSERTD(rhs.isA(FailEx));
    const FailEx& ex = (const FailEx&)rhs;
    Exception::copy(ex);
    delete _str;
    _str = utl::clone(ex._str);
    _label = ex._label;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
FailEx::init(const String* str, const uint_t label)
{
    _str = str;
    _label = label;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
FailEx::deInit()
{
    delete _str;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_END;
