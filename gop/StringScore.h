#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <gop/String.h>
#include <gop/Score.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

GOP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   A String and its corresponding Score.

   \ingroup gop
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class StringScore : public utl::Object
{
    UTL_CLASS_DECL(StringScore, utl::Object);

public:
    /**
       Constructor.
       \param id id
       \param string construction String
       \param score string's evaluation Score
    */
    StringScore(uint_t id, String<uint_t>* string, Score* score)
    {
        init();
        _id = id;
        setString(string);
        setScore(score);
    }

    virtual void copy(const utl::Object& rhs);

    /// \name Accessors (const)
    //@{
    /** Get id. */
    uint_t
    getId()
    {
        return _id;
    }

    /** Get the String. */
    String<uint_t>*
    getString()
    {
        return _string;
    }

    /** Get the Score. */
    Score*
    getScore()
    {
        return _score;
    }

    /** Get the Score's value. */
    double
    getScoreValue()
    {
        return _score->getValue();
    }
    //@}

    /// \name Modification
    //@{
    /** Set the String. */
    void
    setString(String<uint_t>* string)
    {
        delete _string;
        _string = string;
    }

    /** Set the Score. */
    void
    setScore(Score* score)
    {
        delete _score;
        _score = score;
    }
    //@}

private:
    void
    init()
    {
        _string = nullptr;
        _score = nullptr;
    }
    void
    deInit()
    {
        delete _string;
        delete _score;
    }

private:
    uint_t _id;
    String<uint_t>* _string;
    Score* _score;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Non-decreasing ordering for StringScore objects.

   \ingroup gop
*/
struct StringScoreOrdering
{
    /** Evaluate to true iff lhs has a lower Score than rhs. */
    bool
    operator()(StringScore* lhs, StringScore* rhs)
    {
        double lhsScore = lhs->getScoreValue();
        double rhsScore = rhs->getScoreValue();
        return lhsScore < rhsScore;
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   A \c std::vector of StringScore pointers.
   \ingroup gop
*/
using stringscore_vector_t = std::vector<StringScore*>;

////////////////////////////////////////////////////////////////////////////////////////////////////

GOP_NS_END;
