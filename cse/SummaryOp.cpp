#include "libcse.h"
#include <libutl/BufferedFDstream.h>
#include "SummaryOp.h"
#include "Job.h"

//////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;
CLS_NS_USE;

//////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(cse::SummaryOp, cse::JobOp);

//////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////

void
SummaryOp::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(SummaryOp));
    const SummaryOp& so = (const SummaryOp&)rhs;
    JobOp::copy(so);
    _childOps.clear();
}

//////////////////////////////////////////////////////////////////////////////

String
SummaryOp::toString() const
{
    MemStream str;
    str << "summaryOp:" << id()
        << ", job:" << job()->id()
        << ", childOps:";
    jobop_set_id_t::const_iterator it;
    for (it = _childOps.begin(); it != _childOps.end(); it++)
    {
        if (it != _childOps.begin()) str << ", ";
        str << (*it)->id();
    }
    str << '\0';
    return String((char*)str.get());
}

//////////////////////////////////////////////////////////////////////////////

CSE_NS_END;
