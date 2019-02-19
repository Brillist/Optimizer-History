#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <gop/StringInd.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

GOP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Population of individuals.

   Population is a simple container for a group of individuals.  To allow efficient random access,
   the individuals are stored internally in an array.

   \ingroup gop
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class Population : public utl::Object
{
    UTL_CLASS_DECL(Population, utl::Object);

public:
    // friends
    friend class Ind;

    /** Iterator. */
    using iterator = std::vector<StringInd<uint_t>*>::iterator;

    /** Const iterator. */
    using const_iterator = std::vector<StringInd<uint_t>*>::const_iterator;

public:
    /**
       Constructor.
       \param owner ownership flag for contained objects
    */
    Population(bool owner)
    {
        init(owner);
    }

    /** Copy another instance. */
    virtual void copy(const utl::Object& rhs);

    /** Remove all individuals. */
    void clear();

    /// \name Accessors (const)
    //@{
    /** Get the ownership flag. */
    bool
    isOwner() const
    {
        return _owner;
    }

    /** Get a pointer to the individual at the given index. */
    const StringInd<uint_t>*
    get(uint_t idx) const
    {
        ASSERTD(idx < size());
        return _inds[idx];
    }

    /** Get a pointer to the individual at the given index. */
    const StringInd<uint_t>*
    operator[](uint_t idx) const
    {
        return get(idx);
    }

    /** Get a reference to the individual at the given index. */
    const StringInd<uint_t>&
    operator()(uint_t idx) const
    {
        return *get(idx);
    }

    /** Empty? */
    bool
    empty() const
    {
        return _inds.empty();
    }

    /** Return number of individuals. */
    uint_t
    size() const
    {
        return _inds.size();
    }

    /** Get begin iterator. */
    const_iterator
    begin() const
    {
        return _inds.begin();
    }

    /** Get end iterator. */
    const_iterator
    end() const
    {
        return _inds.end();
    }
    //@}

    /// \name Accessors (non-const)
    //@{
    /** Set the ownership flag. */
    void
    setOwner(bool owner)
    {
        _owner = owner;
    }

    /** Get the individual at the given index. */
    StringInd<uint_t>*
    get(uint_t idx)
    {
        ASSERTD(idx < size());
        return _inds[idx];
    }

    /** Set the individual at the given index. */
    void
    set(uint_t idx, const StringInd<uint_t>& ind)
    {
        set(idx, (StringInd<uint_t>&)ind);
    }

    /** Set the individual at the given index. */
    void
    set(uint_t idx, StringInd<uint_t>& ind)
    {
        if (isOwner())
        {
            set(idx, ind.clone());
        }
        else
        {
            set(idx, &ind);
        }
    }

    /** Set the individual at the given index. */
    void set(uint_t idx, StringInd<uint_t>* ind);

    /** Get a pointer to the individual at the given index. */
    StringInd<uint_t>*
    operator[](uint_t idx)
    {
        return get(idx);
    }

    /** Get a reference to the individual at the given index. */
    StringInd<uint_t>&
    operator()(uint_t idx)
    {
        return *get(idx);
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
    //@}

    /// \name Calculations
    //@{
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
    //@}

    /// \name Modification
    //@{
    /**
       Add the given population's individuals.
       \param rhs source Population
       \param beginIdx (optional) begin index for items copied from rhs
       \param endIdx (optional) end index for items copied from rhs
    */
    void add(const Population& rhs, uint_t beginIdx = 0, uint_t endIdx = uint_t_max);

    /** Add an individual. */
    void
    add(const StringInd<uint_t>& ind)
    {
        add(const_cast<StringInd<uint_t>&>(ind));
    }

    /** Add an individual. */
    void
    add(StringInd<uint_t>& ind)
    {
        if (isOwner())
        {
            add(ind.clone());
        }
        else
        {
            add(&ind);
        }
    }

    /** Add an individual. */
    void add(StringInd<uint_t>* ind);

    /** Reserve space in the underlying \c std::vector (to avoid repeated resizing). */
    void
    reserve(uint_t numInds)
    {
        _inds.reserve(numInds);
    }

    /** Resize the underlying \c std::vector. */
    void
    setSize(uint_t newSize)
    {
        _inds.resize(newSize);
    }

    /** Sort using the given ordering. */
    void sort(IndOrdering* ordering);

    /** Randomly shuffle the individuals. */
    void shuffle(lut::rng_t& rng);
    //@}

private:
    void init(bool owner = true);
    void deInit();

    void onAddRemoveInd() const;
    void onChangeScore() const;
    void onChangeFitness() const;

private:
    mutable double _totalScore;    // total score
    mutable double _totalFitness;  // total fitness
    bool _owner;                   // ownership flag
    std::vector<StringInd<uint_t>*> _inds;  // individuals
};

////////////////////////////////////////////////////////////////////////////////////////////////////

GOP_NS_END;
