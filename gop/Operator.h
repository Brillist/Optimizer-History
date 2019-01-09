#ifndef GOP_OPERATOR_H
#define GOP_OPERATOR_H

//////////////////////////////////////////////////////////////////////////////

#include <gop/DataSet.h>
#include <gop/Ind.h>
#include <gop/Objective.h>
#include <gop/OperatorVar.h>

//////////////////////////////////////////////////////////////////////////////

GOP_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////

class Optimizer;

//////////////////////////////////////////////////////////////////////////////

/**
   Operator (abstract).

   Each individual (Ind) carries with it a plan for its own construction.
   An operator makes some change to those instructions, which will result
   in the construction of a different, yet similar individual.

   \author Adam McKee
*/

//////////////////////////////////////////////////////////////////////////////

class Operator : public utl::Object
{
    UTL_CLASS_DECL_ABC(Operator);
public:
	/**
       Constructor.
       \param name operator name
       \param p probability
       \param rng PRNG
    */
    Operator(
        const std::string& name,
        double p,
        utl::RandNumGen* rng = nullptr)
    { init(); _name = name; _p = p; _rng = rng; }

    /** Copy another instance. */
    virtual void copy(const utl::Object& rhs);

    virtual void serialize(
        utl::Stream& stream,
        utl::uint_t io,
        utl::uint_t mode = utl::ser_default);

    virtual utl::String toString() const;

    /// \name Accessors
    //@{
    /** Get the optimizer. */
    Optimizer* getOptimizer() const
    { return _optimizer; }

    /** Set the optimizer. */
    virtual void setOptimizer(Optimizer* optimizer)
    { _optimizer = optimizer; }

    /** Get the RNG. */
    utl::RandNumGen* getRNG() const
    { return _rng; }

    /** Set the RNG. */
    virtual void setRNG(utl::RandNumGen* rng)
    { _rng = rng; }

    /** Get the string base. */
    utl::uint_t getStringBase() const
    { return _stringBase; }

    /** Set the string base. */
    virtual void setStringBase(utl::uint_t stringBase)
    { _stringBase = stringBase; }

    /** Get the name. */
	const std::string& name() const
    { return _name; }

    /** Get the name. */
	std::string& name()
    { return _name; }
    //@}
    
    /** Get the Operator success rate. */
    double p() const;

    /** Get variable success rate. */
    double getSelectedVarP() const;

	/** Get the probability. */
	double getP() const
    { return _p; }

	/** Set the probability. */
	void setP(double p)
    { _p = p; }

    /** Get the number of choices in the operator. */
    utl::int_t getNumChoices() const
    { return _numChoices; }

    /** Set the number of choices in the operator. */
    void setNumChoices (utl::uint_t numChoices)
    { _numChoices = numChoices; }

    /** Get the number of variables in the operator. */
    utl::int_t numVars() const
    { return _numValidVars; }

    /** Get the idx of the selected var. */
    utl::int_t getSelectedVarIdx() const;

    /** Creat a new operator variable. */
    virtual void addOperatorVar(
        utl::uint_t idx,
        utl::uint_t successIter, 
        utl::uint_t totalIter,
        bool* active = &_active);

    /** Select an operator variable for mutation. */
    virtual utl::uint_t selectOperatorVarIdx();

    /** Increase successIter of the selected OperatorVar. */
    void addSuccessIter();

    /** Increase totalIter of the selected OperatorVar. */
    void addTotalIter();

    /** Requires individual to be built? */
    virtual bool needsIndBuild() const
    { return false; }

    /** Initialize state. */
    virtual void initialize(const DataSet* dataSet = nullptr);

    /**
       Execute the operator.
       \param ind individual to be operated on
       \param pop population that individual belongs to
    */
    virtual bool execute(
        Ind* ind = nullptr,
        IndBuilderContext* context = nullptr,
        bool singleStep = false)=0;
//         const Population* pop = nullptr)=0;

    /**
       Execute the operator.
       \param ind individual to be operated on
       \param pop population that individual belongs to
    */
     void operator()(
        Ind* ind = nullptr,
        IndBuilderContext* context = nullptr,
        const Population* pop = nullptr)
    { execute(ind, context, pop); }
protected:
    void setParentScores(
        Ind& child,
        const std::vector<Ind*>& parents);

    utl::RandNumGen* _rng;
    utl::uint_t _stringBase;
private:
    typedef std::set<utl::uint_t> uint_set_t;
private:
    void init();
    void deInit();

    Optimizer* _optimizer;
    std::string _name;
	double _p; //not used for now
    utl::uint_t _successIter;
    utl::uint_t _totalIter;
    opvar_set_t _varSet;
    OperatorVar* _selectedVar;
    utl::uint_t _numChoices;
    utl::uint_t _numValidVars;

    // default value for OpVar' _active
    static  bool _active;
};

//////////////////////////////////////////////////////////////////////////////

typedef std::vector<Operator*> op_vector_t;

//////////////////////////////////////////////////////////////////////////////

GOP_NS_END;

//////////////////////////////////////////////////////////////////////////////

#endif
