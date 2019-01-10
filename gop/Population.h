#ifndef GOP_POPULATION_H
#define GOP_POPULATION_H

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <libutl/RandNumGen.h>
#include <gop/StringInd.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

GOP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Population of individuals.

   Population is a simple container class that represents a grouping of
   individuals (instances of Ind).  The individuals are stored internally
   in an array, to allow fast random access.

   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class Population : public utl::Object
{
    UTL_CLASS_DECL(Population);

public:
    // friends
    friend class Ind; //??
    // typedefs
    typedef std::vector<StringInd<utl::uint_t>*>::iterator iterator;
    typedef std::vector<StringInd<utl::uint_t>*>::const_iterator const_iterator;

public:
    /** Constructor. */
    Population(bool owner)
    {
        init(owner);
    }

    /** Copy another instance. */
    virtual void copy(const utl::Object& rhs);

    /** Remove all individuals. */
    void clear();

    /** Get the ownership flag. */
    bool
    isOwner() const
    {
        return _owner;
    }

    /** Set the ownership flag. */
    void
    setOwner(bool owner)
    {
        _owner = owner;
    }

    /** Set the size. */
    void
    setSize(utl::uint_t newSize)
    {
        _inds.resize(newSize);
    }

    /** Add all of the given population's individuals. */
    void add(const Population& rhs, utl::uint_t beginIdx = 0, utl::uint_t endIdx = utl::uint_t_max);

    /** Add an individual. */
    void
    add(const StringInd<utl::uint_t>& ind)
    {
        add((StringInd<utl::uint_t>&)ind);
    }

    /** Add an individual. */
    void
    add(StringInd<utl::uint_t>& ind)
    {
        if (isOwner())
            add(ind.clone());
        else
        {
            add(&ind);
        }
    }

    /** Add an individual. */
    void add(StringInd<utl::uint_t>* ind);

    /** Get the individual at the given index. */
    const StringInd<utl::uint_t>*
    get(utl::uint_t idx) const
    {
        ASSERTD(idx < size());
        return _inds[idx];
    }

    /** Get the individual at the given index. */
    StringInd<utl::uint_t>*
    get(utl::uint_t idx)
    {
        ASSERTD(idx < size());
        return _inds[idx];
    }

    /** Set the individual at the given index. */
    void
    set(utl::uint_t idx, const StringInd<utl::uint_t>& ind)
    {
        set(idx, (StringInd<utl::uint_t>&)ind);
    }

    /** Set the individual at the given index. */
    void
    set(utl::uint_t idx, StringInd<utl::uint_t>& ind)
    {
        if (isOwner())
            set(idx, ind.clone());
        else
        {
            set(idx, &ind);
        }
    }

    /** Set the individual at the given index. */
    void set(utl::uint_t idx, StringInd<utl::uint_t>* ind);

    /** Get the individual at the given index. */
    const StringInd<utl::uint_t>&
    operator()(utl::uint_t idx) const
    {
        return *get(idx);
    }

    /** Get the individual at the given index. */
    StringInd<utl::uint_t>&
    operator()(utl::uint_t idx)
    {
        return *get(idx);
    }

    /** Get the individual at the given index. */
    const StringInd<utl::uint_t>* operator[](utl::uint_t idx) const
    {
        return get(idx);
    }

    /** Get the individual at the given index. */
    StringInd<utl::uint_t>* operator[](utl::uint_t idx)
    {
        return get(idx);
    }

    /** Empty? */
    bool
    empty() const
    {
        return _inds.empty();
    }

    /** Return number of individuals. */
    utl::uint_t
    size() const
    {
        return _inds.size();
    }

    /** Reserve space for the given number of individuals. */
    void
    reserve(utl::uint_t numInds)
    {
        _inds.reserve(numInds);
    }

    /** Sort using the given ordering. */
    void sort(IndOrdering* ordering);

    /** Randomly shuffle the individuals. */
    void
    shuffle()
    {
        shuffle(nullptr);
    }

    /** Randomly shuffle the individuals. */
    void
    shuffle(utl::RandNumGen& rng)
    {
        shuffle(&rng);
    }

    /** Randomly shuffle the individuals. */
    void shuffle(utl::RandNumGen* rng);

    /** Get total score of all individuals. */
    double totalScore() const;

    /** Get average score of all individuals. */
    double
    averageScore() const
    {
        return totalScore() / (double)size();
    }

    /** Get the standard deviation of the score. */
    double stdDevScore() const;

    /** Get total fitness of all individuals. */
    double totalFitness() const;

    /** Get average fitness of all individuals. */
    double
    averageFitness() const
    {
        return totalFitness() / (double)size();
    }

    /** Get the standard deviation of the fitness. */
    double stdDevFitness() const;

    /** Get begin iterator (const). */
    const_iterator
    begin() const
    {
        return _inds.begin();
    }

    /** Get end iterator (const). */
    const_iterator
    end() const
    {
        return _inds.end();
    }

    /** Get begin iterator. */
    iterator
    begin()
    {
        return _inds.begin();
    }

    /** Get end iterator. */
    iterator
    end()
    {
        return _inds.end();
    }

private:
    void init(bool owner = true);
    void deInit();

    void onAddRemoveInd() const;
    void onChangeScore() const;
    void onChangeFitness() const;

private:
    /** Total score. */
    mutable double _totalScore;
    /** Total fitness. */
    mutable double _totalFitness;

    /** Ownership flag. */
    bool _owner;

    /** Individuals. */
    std::vector<StringInd<utl::uint_t>*> _inds;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

GOP_NS_END;

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
