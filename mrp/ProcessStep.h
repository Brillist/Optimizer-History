#ifndef MRP_PROCESSSTEP_H
#define MRP_PROCESSSTEP_H

//////////////////////////////////////////////////////////////////////////////

#include <libutl/Array.h>
#include <cse/JobOp.h>

//////////////////////////////////////////////////////////////////////////////

MRP_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////

enum ps_pt_per_t
{
    pt_per_piece = 0,
    pt_per_batch = 1,
    pt_per_undefined = 2
};

enum ps_fc_per_t
{
    fc_per_piece = 0,
    fc_per_batch = 1,
    fc_per_undefined = 2
};

//////////////////////////////////////////////////////////////////////////////

/**
   Process step.

   \author Adam McKee
*/

//////////////////////////////////////////////////////////////////////////////

class ProcessStep : public utl::Object
{
    UTL_CLASS_DECL(ProcessStep);
public:
    virtual void copy(const utl::Object& rhs);

    virtual void serialize(
        utl::Stream& stream,
        utl::uint_t io,
        utl::uint_t mode = utl::ser_default);

    /// \name Accessors
    //@{
    /** Id. */
    utl::uint_t id() const
    { return _id; }

    /** Id. */
    utl::uint_t& id()
    { return _id; }

    /** Name. */
    const std::string& name() const
    { return _name; }

    /** Name. */
    std::string& name()
    { return _name; }

    /** Processing time. */
    utl::uint_t processingTime() const
    { return _processingTime; }

    /** Processing time. */
    utl::uint_t& processingTime()
    { return _processingTime; }

    /** PtPer. */
    ps_pt_per_t ptPer() const
    {
        return _ptPer;
    }

    /** PtBatchSize. */
    utl::uint_t ptBatchSize() const
    { return _ptBatchSize; }

    /** FixedCost. */
    double fixedCost() const
    { return _fixedCost; }

    /** FixedCostPer. */
    ps_fc_per_t fcPer() const
    { return _fcPer; }

    /** FixedCostBatchSize. */
    utl::uint_t fcBatchSize() const
    { return _fcBatchSize; }

    /** List of required items. */
    const utl::Array& itemRequirements() const
    { return _itemReqs; }

    /** List of required items. */
    utl::Array& itemRequirements()
    { return _itemReqs; }

    /** List of required resources. */
    const utl::Array& resourceRequirements() const
    { return _resReqs; }

    /** List of required resources. */
    utl::Array& resourceRequirements()
    { return _resReqs; }

    /** Resource cap/pt pairs. */
    const utl::Array& resourceCapPts() const
    { return _resCapPts; }

    /** Resource cap/pt pairs. */
    utl::Array& resourceCapPts()
    { return _resCapPts; }
    //@}

    /** create a JobOp object. */
    cse::JobOp* createJobOp(utl::uint_t id, utl::uint_t quantity);

    utl::String toString() const;
private:
    void init()
    { _id = _processingTime = utl::uint_t_max; }
    void deInit() {}
private:
    utl::uint_t _id;
    std::string _name;
    utl::uint_t _processingTime;
    ps_pt_per_t _ptPer;
    utl::uint_t _ptBatchSize;
    double _fixedCost;
    ps_fc_per_t _fcPer;
    utl::uint_t _fcBatchSize; 

    utl::Array _itemReqs;
    utl::Array _resReqs;
    utl::Array _resCapPts;
};

//////////////////////////////////////////////////////////////////////////////

struct StepOrderingIncId :
    public std::binary_function<ProcessStep*, ProcessStep*, bool>
{
    bool operator()(const ProcessStep* lhs, const ProcessStep* rhs) const;
};

//////////////////////////////////////////////////////////////////////////////

typedef std::set<ProcessStep*, StepOrderingIncId> step_set_id_t;
typedef std::vector<ProcessStep*> step_vector_t;

//////////////////////////////////////////////////////////////////////////////

MRP_NS_END;

//////////////////////////////////////////////////////////////////////////////

#endif
