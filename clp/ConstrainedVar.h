#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Constrained variable.

   A constrained variable has a finite domain, and may be subject to
   domain reduction in the course of constraint propagation.

   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class ConstrainedVar : public utl::Object
{
    UTL_CLASS_DECL_ABC(ConstrainedVar, utl::Object);
    UTL_CLASS_NO_COPY;
    UTL_CLASS_DEFID;

public:
    /** Restore domain to last choice point. */
    virtual void backtrack() = 0;

    /** Is self managed? */
    virtual bool
    isManaged() const
    {
        return true;
    }

    /** Indicate whether self is managed. */
    virtual void
    setManaged(bool managed)
    {
        ABORT();
    }

    /** Post associated constraints. */
    virtual void
    postConstraints()
    {
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_END;
