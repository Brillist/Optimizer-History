#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <libutl/Vector.h>
#include <clp/IntExpDomain.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Integer expression domain (Counted Array Representation).

   IntExpDomainCAR records the domain of a CountedIntVar.

   \see CountedIntVar
   \ingroup clp
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class IntExpDomainCAR : public IntExpDomain
{
    friend class IntExpDomainCARit;
    UTL_CLASS_DECL(IntExpDomainCAR, IntExpDomain);

public:
    using int_uint_map_t = std::map<int, uint_t>;

public:
    /** Constructor. */
    IntExpDomainCAR(Manager* mgr);

    /** Constructor. */
    IntExpDomainCAR(Manager* mgr, const int_uint_map_t& domain);

    virtual void copy(const utl::Object& rhs);

    /**
       Decrement the count for a value.
       \param val value to decrement the count for
       \param num amount to deduct (default is 1)
    */
    uint_t decrement(int val, uint_t num = 1);

    /// \name Accessors (const)
    //@{
    virtual bool has(int val) const;
    virtual IntExpDomainIt* begin() const;
    virtual IntExpDomainIt* end() const;
    virtual int getPrev(int val) const;
    virtual int getNext(int val) const;
    //@}

protected:
    void
    saveState()
    {
        if (_stateDepth < _mgr->depth())
            saveState();
    }

    virtual void _saveState();

    virtual void removeRange(int min, int max);

private:
    void init();
    void init(const int_uint_map_t& domain);
    void deInit();

    uint_t getCount(uint_t idx) const;

    bool
    zeroCount(uint_t idx)
    {
        return (setCount(idx, 0) != 0);
    }

    uint_t setCount(uint_t idx, uint_t count);

private:
    using int_vector_t = utl::Vector<int>;
    using uint32_vector_t = utl::Vector<uint32_t>;

private:
    uint_t _num;
    uint_t _bits;
    uint_t _bitsLog2;
    uint_t _countsPerWord;
    uint_t _countsPerWordLog2;
    uint_t _countsPerWordMask;
    uint32_t _mask;
    int_vector_t _valuesArray;
    uint32_vector_t _countsArray;
    int* _values;

    // reversible /////////////////////////////////////
    uint32_t* _counts;
    uint_t _stateDepth;
    // reversible /////////////////////////////////////
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_END;
