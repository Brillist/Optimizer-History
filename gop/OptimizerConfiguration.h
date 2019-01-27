#ifndef GOP_OPTIMIZERCONFIGURATION_H
#define GOP_OPTIMIZERCONFIGURATION_H

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

   OptimizerConfiguration is an abstract base for classes that store
   optimizer configuration parameters.

   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class OptimizerConfiguration : public utl::Object
{
    UTL_CLASS_DECL(OptimizerConfiguration, utl::Object);

public:
    typedef std::vector<Objective*> objective_vector_t;
    typedef std::vector<Operator*> op_vector_t;

public:
    /** Initialize to empty. */
    virtual void clear();

    /** Copy another instance. */
    virtual void copy(const utl::Object& rhs);

    virtual void
    serialize(utl::Stream& stream, uint_t io, uint_t mode = utl::ser_default);

    /** Get the minimum number of iterations. */
    uint_t
    minIterations() const
    {
        return _minIterations;
    }

    /** Get the minimum number of iterations. */
    uint_t&
    minIterations()
    {
        return _minIterations;
    }

    /** Get the maximum number of iterations. */
    uint_t
    maxIterations() const
    {
        return _maxIterations;
    }

    /** Get the maximum number of iterations. */
    uint_t&
    maxIterations()
    {
        return _maxIterations;
    }

    /** Get the improvement gap. */
    uint_t
    improvementGap() const
    {
        return _improvementGap;
    }

    /** Get the improvement gap. */
    uint_t&
    improvementGap()
    {
        return _improvementGap;
    }

    /** Get the individual. */
    gop::StringInd<uint_t>*
    ind() const
    {
        return _ind;
    }

    /** Set the individual. */
    void
    setInd(gop::StringInd<uint_t>* ind)
    {
        delete _ind;
        _ind = ind;
    }

    /** Get the ind-builder. */
    IndBuilder*
    indBuilder() const
    {
        return _indBuilder;
    }

    /** Set the ind-builder. */
    void
    setIndBuilder(IndBuilder* indBuilder)
    {
        delete _indBuilder;
        _indBuilder = indBuilder;
    }

    /** Get the ind-builder context. */
    IndBuilderContext*
    context() const
    {
        return _context;
    }

    /** Get the ind-builder context. */
    IndBuilderContext*&
    context()
    {
        return _context;
    }

    /** Get the data-set. */
    const DataSet*
    dataSet() const
    {
        return _context->dataSet();
    }

    /** Get the list of objectives. */
    const objective_vector_t&
    objectives() const
    {
        return _objectives;
    }

    /** Add an objective. */
    void
    add(Objective* objective)
    {
        _objectives.push_back(objective);
    }

    /** Get the list of operators. */
    const op_vector_t&
    getOperators() const
    {
        return _ops;
    }

    /** Get the list of operators. */
    op_vector_t&
    getOperators()
    {
        return _ops;
    }

    /** Add an operator. */
    void
    add(Operator* op)
    {
        _ops.push_back(op);
    }

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

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
