#include "libcls.h"
#include <libutl/AutoPtr.h>
#include "CompositeResource.h"
#include "CompositeResourceRequirement.h"
#include "IntActivity.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;
CLP_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(cls::CompositeResourceRequirement);

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

CompositeResourceRequirement::CompositeResourceRequirement(IntActivity* act,
                                                           CompositeResource* res,
                                                           uint_t minCap,
                                                           uint_t maxCap)
{
    _act = act;
    _res = res;
    _minCap = minCap;
    _maxCap = maxCap;
    _preferredResources = nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int
CompositeResourceRequirement::compare(const Object& rhs) const
{
    ASSERTD(rhs.isA(CompositeResourceRequirement));
    const CompositeResourceRequirement& cr = (const CompositeResourceRequirement&)rhs;
    int res = utl::compare(_res->serialId(), cr._res->serialId());
    if (res != 0)
        return res;
    res = utl::compare((void*)_res, (void*)cr._res);
    return res;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Manager*
CompositeResourceRequirement::manager() const
{
    return _act->manager();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
CompositeResourceRequirement::init()
{
    _act = nullptr;
    _res = nullptr;
    _minCap = 0;
    _maxCap = 0;
    _preferredResources = nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
CompositeResourceRequirement::deInit()
{
    delete _preferredResources;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_END;
