#ifndef GOP_STRINGSCORE_H
#define GOP_STRINGSCORE_H

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <gop/String.h>
#include <gop/Score.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

GOP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/** 
    StringScore has two major attributes: a string and its corresponding 
    score. It is designed to be used by optimizers (e.g. SA, RandomWalk, etc)
    to remember a string and its score (usually it's the best score so
    far).

    Joe Zhou
    Oct. 2005

    Note: this class was defined in HillClimber.h before.
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class StringScore : public utl::Object
{
    UTL_CLASS_DECL(StringScore, utl::Object);

public:
    StringScore(uint_t id, String<uint_t>* string, Score* score)
    {
        init();
        _id = id;
        setString(string);
        setScore(score);
        /*         _string = string; */
        /*         _score = score; */
    }

    virtual void copy(const utl::Object& rhs);

    uint_t
    getId()
    {
        return _id;
    }

    String<uint_t>*
    getString()
    {
        return _string;
    }

    void
    setString(String<uint_t>* string)
    {
        delete _string;
        _string = string;
    }

    Score*
    getScore()
    {
        return _score;
    }

    void
    setScore(Score* score)
    {
        delete _score;
        _score = score;
    }

    double
    getScoreValue()
    {
        return _score->getValue();
    }

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

    uint_t _id;
    String<uint_t>* _string;
    Score* _score;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

struct stringScoreOrdering : public std::binary_function<StringScore*, StringScore*, bool>
{
    bool
    operator()(StringScore* strScore1, StringScore* strScore2)
    {
        double score1 = strScore1->getScoreValue();
        double score2 = strScore2->getScoreValue();
        return score1 < score2;
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////

typedef std::vector<StringScore*> stringscore_vector_t;

////////////////////////////////////////////////////////////////////////////////////////////////////

GOP_NS_END;

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
