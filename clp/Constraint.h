#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <clp/Goal.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Constraint (abstract).

   A constraint enforces a relationship between constrained variables.  When a constraint is posted
   (post()), it requests to be executed when domain reduction occurs in the variables it is
   concerned with.

   \ingroup clp
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class Constraint : public Goal
{
    UTL_CLASS_DECL_ABC(Constraint, Goal);

public:
    /** Constructor. */
    Constraint(Manager* mgr)
        : Goal(mgr)
    {
        init();
    }

    virtual void copy(const utl::Object& rhs);

    /** Return a managed copy of self. */
    virtual Constraint* mclone();

    /** Make managed copies of unmanaged referenced objects. */
    virtual void mcopy();

    /** Post constraints for expressions. */
    virtual void postExpConstraints();

    /** Post the constraint. */
    virtual void post() = 0;

    /** Remove the constraint. */
    virtual void unpost() = 0;

    /// \name Accessors (const)
    //@{
    /** Get the depth of posting. */
    uint_t
    postDepth() const
    {
        return _postDepth;
    }

    /** Get the \b posted flag. */
    bool
    posted() const
    {
        return _posted;
    }

    /** Get the \b managed flag. */
    bool
    managed() const
    {
        return _managed;
    }
    //@}

    /// \name Accessors (non-const)
    //@{
    /** Set the \b posted flag. */
    void setPosted(bool posted);

    /** Set the \b managed flag. */
    void
    setManaged(bool managed)
    {
        _managed = managed;
    }
    //@}

private:
    void init();
    void
    deInit()
    {
        ASSERTD(!posted());
    }

private:
    bool _posted;
    bool _managed;
    uint_t _postDepth;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_END;
