#include "libgop.h"
#include <libutl/MemStream.h>
#include <libutl/Float.h>
#include "Score.h"

//////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;

//////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(gop::Score, utl::Object);

//////////////////////////////////////////////////////////////////////////////

GOP_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////

Score::Score(double value, score_type_t type)
{
    _value = value;
    _type = type;
}

//////////////////////////////////////////////////////////////////////////////

void
Score::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(Score));
    const Score& score = (const Score&)rhs;
    _value = score._value;
    _type = score._type;
}

//////////////////////////////////////////////////////////////////////////////

int
Score::compare(const Object& rhs) const
{
    if (!rhs.isA(Score)) return Object::compare(rhs);
    const Score& score = (const Score&)rhs;
    int result;
    result = lut::compare(_value, score._value);
    if (result != 0) return result;
    result = lut::compare(_type, score._type);
    return result;
}

//////////////////////////////////////////////////////////////////////////////

void
Score::serialize(Stream& stream, uint_t io, uint_t)
{
    utl::serialize(_value, stream, io);
    utl::serialize((uint_t&)_type, stream, io);
}

//////////////////////////////////////////////////////////////////////////////

String
Score::toString() const
{
    utl::MemStream str;
    str << Float(_value).toString("precision:2");
    switch (_type)
    {
    case score_failed:
        str << "(failed)";
        break;
    case score_ct_violated:
        str << "(ct_violated)";
        break;
    case score_succeeded:
        break;
    case score_undefined:
        str << "(undefined)";
        break;
    default:
        ABORT();
    }
    str << '\0';
    return utl::String((char*)str.get());
}

//////////////////////////////////////////////////////////////////////////////

void
Score::init()
{
    _value = 0;
    _type = score_undefined;
}

//////////////////////////////////////////////////////////////////////////////

GOP_NS_END;
