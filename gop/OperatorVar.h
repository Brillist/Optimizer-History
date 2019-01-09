#ifndef GOP_OPERATORVAR_H
#define GOP_OPERATORVAR_H

//////////////////////////////////////////////////////////////////////////////

#include <gop/DataSet.h>
#include <gop/Ind.h>
#include <gop/Objective.h>

//////////////////////////////////////////////////////////////////////////////

GOP_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////

/**
   Operator Variable.

   \author Joe Zhou
*/

//////////////////////////////////////////////////////////////////////////////

class OperatorVar : public utl::Object
{
    UTL_CLASS_DECL(OperatorVar);
public:
    /** Constructor. */    
    OperatorVar(
        utl::uint_t idx,
        bool* active,
        utl::uint_t successIter,
        utl::uint_t totalIter);

    /** Copy. */
    void copy(const utl::Object& rhs);

    /// \name Accessors
    //@{
    /** Get _idx. */
    const utl::uint_t idx() const
    { return _idx; }

    /** Get active value. */
    bool active()
    {return *_active; }

    /** success probability. */
    const double p() const
    { return _p; }
    //@}

    /** Increase _successIter. */
    void addSuccessIter()
    { _successIter++; _p = (double)_successIter / (double)_totalIter; }

    /** Increase _totalIter. */
    void addTotalIter()
    { _totalIter++; _p = (double)_successIter / (double)_totalIter; }

private:
    void init()
    { ABORT(); }

    void deInit() {}
private:
    utl::uint_t _idx;
    bool* _active;
    utl::uint_t _successIter;
    utl::uint_t _totalIter;
    double _p;
};

//////////////////////////////////////////////////////////////////////////////

struct OperatorVarDecPordering 
    : public std::binary_function<OperatorVar*,OperatorVar*,bool>
{
    bool operator()(const OperatorVar* lhs, const OperatorVar* rhs) const;
};

//////////////////////////////////////////////////////////////////////////////

typedef std::set<OperatorVar*,OperatorVarDecPordering> opvar_set_t;

//////////////////////////////////////////////////////////////////////////////

GOP_NS_END;

//////////////////////////////////////////////////////////////////////////////

#endif
