#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <cse/JobOp.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Summary JobOp.

   A SummaryOp is a special type of JobOp that has a set of child JobOp%s.

   \ingroup cse
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class SummaryOp : public JobOp
{
    UTL_CLASS_DECL(SummaryOp, JobOp);
    UTL_CLASS_DEFID;

public:
    virtual void copy(const utl::Object& rhs);

    virtual String toString() const;

    /// \name Accessors (const)
    /** Get the set of direct child ops. */
    const jobop_set_id_t&
    childOps() const
    {
        return _childOps;
    }

    /** Get the precedence-node for the end-time. */
    const clp::CycleGroup*
    endCycleGroup() const
    {
        auto act = activity();
        ASSERTD(act != nullptr);
        return act->efBound().cycleGroup();
    }
    //@}

    /** Add a child op. */
    void addChildOp(JobOp* op)
    {
        _childOps.insert(op);
    }

private:
    jobop_set_id_t _childOps;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

using summaryop_vector_t = std::vector<SummaryOp*>;

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;
