#include "libclp.h"
#include "IntSpan.h"
#include "Manager.h"

//////////////////////////////////////////////////////////////////////////////

#undef new
#include <iomanip>
#include <libutl/gblnew_macros.h>

//////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;

//////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(clp::IntSpan, utl::Object);

//////////////////////////////////////////////////////////////////////////////

CLP_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////

IntSpan::IntSpan(
    int min,
    int max,
    uint_t v0,
    uint_t v1,
    uint_t level)
{
    init();
    _min = min;
    _max = max;
    _v0 = v0;
    _v1 = v1;
    if (level != uint_t_max)
    {
        setLevel(level);
    }
}

//////////////////////////////////////////////////////////////////////////////

void
IntSpan::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(IntSpan));
    const IntSpan& is = (const IntSpan&)rhs;
    _min = is._min;
    _max = is._max;
    _v0 = is._v0;
    _v1 = is._v1;
}

//////////////////////////////////////////////////////////////////////////////

utl::String
IntSpan::toString() const
{
    MemStream str;
    str << "[" << _min << "," << _max << "]: "
        << "(" << _v0 << "," << _v1 << ")" << '\0';
    return utl::String((char*)str.get());
}

//////////////////////////////////////////////////////////////////////////////

bool
IntSpan::canMergeWith(const IntSpan* rhs) const
{
    return (_v0 == rhs->_v0) && (_v1 == rhs->_v1);
}

//////////////////////////////////////////////////////////////////////////////

void
IntSpan::clearPointers()
{
    _prev = nullptr;
    memset(_next, 0, CLP_INTSPAN_MAXDEPTH * sizeof(IntSpan*));
}

//////////////////////////////////////////////////////////////////////////////

uint_t
IntSpan::capacity() const
{
    return (_v1 - _v0);
}

//////////////////////////////////////////////////////////////////////////////

void
IntSpan::_saveState(Manager* mgr)
{
    mgr->revSet(&_stateDepth, 5);
    mgr->revSet(_prev);
    if (_level == 0)
    {
        mgr->revSet((size_t&)_next[0]);
    }
    else
    {
        mgr->revSet((size_t*)_next, _level + 1);
    }
    _stateDepth = mgr->depth();
}

//////////////////////////////////////////////////////////////////////////////

void
IntSpan::setLevel(utl::uint_t level)
{
    if (_next != nullptr)
    {
        delete [] _next;
    }
    _level = level;
    uint_t num = level + 1;
    _next = new IntSpan*[num];
    memset(_next, 0, num * sizeof(IntSpan*));
}

//////////////////////////////////////////////////////////////////////////////

void
IntSpan::init()
{
#ifdef DEBUG
    static size_t spanId = 0;
    _id = spanId++;
#endif

    _stateDepth = 0;
    _prev = nullptr;
    _next = nullptr;
    _level = 0;
}

//////////////////////////////////////////////////////////////////////////////

void
IntSpan::deInit()
{
    ASSERTD(_next != nullptr);
    delete [] _next;
}

//////////////////////////////////////////////////////////////////////////////

CLP_NS_END;
