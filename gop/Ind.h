#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <functional>

////////////////////////////////////////////////////////////////////////////////////////////////////

GOP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

class OSID;
class Objective;
class Population;

////////////////////////////////////////////////////////////////////////////////////////////////////
// Ind /////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Individual problem solution (abstract).

   Ind is an abstract base for classes that represent individual problem solutions.

   \see IndBuilder
   \see Population
   \ingroup gop
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class Ind : public utl::Object
{
    UTL_CLASS_DECL_ABC(Ind, utl::Object);

public:
    // since we can't #include "Objective.h"
    using objective_vector_t = std::vector<Objective*>;

public:
    virtual void copy(const utl::Object& rhs);

    virtual utl::String toString() const;

    /** Serialize from a stream. */
    virtual void
    serializeIn(std::istream& is)
    {
    }

    /** Has this individual been built? */
    virtual bool isBuilt() const = 0;

    /** Clear the build. */
    virtual void buildClear() = 0;

    /** Does self dominate the given individual? */
    bool dominates(const Ind* rhs, const objective_vector_t& objectives) const;

    /** Get the population that self belongs to. */
    const Population*
    getPop() const
    {
        return _pop;
    }

    /** Get the number of scores. */
    uint_t
    numScores() const
    {
        return _scores.size();
    }

    /** Get the score for the given objective. */
    double
    getScore(uint_t idx = 0) const
    {
        ASSERTD(idx < _scores.size());
        return _scores[idx];
    }

    /** Set the score. */
    void
    setScore(double score)
    {
        setScore(0, score);
    }

    /** Set the score for the given objective. */
    void setScore(uint_t idx, double score);

    /** Get the fitness. */
    double
    fitness() const
    {
        return _fitness;
    }

    /** Get the fitness. */
    double
    getFitness() const
    {
        return _fitness;
    }

    /** Set the fitness. */
    void setFitness(double fitness);

    /** Get the parent score for the given objective. */
    double
    getParentScore(uint_t idx = 0) const
    {
        ASSERTD(idx < _parentScores.size());
        return _parentScores[idx];
    }

    /** Set the parent score for the given objective. */
    void
    setParentScore(uint_t idx, double parentScore)
    {
        if (idx >= _parentScores.size())
            _parentScores.resize(idx + 1);
        _parentScores[idx] = parentScore;
    }

    /** Get operator index. */
    uint_t
    getOpIdx() const
    {
        return _opIdx;
    }

    /** Set operator index. */
    void
    setOpIdx(uint_t opIdx)
    {
        _opIdx = opIdx;
    }

private:
    void init();
    void deInit();

private:
    const Population* _pop;
    std::vector<double> _scores;
    std::vector<double> _parentScores;
    double _fitness;
    uint_t _opIdx;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// IndOrdering /////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Ordering for subclasses of Ind (abstract).

   Override the pure virtual method compare() to implement a customized individual ordering.

   \ingroup gop
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class IndOrdering : public utl::Object
{
    UTL_CLASS_DECL_ABC(IndOrdering, utl::Object);
    UTL_CLASS_NO_COPY;

public:
    /**
       Constructor.
       \param inverted inverted ordering?
    */
    IndOrdering(bool inverted)
    {
        init(inverted);
    }

    /**
       Compare two individuals.  You should override this method to  perform the specialized
       comparison you want.  Any override of this method should check for an inverted comparison
       prior to returning.

       \return < 0 if lhs < rhs, 0 if lhs == rhs, > 0 if lhs > rhs
       \param lhs left-hand-side individual
       \param rhs right-hand-side individual
    */
    virtual int compare(const Ind* lhs, const Ind* rhs) const = 0;

    /** Inverted ordering? */
    bool
    inverted() const
    {
        return _inverted;
    }

    /** Set the inverted flag. */
    void
    setInverted(bool inverted)
    {
        _inverted = inverted;
    }

    /** Compare two individuals. */
    int
    operator()(const Ind* lhs, const Ind* rhs)
    {
        return compare(lhs, rhs);
    }

protected:
    bool _inverted;

private:
    void
    init(bool inverted = false)
    {
        _inverted = inverted;
    }
    void
    deInit()
    {
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// IndOrderingSTL //////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   STL adaptor for IndOrdering.

   \ingroup gop
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

struct IndOrderingSTL
{
    /**
       Constructor.
       \param ordering IndOrdering that compares Ind objects
    */
    IndOrderingSTL(IndOrdering* ordering)
    {
        _ordering = ordering;
    }

    /** Compare two individuals. */
    bool
    operator()(const Ind* lhs, const Ind* rhs) const
    {
        ASSERTD(_ordering != nullptr);
        return (_ordering->compare(lhs, rhs) < 0);
    }

private:
    IndOrdering* _ordering;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// IndScoreOrdering ////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Individual ordering based on score.

   \see Ind::getScore
   \ingroup gop
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class IndScoreOrdering : public IndOrdering
{
    UTL_CLASS_DECL(IndScoreOrdering, IndOrdering);

public:
    /**
       Constructor.
       \param inverted inverted ordering?
    */
    IndScoreOrdering(bool inverted)
        : IndOrdering(inverted)
    {
    }

    /**
       Constructor.
       \param objectiveIdx objective for score comparison
       \param inverted inverted ordering?
    */
    IndScoreOrdering(uint_t objectiveIdx, bool inverted = false)
        : IndOrdering(inverted)
    {
        init(objectiveIdx);
    }

    virtual int compare(const Ind* lhs, const Ind* rhs) const;

private:
    void
    init(uint_t objectiveIdx = 0)
    {
        _objectiveIdx = objectiveIdx;
    }

    void
    deInit()
    {
    }

private:
    uint_t _objectiveIdx;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// IndFitnessOrdering //////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Individual ordering based on fitness.

   \see Ind::getFitness
   \ingroup gop
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class IndFitnessOrdering : public IndOrdering
{
    UTL_CLASS_DECL(IndFitnessOrdering, IndOrdering);
    UTL_CLASS_DEFID;

public:
    /**
       Constructor.
       \param inverted inverted ordering?
    */
    IndFitnessOrdering(bool inverted)
        : IndOrdering(inverted)
    {
    }

    virtual int compare(const Ind* lhs, const Ind* rhs) const;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

GOP_NS_END;
