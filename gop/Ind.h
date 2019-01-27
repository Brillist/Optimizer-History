#ifndef GOP_IND_H
#define GOP_IND_H

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <functional>

////////////////////////////////////////////////////////////////////////////////////////////////////

GOP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

class OSID;
class Objective;
class Population;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Ind ////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Individual problem solution (abstract).

   Ind is an abstract base for classes that represent individual problem
   solutions.  For example, in scheduling problems, a schedule specifying
   start/end times for activities is an individual.

   \see IndBuilder
   \see Population
   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class Ind : public utl::Object
{
    UTL_CLASS_DECL_ABC(Ind, utl::Object);

public:
    /** Copy another instance. */
    virtual void copy(const utl::Object& rhs);

    /** Serialize from a stream. */
    virtual void
    serializeIn(std::istream& is)
    {
    }

    /** Has self been built? */
    virtual bool isBuilt() const = 0;

    /** Clear the build. */
    virtual void buildClear() = 0;

    /** Does self dominate the given individual? */
    bool dominates(const Ind* rhs, const std::vector<Objective*>& objectives) const;

    /** Get a human-readable string representation. */
    virtual utl::String toString() const;

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

    /** Get the optimizer-specific related data. */
    const OSID*
    osid() const
    {
        return _osid;
    }

    /** Get the optimizer-specific related data. */
    OSID*
    osid()
    {
        return _osid;
    }

    /** Set optimizer-specific related data. */
    void setOSID(OSID* osid);

    /** Clear optimizer-specific related data. */
    void clearOSID();

private:
    void init();
    void deInit();

    const Population* _pop;
    std::vector<double> _scores;
    std::vector<double> _parentScores;
    double _fitness;
    uint_t _opIdx;

    // optimizer-specific related data
    OSID* _osid;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
/// OSID ///////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Abstract base for optimizer-specific individual data.

   Subclasses of Optimizer may need to store additional information in
   the individual.  OSID serves as an abstract base for classes that store
   this extra individual-associated information.

   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class OSID : public utl::Object
{
    UTL_CLASS_DECL_ABC(OSID, utl::Object);
    UTL_CLASS_DEFID;

public:
    /** Copy another instance. */
    virtual void
    copy(const utl::Object& rhs)
    {
    }

    /** Clear data. */
    virtual void
    clear()
    {
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////
/// IndOrdering ////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Abstract base for individual ordering strategies.

   Override to pure virtual method compare() to implement a customized
   individual ordering.

   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class IndOrdering : public utl::Object
{
    UTL_CLASS_DECL_ABC(IndOrdering, utl::Object);
    UTL_CLASS_NO_COPY;

public:
    /** Constructor. */
    IndOrdering(bool inverted)
    {
        init(inverted);
    }

    /**
       Compare two individuals.  You should override this method to
       perform the specialized comparison you want.  Any override of this
       method should check for an inverted comparison prior to returning.

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
/// IndOrderingSTL /////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   STL adaptor for IndOrdering.

   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

struct IndOrderingSTL : public std::binary_function<Ind*, Ind*, bool>
{
    /** Constructor. */
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
/// IndScoreOrdering ///////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Individual ordering based on score.

   \see Ind::getScore
   \author Adam McKee
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
       \param objectiveIdx objective to get score for
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
/// IndFitnessOrdering /////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Individual ordering based on fitness.

   \see Ind::getFitness
   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class IndFitnessOrdering : public IndOrdering
{
    UTL_CLASS_DECL(IndFitnessOrdering, IndOrdering);
    UTL_CLASS_DEFID;

public:
    /** Constructor. */
    IndFitnessOrdering(bool inverted)
        : IndOrdering(inverted)
    {
    }

    virtual int compare(const Ind* lhs, const Ind* rhs) const;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

GOP_NS_END;

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
