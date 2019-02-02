#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <gop/DataSet.h>
#include <gop/Ind.h>
#include <gop/Objective.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

GOP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Operator Variable.

   \author Joe Zhou
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class OperatorVar : public utl::Object
{
    UTL_CLASS_DECL(OperatorVar, utl::Object);

public:
    /** Constructor. */
    OperatorVar(uint_t idx, bool* active, uint_t successIter, uint_t totalIter);

    /** Copy. */
    void copy(const utl::Object& rhs);

    /// \name Accessors
    //@{
    /** Get _idx. */
    const uint_t
    idx() const
    {
        return _idx;
    }

    /** Get active value. */
    bool
    active()
    {
        return *_active;
    }

    /** success probability. */
    const double
    p() const
    {
        return _p;
    }
    //@}

    /** Increase _successIter. */
    void
    addSuccessIter()
    {
        _successIter++;
        _p = (double)_successIter / (double)_totalIter;
    }

    /** Increase _totalIter. */
    void
    addTotalIter()
    {
        _totalIter++;
        _p = (double)_successIter / (double)_totalIter;
    }

private:
    void
    init()
    {
        ABORT();
    }

    void
    deInit()
    {
    }

private:
    uint_t _idx;
    bool* _active;
    uint_t _successIter;
    uint_t _totalIter;
    double _p;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

struct OperatorVarDecPordering
{
    bool operator()(const OperatorVar* lhs, const OperatorVar* rhs) const;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

typedef std::set<OperatorVar*, OperatorVarDecPordering> opvar_set_t;

////////////////////////////////////////////////////////////////////////////////////////////////////

GOP_NS_END;
