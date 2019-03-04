#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <libutl/Vector.h>
#include <clp/IntExpDomain.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Integer expression domain (array representation).

   IntExpDomainAR is a domain representation for IntExp that begins with a set of values,
   and records the presence or absence of those values with a single bit.  It's an efficient
   domain representation if the difference between the smallest and largest initial domain
   values is not very large.

   \ingroup clp
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class IntExpDomainAR : public IntExpDomain
{
    friend class IntExpDomainARit;
    UTL_CLASS_DECL(IntExpDomainAR, IntExpDomain);

public:
    using int_set_t = std::set<int>;
    using uint_set_t = std::set<uint_t>;

public:
    /**
       Constructor.
       \param mgr associated Manager
    */
    IntExpDomainAR(Manager* mgr);

    /**
       Constructor.
       \param mgr associated Manager
       \param domain initial domain (a set of signed integer values)
       \param empty initialize as empty?
    */
    IntExpDomainAR(Manager* mgr, const int_set_t& domain, bool empty = false);

    /**
       Constructor.
       \param mgr associated Manager
       \param domain initial domain (a set of usigned integer values)
       \param empty initialize as empty?
    */
    IntExpDomainAR(Manager* mgr, const uint_set_t& domain, bool empty = false);

    /**
       Constructor.
       \param mgr associated Manager
       \param num number of values
       \param values array of signed integer values
       \param valuesOwner take ownership of `values`?
       \param empty initialize as empty?
    */
    IntExpDomainAR(
        Manager* mgr, uint_t num, int* values, bool valuesOwner = true, bool empty = false);

    virtual void copy(const utl::Object& rhs);

    /** Copy flags from another instance. */
    void copyFlags(const IntExpDomainAR* rhs);

    /** Test equality of flags with another instance. */
    bool flagsEqual(const IntExpDomainAR* rhs) const;

    virtual bool has(int val) const;

    virtual IntExpDomainIt* begin() const;

    virtual IntExpDomainIt* end() const;

    virtual int getPrev(int val) const;

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
