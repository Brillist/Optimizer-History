#ifndef MRP_PROCESSPLAN_H
#define MRP_PROCESSPLAN_H

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <libutl/Array.h>
#include <mrp/PlanStepRelation.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

MRP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Process plan.

   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class ProcessPlan : public utl::Object
{
    UTL_CLASS_DECL(ProcessPlan);

public:
    virtual void copy(const utl::Object& rhs);

    virtual int compare(const utl::Object& rhs) const;

    virtual void
    serialize(utl::Stream& stream, utl::uint_t io, utl::uint_t mode = utl::ser_default);

    /// \name Accessors
    //@{
    /** Id. */
    utl::uint_t
    id() const
    {
        return _id;
    }

    /** Id. */
    utl::uint_t&
    id()
    {
        return _id;
    }

    /** Name. */
    const std::string&
    name() const
    {
        return _name;
    }

    /** Name. */
    std::string&
    name()
    {
        return _name;
    }

    /** Version. */
    const std::string&
    version() const
    {
        return _version;
    }

    /** Version. */
    std::string&
    version()
    {
        return _version;
    }

    /** Steps. */
    const planstep_vector_t&
    planSteps() const
    {
        return _plansteps;
    }

    /** Steps. */
    planstep_vector_t&
    planSteps()
    {
        return _plansteps;
    }
    //@}

    utl::String toString() const;

private:
    void
    init()
    {
        _id = utl::uint_t_max;
    }
    void
    deInit()
    {
    }

private:
    utl::uint_t _id;
    std::string _name;
    std::string _version;
    planstep_vector_t _plansteps;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

struct PlanOrderingIncId : public std::binary_function<ProcessPlan*, ProcessPlan*, bool>
{
    bool operator()(const ProcessPlan* lhs, const ProcessPlan* rhs) const;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

typedef std::set<ProcessPlan*, PlanOrderingIncId> plan_set_id_t;

////////////////////////////////////////////////////////////////////////////////////////////////////

MRP_NS_END;

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
