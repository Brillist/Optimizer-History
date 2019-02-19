#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <gop/DataSet.h>
#include <gop/StringInd.h>
#include <gop/IndBuilder.h>
#include <gop/Objective.h>
#include <gop/Operator.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

GOP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Optimizer configuration.

   OptimizerConfiguration stores optimizer configuration parameters.

   \ingroup gop
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class OptimizerConfiguration : public utl::Object
{
    UTL_CLASS_DECL(OptimizerConfiguration, utl::Object);

public:
    typedef std::vector<Objective*> objective_vector_t;
    typedef std::vector<Operator*> op_vector_t;

public:
    virtual void clear();

    virtual void copy(const utl::Object& rhs);

    virtual void serialize(utl::Stream& stream, uint_t io, uint_t mode = utl::ser_default);

    /// \name Accessors (const)
    /** Get the minimum number of iterations. */
    uint_t
    minIterations() const
    {
        return _minIterations;
    }

    /** Get the maximum number of iterations. */
    uint_t
    maxIterations() const
    {
        return _maxIterations;
    }

    /** Get the improvement gap. */
    uint_t
    improvementGap() const
    {
        return _improvementGap;
    }

    /** Get the individual (StringInd<uint_t>). */
    gop::StringInd<uint_t>*
    ind() const
    {
        return _ind;
    }

    /** Get the IndBuilder. */
    IndBuilder*
    indBuilder() const
    {
        return _indBuilder;
    }

    /** Get the IndBuilder context. */
    IndBuilderContext*
    context() const
    {
        return _context;
    }

    /** Get the list of objectives. */
    const objective_vector_t&
    objectives() const
    {
        return _objectives;
    }

    /** Get the data-set. */
    const DataSet*
    dataSet() const
    {
        return _context->dataSet();
    }

    /** Get the list of Operator objects. */
    const op_vector_t&
    operators() const
    {
        return _ops;
    }
    //@}

    /// \name Accessors (non-const)
    //@{
    /** Get the minimum number of iterations. */
    void
    setMinIterations(uint_t minIterations)
    {
        _minIterations = minIterations;
    }

    /** Get the maximum number of iterations. */
    void
    setMaxIterations(uint_t maxIterations)
    {
        _maxIterations = maxIterations;
    }

    /** Get the improvement gap. */
    void
    setImprovementGap(uint_t improvementGap)
    {
        _improvementGap = improvementGap;
    }

    /** Set the individual (StringInd<uint_t>). */
    void
    setInd(gop::StringInd<uint_t>* ind)
    {
        delete _ind;
        _ind = ind;
    }

    /** Set the IndBuilder. */
    void
    setIndBuilder(IndBuilder* indBuilder)
    {
        delete _indBuilder;
        _indBuilder = indBuilder;
    }

    /** Set the IndBuilder context. */
    void
    setContext(IndBuilderContext* context)
    {
        _context = context;
    }

    /** Get the list of Operator objects. */
    op_vector_t&
    operators()
    {
        return _ops;
    }
    //@}

    /// \name Modification
    //@{
    /** Add an objective. */
    void
    add(Objective* objective)
    {
        _objectives.push_back(objective);
    }

    /** Add an operator. */
    void
    add(Operator* op)
    {
        _ops.push_back(op);
    }
    //@}

private:
    void init();
    void deInit();

private:
    uint_t _minIterations;
    uint_t _maxIterations;
    uint_t _improvementGap;
    StringInd<uint_t>* _ind;
    IndBuilder* _indBuilder;
    IndBuilderContext* _context;
    objective_vector_t _objectives;
    op_vector_t _ops;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

GOP_NS_END;
