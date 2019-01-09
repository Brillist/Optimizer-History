#include "libcls.h"
#include "CompositeSpan.h"

//////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;
CLP_NS_USE;

//////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(cls::CompositeSpan, clp::IntSpan);

//////////////////////////////////////////////////////////////////////////////

CLS_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////

CompositeSpan::CompositeSpan(
    int min,
    int max,
    IntExpDomainAR* resIds,
    uint_t level)
    : IntSpan(min, max, 0, 0, level)
{
    setResIds(resIds);
}

//////////////////////////////////////////////////////////////////////////////

void
CompositeSpan::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(CompositeSpan));
    const CompositeSpan& cs = (const CompositeSpan&)rhs;
    delete resIds();
    IntSpan::copy(cs);
    setResIds(utl::clone(cs.resIds()));
}

//////////////////////////////////////////////////////////////////////////////

utl::String
CompositeSpan::toString() const
{
    MemStream str;
    str << "[" << _min << "," << _max << "]: ";
    if (resIds() == nullptr)
    {
        str << "(null)";
    }
    else
    {
        str << resIds()->toString();
    }
    str << '\0';
    return utl::String((char*)str.get());
}

//////////////////////////////////////////////////////////////////////////////

bool
CompositeSpan::canMergeWith(const clp::IntSpan* rhs) const
{
    ASSERTD(rhs->isA(CompositeSpan));
    const CompositeSpan* cs = (const CompositeSpan*)rhs;
    ASSERTD((resIds() != nullptr) && (cs->resIds() != nullptr));
    return resIds()->flagsEqual(cs->resIds());
}

//////////////////////////////////////////////////////////////////////////////

uint_t
CompositeSpan::capacity() const
{
    const IntExpDomainAR* resIds = this->resIds();
    ASSERTD(resIds != nullptr);
    return resIds->size();
}

//////////////////////////////////////////////////////////////////////////////

void
CompositeSpan::init()
{
    setResIds(nullptr);
}

//////////////////////////////////////////////////////////////////////////////

void
CompositeSpan::deInit()
{
}

//////////////////////////////////////////////////////////////////////////////

CLS_NS_END;
