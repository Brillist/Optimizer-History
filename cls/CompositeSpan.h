#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <clp/IntExpDomainAR.h>
#include <clp/IntSpan.h>
#include <cls/IntActivity.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Available DiscreteResource%s during a time period.

   CompositeTimetableDomain stores CompositeSpan objects in a skip-list.

   \see CompositeTimetableDomain
   \ingroup clp
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class CompositeSpan : public clp::IntSpan
{
    UTL_CLASS_DECL(CompositeSpan, clp::IntSpan);

public:
    /**
       \param min start of time span
       \param max end of time span
       \param resIds available resource ids (default: null)
       \param level (level within skip-list).
    */
    CompositeSpan(int min,
                  int max,
                  clp::IntExpDomainAR* resIds = nullptr,
                  uint_t level = uint_t_max);

    virtual void copy(const utl::Object& rhs);

    virtual String toString() const;

    virtual bool canMergeWith(const clp::IntSpan* rhs) const;

    virtual uint_t capacity() const;

    /// \name Accessors (const)
    //@{
    /** Get resource ids. */
    const clp::IntExpDomainAR*
    resIds() const
    {
        return *reinterpret_cast<const clp::IntExpDomainAR* const*>(&_v0);
    }
    //@}

    /// \name Accessors (non-const)
    //@{
    /** Get resource ids. */
    clp::IntExpDomainAR*
    resIds()
    {
        return *reinterpret_cast<clp::IntExpDomainAR**>(&_v0);
    }

    /** Set resource ids. */
    void
    setResIds(clp::IntExpDomainAR* resIds)
    {
        *reinterpret_cast<clp::IntExpDomainAR**>(&_v0) = resIds;
    }
    //@}

private:
    void init();
    void deInit();
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_END;
