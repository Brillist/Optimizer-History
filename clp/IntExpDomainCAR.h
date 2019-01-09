#ifndef CLP_INTEXPDOMAINCAR_H
#define CLP_INTEXPDOMAINCAR_H

//////////////////////////////////////////////////////////////////////////////

#include <libutl/Vector.h>
#include <clp/IntExpDomain.h>

//////////////////////////////////////////////////////////////////////////////

CLP_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////

/**
   Integer expression domain (counted array representation).

   \author Adam McKee
*/

//////////////////////////////////////////////////////////////////////////////

class IntExpDomainCAR : public IntExpDomain
{
    friend class IntExpDomainCARit;
    UTL_CLASS_DECL(IntExpDomainCAR);
public:
    typedef std::map<int, utl::uint_t> int_uint_map_t;
public:
    /** Constructor. */
    IntExpDomainCAR(Manager* mgr);

    /** Constructor. */
    IntExpDomainCAR(Manager* mgr, const int_uint_map_t& domain);

    /** Copy another instance. */
    virtual void copy(const utl::Object& rhs);

    /** Decrement the count for the given value. */
    utl::uint_t decrement(int val, utl::uint_t num = 1);

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
    void saveState()
        { if (_stateDepth < _mgr->depth()) saveState(); }

    virtual void _saveState();

    virtual void removeRange(int min, int max);
private:
    void init();
    void init(const int_uint_map_t& domain);
    void deInit();

    utl::uint_t getCount(utl::uint_t idx) const;

    bool zeroCount(utl::uint_t idx)
    { return (setCount(idx, 0) != 0); }

    utl::uint_t setCount(utl::uint_t idx, utl::uint_t count);
private:
    typedef utl::Vector<int> int_vector_t;
    typedef utl::Vector<utl::uint32_t> uint32_vector_t;
private:
    utl::uint_t _num;
    utl::uint_t _bits;
    utl::uint_t _bitsLog2;
    utl::uint_t _countsPerWord;
    utl::uint_t _countsPerWordLog2;
    utl::uint_t _countsPerWordMask;
    utl::uint32_t _mask;
    int_vector_t _valuesArray;
    uint32_vector_t _countsArray;
    int* _values;

    // reversible ///////////////
    utl::uint32_t* _counts;
    utl::uint_t _stateDepth;
    // reversible ///////////////
};

//////////////////////////////////////////////////////////////////////////////

CLP_NS_END;

//////////////////////////////////////////////////////////////////////////////

#endif
