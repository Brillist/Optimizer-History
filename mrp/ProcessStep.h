#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <libutl/Array.h>
#include <cse/JobOp.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

MRP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Process step.

   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class ProcessStep : public utl::Object
{
    UTL_CLASS_DECL(ProcessStep, utl::Object);

public:
    virtual void copy(const utl::Object& rhs);

    virtual void serialize(utl::Stream& stream, uint_t io, uint_t mode = utl::ser_default);

    /// \name Accessors
    //@{
    /** Id. */
    uint_t
    id() const
    {
        return _id;
    }

    /** Id. */
    uint_t&
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

    /** Processing time. */
    uint_t
    processingTime() const
    {
        return _processingTime;
    }

    /** Processing time. */
    uint_t&
    processingTime()
    {
        return _processingTime;
    }

    /** PtPer. */
    ps_pt_per_t
    ptPer() const
    {
        return _ptPer;
    }

    /** PtBatchSize. */
    uint_t
    ptBatchSize() const
    {
        return _ptBatchSize;
    }

    /** FixedCost. */
    double
    fixedCost() const
    {
        return _fixedCost;
    }

    /** FixedCostPer. */
    ps_fc_per_t
    fcPer() const
    {
        return _fcPer;
    }

    /** FixedCostBatchSize. */
    uint_t
    fcBatchSize() const
    {
        return _fcBatchSize;
    }

    /** List of required items. */
    const utl::Array&
    itemRequirements() const
    {
        return _itemReqs;
    }

    /** List of required items. */
    utl::Array&
    itemRequirements()
    {
        return _itemReqs;
    }

    /** List of required resources. */
    const utl::Array&
    resourceRequirements() const
    {
        return _resReqs;
    }

    /** List of required resources. */
    utl::Array&
    resourceRequirements()
    {
        return _resReqs;
    }

    /** Resource cap/pt pairs. */
    const utl::Array&
    resourceCapPts() const
    {
        return _resCapPts;
    }

    /** Resource cap/pt pairs. */
    utl::Array&
    resourceCapPts()
    {
        return _resCapPts;
    }
    //@}

    /** create a JobOp object. */
    cse::JobOp* createJobOp(uint_t id, uint_t quantity);

    String toString() const;

private:
    void
    init()
    {
        _id = _processingTime = uint_t_max;
    }
    void
    deInit()
    {
    }

private:
    uint_t _id;
    std::string _name;
    uint_t _processingTime;
    ps_pt_per_t _ptPer;
    uint_t _ptBatchSize;
    double _fixedCost;
    ps_fc_per_t _fcPer;
    uint_t _fcBatchSize;

    utl::Array _itemReqs;
    utl::Array _resReqs;
    utl::Array _resCapPts;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

struct StepOrderingIncId : public std::binary_function<ProcessStep*, ProcessStep*, bool>
{
    bool operator()(const ProcessStep* lhs, const ProcessStep* rhs) const;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

typedef std::set<ProcessStep*, StepOrderingIncId> step_set_id_t;
typedef std::vector<ProcessStep*> step_vector_t;

////////////////////////////////////////////////////////////////////////////////////////////////////

MRP_NS_END;
