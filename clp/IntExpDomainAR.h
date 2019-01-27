#ifndef CLP_INTEXPDOMAINAR_H
#define CLP_INTEXPDOMAINAR_H

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <libutl/Vector.h>
#include <clp/IntExpDomain.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Integer expression domain (simple array representation).

   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class IntExpDomainAR : public IntExpDomain
{
    friend class IntExpDomainARit;
    UTL_CLASS_DECL(IntExpDomainAR, IntExpDomain);

public:
    typedef std::set<int> int_set_t;
    typedef std::set<uint_t> uint_set_t;

public:
    /** Constructor. */
    IntExpDomainAR(Manager* mgr);

    /** Constructor. */
    IntExpDomainAR(Manager* mgr, const int_set_t& domain, bool empty = false);

    /** Constructor. */
    IntExpDomainAR(Manager* mgr, const uint_set_t& domain, bool empty = false);

    /** Constructor. */
    IntExpDomainAR(
        Manager* mgr, uint_t num, int* values, bool valuesOwner = true, bool empty = false);

    /** Copy another instance. */
    virtual void copy(const utl::Object& rhs);

    /** Copy flags from another instance. */
    void copyFlags(const IntExpDomainAR* rhs);

    /** Contains the given value? */
    virtual bool has(int val) const;

    /** Test equality of flags. */
    bool
    flagsEqual(const IntExpDomainAR* rhs) const
    {
        ASSERTD(_flagsSize == rhs->_flagsSize);
        if (_size != rhs->_size)
            return false;
        if (_flagsSize == 1)
            return (*_flags == *rhs->_flags);
        return (memcmp(_flags, rhs->_flags, _flagsSize * 4) == 0);
    }

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
            _saveState();
    }

    virtual void _saveState();

    virtual void addRange(int min, int max);

    virtual void removeRange(int min, int max);

private:
    void init();
    void init(const std::set<int>& domain, bool empty);
    void init(uint_t num, int* values, bool valuesOwner, bool empty);
    void deInit();

    uint_t findValueIdx(int val) const;

    uint_t findForward(uint_t idx) const;

    uint_t findBackward(uint_t idx) const;

    bool getFlag(uint_t idx) const;

    bool setFlag(uint_t idx);

    bool clearFlag(uint_t idx);

private:
    uint_t _num;
    bool _valuesOwner;
    int* _values;
    uint_t _flagsSize;

    // reversible /////////////////////////////////////
    uint_t _stateDepth;
    uint32_t* _flags;
    // reversible /////////////////////////////////////
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_END;

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
