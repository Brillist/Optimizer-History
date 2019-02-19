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

   \ingroup gop
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class OperatorVar : public utl::Object
{
    UTL_CLASS_DECL(OperatorVar, utl::Object);

public:
    /**
       Constructor.

       The initial success rate (\see p) of the operator is (successIter / totalIter).

       \param idx index of this variable (corresponding to a value the Operator can change).
       \param active pointer to active status flag
       \param successIter initial success iterations
       \param totalIter initial total iterations
    */
    OperatorVar(uint_t idx, bool* active, uint_t successIter, uint_t totalIter);

    void copy(const utl::Object& rhs);

    /// \name Accessors (const)
    //@{
    /** Get index. */
    const uint_t
    idx() const
    {
        return _idx;
    }

    /** Get \c active flag. */
    bool
    active() const
    {
        return *_active;
    }

    /** Get success rate. */
    const double
    p() const
    {
        return _p;
    }
    //@}

    /// \name Modification
    //@{
    /** Increase the count of successful iterations (and update the success rate). */
    void
    addSuccessIter()
    {
        _successIter++;
        _p = (double)_successIter / (double)_totalIter;
    }

    /** Increase the count of total iterations (and update the success rate). */
    void
    addTotalIter()
    {
        _totalIter++;
        _p = (double)_successIter / (double)_totalIter;
    }
    //@}

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

/**
   Non-increasing ordering for OperatorVar objects.
   \ingroup gop
*/
struct OperatorVarDecPordering
{
    bool operator()(const OperatorVar* lhs, const OperatorVar* rhs) const;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   A \c std::set of OperatorVar pointers ordered by OperatorVarDecPordering.
   \ingroup gop
*/
using opvar_set_t = std::set<OperatorVar*, OperatorVarDecPordering>;

/**
   A \c std::vector of OperatorVar pointers.
   \ingroup gop
*/
using opvar_vector_t = std::vector<OperatorVar*>;

////////////////////////////////////////////////////////////////////////////////////////////////////

GOP_NS_END;
