#include "libclp.h"
#include "Constraint.h"
#include "Manager.h"

//////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;

//////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL_ABC(clp::Constraint, clp::Goal);

//////////////////////////////////////////////////////////////////////////////

CLP_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////

void
Constraint::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(Constraint));
    const Constraint& ct = (const Constraint&)rhs;
    Goal::copy(ct);
    _posted = false;
}

//////////////////////////////////////////////////////////////////////////////

Constraint*
Constraint::mclone()
{
    Constraint* ct;
    if (isManaged())
    {
        ct = this;
        ct->mcopy();
    }
    else
    {
        ct = self.clone();
        ct->mcopy();
        Manager* mgr = manager();
        ASSERTD(mgr != nullptr);
        mgr->add(ct);
    }
    return ct;
}

//////////////////////////////////////////////////////////////////////////////

void
Constraint::setPosted(bool posted)
{
    if (posted == _posted)
    {
        return;
    }
    _posted = posted;
    if (_posted)
    {
        Manager* mgr = manager();
        ASSERTD(mgr != nullptr);
        _postDepth = mgr->depth();
    }
    else
    {
        _postDepth = 0;
    }
}

//////////////////////////////////////////////////////////////////////////////

void
Constraint::init()
{
    _posted = false;
    _managed = false;
    _postDepth = 0;
}

//////////////////////////////////////////////////////////////////////////////

CLP_NS_END;
