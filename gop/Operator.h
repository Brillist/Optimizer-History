#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <gop/DataSet.h>
#include <gop/Ind.h>
#include <gop/Objective.h>
#include <gop/OperatorVar.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

GOP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

class Optimizer;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Operator (abstract).

   An Operator alters an \link Ind individual \endlink by altering the instructions used to
   construct it.

   \see Ind
   \see IndBuilder
   \ingroup gop
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class Operator : public utl::Object
{
    UTL_CLASS_DECL_ABC(Operator, utl::Object);

public:
    /** Copy another instance. */
    virtual void copy(const utl::Object& rhs);

    virtual void serialize(utl::Stream& stream, uint_t io, uint_t mode = utl::ser_default);

    virtual utl::String toString() const;

    /// \name Accessors (const)
    //@{
    /** Get the optimizer. */
    Optimizer*
    optimizer() const
    {
        return _optimizer;
    }

    /** Get the name. */
    std::string
    name() const
    {
        return this->getClassName();
    }

    /** Get the RNG. */
    lut::rng_t*
    rng() const
    {
        return _rng;
    }

    /** Get the string base. */
    uint_t
    stringBase() const
    {
        return _stringBase;
    }

    /** Get the number of choices in the operator. */
    uint_t
    numChoices() const
    {
        return _numChoices;
    }

    /** Get the number of variables in the operator. */
    uint_t
    numVars() const
    {
        return _numValidVars;
    }

    /** Get the success rate. */
    double p() const;

    /** Get the selected \link OperatorVar variable's \endlink index. */
    uint_t varIdx() const;

    /** Get the selected \link OperatorVar variable's \endlink success rate. */
    double varP() const;
    //@}

    /// \name Accessors (non-const)
    //@{
    /** Set the optimizer. */
    virtual void
    setOptimizer(Optimizer* optimizer)
    {
        _optimizer = optimizer;
    }

    /** Set the RNG. */
    virtual void
    setRNG(lut::rng_t* rng)
    {
        _rng = rng;
    }

    /** Set the string base. */
    virtual void
    setStringBase(uint_t stringBase)
    {
        _stringBase = stringBase;
    }

    /** Set the number of choices in the operator. */
    void
    setNumChoices(uint_t numChoices)
    {
        _numChoices = numChoices;
    }
    //@}

    /// \name Modification
    //@{
    /** Create a new operator variable. */
    virtual void
    addOperatorVar(uint_t idx, uint_t successIter, uint_t totalIter, bool* active = &_active);

    /** Select an operator variable for mutation. */
    virtual uint_t selectVar();

    /** Increase successIter of the selected OperatorVar. */
    void addSuccessIter();

    /** Increase totalIter of the selected OperatorVar. */
    void addTotalIter();
    //@}

    /** Initialize state. */
    virtual void initialize(const DataSet* dataSet = nullptr);

    /**
       Execute the operator.
       \param ind individual to be operated on
       \param context IndBuilder context
       \param singleStep single step?
    */
    virtual bool
    execute(Ind* ind = nullptr, IndBuilderContext* context = nullptr, bool singleStep = false) = 0;

    /**
       Execute the operator (as if calling a function).
       \param ind individual to be operated on
       \param context IndBuilder context
       \param pop population that individual belongs to
    */
    void
    operator()(Ind* ind = nullptr,
               IndBuilderContext* context = nullptr,
               const Population* pop = nullptr)
    {
        execute(ind, context, pop);
    }

protected:
    lut::rng_t* _rng;
    uint_t _stringBase;

private:
    void init();
    void deInit();

private:
    Optimizer* _optimizer;
    uint_t _successIter;
    uint_t _totalIter;
    opvar_set_t _varSet;
    OperatorVar* _selectedVar;
    uint_t _numChoices;
    uint_t _numValidVars;

    // default value for OpVar' _active
    static bool _active;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   A \c std::vector of Operator pointers.
   \ingroup gop
*/
using op_vector_t = std::vector<Operator*>;

////////////////////////////////////////////////////////////////////////////////////////////////////

GOP_NS_END;
