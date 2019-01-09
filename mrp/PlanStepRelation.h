#ifndef MRP_PLANSTEPRELATION_H
#define MRP_PLANSTEPRELATION_H

//////////////////////////////////////////////////////////////////////////////

#include <libutl/Object.h>
#include <cse/PrecedenceCt.h>
#include <mrp/ProcessStep.h>

//////////////////////////////////////////////////////////////////////////////

MRP_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////

class ProcessPlan;
class ProcessStep;

//////////////////////////////////////////////////////////////////////////////

/**
   Every process plan's process steps and their precedence relations.

   \author Joe Zhou
*/

//////////////////////////////////////////////////////////////////////////////

class PlanStepRelation : public utl::Object
{
    UTL_CLASS_DECL(PlanStepRelation);
public:
    /** Constructor. */
    PlanStepRelation(
        ProcessPlan* plan,
        ProcessStep* step)
    { _plan = plan; _step = step; }

    virtual void copy(const utl::Object& rhs);

    virtual void serialize(
        utl::Stream& stream,
        utl::uint_t io,
        utl::uint_t mode = utl::ser_default);

    /// \name Accessors
    //@{
    /** Process plan. */
    const ProcessPlan* plan() const
    { return _plan; }

    /** Process plan. */
    ProcessPlan* plan()
    { return _plan; }

    /** Process step. */
    const ProcessStep* step() const
    { return _step; }

    /** Process step. */
    ProcessStep* step()
    { return _step; }

    /**  Successor step. */
    const ProcessStep* succStep() const
    { return _succStep; }

    /**  Successor step. */
    ProcessStep* succStep()
    { return _succStep; }

    /** Type of relation. */
    cse::precedence_ct_t type() const
    { return _type; }

    /** Type of relation. */
    cse::precedence_ct_t& type()
    { return _type; }

    /** Lag. */
    int lag() const
    { return _lag; }

    /** Lag. */
    int& lag()
    { return _lag; }
    //@}

    /** reset process plan. */
    void setPlan(ProcessPlan* plan, bool owner = false);

    /** reset process step. */
    void setStep(ProcessStep* step, bool owner = false);

    /** reset successor step. */
    void setSuccStep(ProcessStep* succStep, bool owner = false);

    /** create a PrecedenceCt object. */
    cse::PrecedenceCt* createPrecedenceCt();

    utl::String toString() const;
private:
    void init();
    void deInit();
private:
    ProcessPlan* _plan;
    ProcessStep* _step; 
    ProcessStep* _succStep;
    cse::precedence_ct_t _type;
    int _lag;

    bool _planOwner;
    bool _stepOwner;
    bool _succStepOwner;
};

//////////////////////////////////////////////////////////////////////////////

typedef std::vector<PlanStepRelation*> planstep_vector_t;

//////////////////////////////////////////////////////////////////////////////

MRP_NS_END;

//////////////////////////////////////////////////////////////////////////////

#endif
