#ifndef CLP_INTEXPDOMAINCAR_H
#define CLP_INTEXPDOMAINCAR_H

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <libutl/Vector.h>
#include <clp/IntExpDomain.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Integer expression domain (counted array representation).

   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class IntExpDomainCAR : public IntExpDomain
{
    friend class IntExpDomainCARit;
    UTL_CLASS_DECL(IntExpDomainCAR, IntExpDomain);

public:
    typedef std::map<int, uint_t> int_uint_map_t;

public:
    /** Constructor. */
    IntExpDomainCAR(Manager* mgr);

    /** Constructor. */
    IntExpDomainCAR(Manager* mgr, const int_uint_map_t& domain);

    /** Copy another instance. */
    virtual void copy(const utl::Object& rhs);

    /** Decrement the count for the given value. */
    uint_t decrement(int val, uint_t num = 1);

    /** Contains the given value? */
    virtual bool has(int val) const;

    /** Get begin iterator. */
    virtual IntExpDomainIt* begin() const;

    /** Get end iterator. */
    virtual IntExpDomainIt* end() const;

    /** Get the maximum value < val. */
    virtual int getPrev(int val) const;

    /** Get the minimum value > val. */
    virtual int getNext(int val) const;

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
    typedef utl::Vector<int> int_vector_t;
    typedef utl::Vector<uint32_t> uint32_vector_t;

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

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
