#ifndef CLS_COMPOSITESPAN_H
#define CLS_COMPOSITESPAN_H

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <clp/IntExpDomainAR.h>
#include <clp/IntSpan.h>
#include <cls/IntActivity.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

class CompositeSpan : public clp::IntSpan
{
    UTL_CLASS_DECL(CompositeSpan, clp::IntSpan);

public:
    CompositeSpan(int min,
                  int max,
                  clp::IntExpDomainAR* resIds = nullptr,
                  uint_t level = uint_t_max);

    virtual void copy(const utl::Object& rhs);

    virtual String toString() const;

    virtual bool canMergeWith(const clp::IntSpan* rhs) const;

    virtual uint_t capacity() const;

    const clp::IntExpDomainAR*
    resIds() const
    {
        return *(const clp::IntExpDomainAR**)&_v0;
    }

    clp::IntExpDomainAR*
    resIds()
    {
        return *(clp::IntExpDomainAR**)&_v0;
    }

    void
    setResIds(clp::IntExpDomainAR* resIds)
    {
        (*(clp::IntExpDomainAR**)&_v0) = resIds;
    }

    void
    copyFlags(const CompositeSpan* rhs)
    {
        ASSERTD(resIds() != nullptr);
        resIds()->copyFlags(rhs->resIds());
    }

private:
    void init();
    void deInit();
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_END;

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
