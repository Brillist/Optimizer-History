#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

GOP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/** 
    An evaluation score and its type.

    Joe Zhou
    Nov. 2005
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

enum score_type_t
{
    score_failed = 0,
    score_ct_violated = 1,
    score_succeeded = 2,
    score_undefined = 3
};

////////////////////////////////////////////////////////////////////////////////////////////////////

class Score : public utl::Object
{
    UTL_CLASS_DECL(Score, utl::Object);

public:
    Score(double value, score_type_t type = score_undefined);

    virtual void copy(const utl::Object& rhs);

    virtual int compare(const utl::Object& rhs) const;

    virtual void serialize(utl::Stream& stream, uint_t io, uint_t mode = utl::ser_default);

    virtual utl::String toString() const;

    double
    getValue()
    {
        return _value;
    }

    score_type_t
    getType()
    {
        return _type;
    }

    void
    setValue(double value)
    {
        _value = value;
    }

    void
    setType(score_type_t type)
    {
        _type = type;
    }

private:
    void init();
    void
    deInit()
    {
    }

    double _value;
    score_type_t _type;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

typedef std::map<uint_t, Score*> uint_score_map_t;

////////////////////////////////////////////////////////////////////////////////////////////////////

GOP_NS_END;
