#ifndef CLP_CONSTRAINT_H
#define CLP_CONSTRAINT_H

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <clp/Goal.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Constraint (abstract).

   A constraint is basically a rule that specifies some relationship between
   constrained variables.  When a constraint is posted (post()), it requests
   to be notified when domain reduction occurs in the variables it is
   concerned with.  When the constraint executes, it looks at the domain
   reductions that have occurred since it last ran, and possibly does
   additional domain reduction to ensure satisfaction of the rule it is
   charged with enforcing.

   \see ConstrainedVar
   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class Constraint : public Goal
{
    UTL_CLASS_DECL_ABC(Constraint);

public:
    /** Constructor. */
    Constraint(Manager* mgr)
        : Goal(mgr)
    {
        init();
    }

    virtual void copy(const utl::Object& rhs);

    /** Return a fully managed copy of self. */
    virtual Constraint* mclone();

    /** Make managed copies of unmanaged referenced objects. */
    virtual void
    mcopy()
    {
    }

    /** Post constraints for expressions. */
    virtual void
    postExpConstraints()
    {
    }

    /** Post the constraint. */
    virtual void post() = 0;

    /** Remove the constraint. */
    virtual void unpost() = 0;

    /** Get the depth of posting. */
    utl::uint_t
    getPostDepth() const
    {
        return _postDepth;
    }

    /** Get the \b posted flag. */
    bool
    isPosted() const
    {
        return _posted;
    }

    /** Set the \b posted flag. */
    void setPosted(bool posted);

    /** Get the \b managed flag. */
    bool
    isManaged() const
    {
        return _managed;
    }

    /** Set the \b managed flag. */
    void
    setManaged(bool managed)
    {
        _managed = managed;
    }

private:
    void init();
    void
    deInit()
    {
        ASSERTD(!isPosted());
    }

private:
    bool _posted;
    bool _managed;
    utl::uint_t _postDepth;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_END;

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
