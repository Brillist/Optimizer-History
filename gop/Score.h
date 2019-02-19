#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

GOP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Score type.

   \ingroup gop
*/
enum score_type_t
{
    score_failed = 0,      /**< construction failed */
    score_ct_violated = 1, /**< constraint violation */
    score_succeeded = 2,   /**< successful */
    score_undefined = 3
};

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
    An IndEvaluator score and its type.

   \ingroup gop
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class Score : public utl::Object
{
    UTL_CLASS_DECL(Score, utl::Object);

public:
    /**
       Constructor.
       \param value evaluation score
       \param type type of score
    */
    Score(double value, score_type_t type = score_undefined);

    virtual void copy(const utl::Object& rhs);

    virtual int compare(const utl::Object& rhs) const;

    virtual void serialize(utl::Stream& stream, uint_t io, uint_t mode = utl::ser_default);

    virtual utl::String toString() const;

    /// \name Accessors
    //@{
    /** Get the score value. */
    double
    getValue() const
    {
        return _value;
    }

    /** Get the score type. */
    score_type_t const
    getType()
    {
        return _type;
    }
    //@}

    /// \name Modification
    //@{
    /** Set the score value. */
    void
    setValue(double value)
    {
        _value = value;
    }

    /** Set the score type. */
    void
    setType(score_type_t type)
    {
        _type = type;
    }
    //@}

private:
    void init();
    void
    deInit()
    {
    }

private:
    double _value;
    score_type_t _type;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   A \c std::map from keys of type \ref uint_t to values of type Score pointer.
   \ingroup gop
*/
using uint_score_map_t = std::map<uint_t, Score*>;

////////////////////////////////////////////////////////////////////////////////////////////////////

GOP_NS_END;
