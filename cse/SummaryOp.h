#ifndef CSE_SUMMARYOP_H
#define CSE_SUMMARYOP_H

//////////////////////////////////////////////////////////////////////////////

#include <cse/JobOp.h>

//////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////

/**
   Summary operation.

   \author Adam McKee
*/

//////////////////////////////////////////////////////////////////////////////

class SummaryOp : public JobOp
{
    UTL_CLASS_DECL(SummaryOp);
    UTL_CLASS_DEFID;
public:
    /** Copy another instance. */
    virtual void copy(const utl::Object& rhs);

    /** Get the set of direct child ops. */
    const jobop_set_id_t& childOps() const
        { return _childOps; }

    /** Get the set of direct child ops. */
    jobop_set_id_t& childOps()
        { return _childOps; }

    /** Get the precedence-node for the end-time. */
    const clp::CycleGroup* endCycleGroup() const
    {
        cls::Activity* act = activity();
        ASSERTD(act != nullptr);
        return act->efBound().cycleGroup();
    }

    utl::String toString() const;
private:
    jobop_set_id_t _childOps;
};

//////////////////////////////////////////////////////////////////////////////

typedef std::vector<SummaryOp*> summaryop_vect_t;

//////////////////////////////////////////////////////////////////////////////

CSE_NS_END;

//////////////////////////////////////////////////////////////////////////////

#endif
