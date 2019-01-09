#ifndef GOP_SCORE_H
#define GOP_SCORE_H

//////////////////////////////////////////////////////////////////////////////

GOP_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////

/** 
    Score has two major attributes: a value and a type. It is designed 
    to handle all types of scores (failed_score, constraint_violated_score,
    succeeded_score, etc.).
    far).

    Joe Zhou
    Nov. 2005
*/

//////////////////////////////////////////////////////////////////////////////

enum score_type_t
{
    score_failed = 0,
    score_ct_violated = 1,
    score_succeeded = 2,
    score_undefined = 3
};

//////////////////////////////////////////////////////////////////////////////

class Score : public utl::Object
{
    UTL_CLASS_DECL(Score);
public:
    Score(double value, score_type_t type = score_undefined);

    virtual void copy(const utl::Object& rhs);

    virtual int compare(const utl::Object& rhs) const;

    virtual void serialize(
        utl::Stream& stream,
        utl::uint_t io,
        utl::uint_t mode = utl::ser_default);

    double getValue()
    { return _value; }

    score_type_t getType()
    { return _type; }

    void setValue(double value)
    { _value = value; }

    void setType(score_type_t type)
    { _type = type; }

    utl::String toString() const;

private:
    void init();
    void deInit(){}

    double _value;
    score_type_t _type;
};

//////////////////////////////////////////////////////////////////////////////

typedef std::map<utl::uint_t, Score*> uint_score_map_t;

//////////////////////////////////////////////////////////////////////////////

GOP_NS_END;

//////////////////////////////////////////////////////////////////////////////

#endif
