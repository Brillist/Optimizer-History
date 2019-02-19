#include "libgop.h"
#include <libutl/Float.h>
#include <libutl/Bool.h>
#include <libutl/BufferedFDstream.h>
#include <libutl/MemStream.h>
#include <gop/Optimizer.h>
#include "Operator.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef DEBUG
// #define DEBUG_UNIT
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL_ABC(gop::Operator);

////////////////////////////////////////////////////////////////////////////////////////////////////

GOP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////
//// Operator //////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

bool Operator::_active = true;

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Operator::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(Operator));
    const Operator& op = (const Operator&)rhs;
    _rng = nullptr;
    _stringBase = 0;
    _optimizer = nullptr;
    _successIter = op._successIter;
    _totalIter = op._totalIter;
    _numChoices = op._numChoices;
    _varSet = op._varSet;
    _selectedVar = op._selectedVar;
    _active = op._active;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Operator::serialize(Stream& stream, uint_t io, uint_t)
{
    // TODO: remove these (front-end change required)
    std::string dummyName;
    double dummyP = 0.0;
    lut::serialize(dummyName, stream, io);
    utl::serialize(dummyP, stream, io);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

utl::String
Operator::toString() const
{
    utl::MemStream str;
    str << name().c_str() << ": numChoices:" << numChoices() << ", numVars:" << numVars()
        << ", sucRate:" << Float(p()).toString(2) << ", successIter:" << _successIter
        << ", totalIter:" << _totalIter << '\0';
    return utl::String((char*)str.get());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

double
Operator::p() const
{
    // pick up the first var from the sorted var list,
    // return its p() as the op's p().
    for (auto var : _varSet)
    {
        if (var->active())
            return var->p();
    }
    return 0.0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

uint_t
Operator::varIdx() const
{
    ASSERTD(_selectedVar != nullptr);
    return _selectedVar->idx();
}
////////////////////////////////////////////////////////////////////////////////////////////////////

double
Operator::varP() const
{
    ASSERTD(_selectedVar != nullptr);
    return _selectedVar->p();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Operator::addOperatorVar(uint_t idx, uint_t initSuccessIter, uint_t initTotalIter, bool* active)
{
    // create a new OperatorVar
    auto var = new OperatorVar(idx, active, initSuccessIter, initTotalIter);
    _varSet.insert(var);

    // update count of valid variables (those with non-zero success rate)
    if (var->p() > 0)
        _numValidVars++;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

uint_t
Operator::selectVar()
{
    ASSERTD(_varSet.size() > 0);

#ifdef DEBUG_UNIT
    utl::cout << "SelectVar  OpVarSize:" << (uint_t)_varSet.size();
    for (it = _varSet.begin(); it != _varSet.end(); it++)
    {
        OperatorVar* var = *it;
        utl::cout << " (" << var->idx() << ", active:" << (Bool)var->active()
            << ", p:" << Float(var->p()).toString(2) << ")";
    }
    utl::cout << utl::endlf;
#endif

    // vars = variables that are tied for highest success rate
    opvar_vector_t vars;
    double successRate = 0;
    for (auto var : _varSet)
    {
        if (!var->active())
            continue;
        if (successRate == 0)
            successRate = var->p();
        if (var->p() < successRate)
            break;
        vars.push_back(var);
    }

    // choose a var from vars
    ASSERTD(vars.size() > 0);
    auto idx = _rng->uniform((size_t)0, vars.size() - 1);
    _selectedVar = vars[idx];

    // return the index of the selected variable
    return _selectedVar->idx();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Operator::addSuccessIter()
{
    _successIter++;
    ASSERTD(_selectedVar != nullptr);
    _varSet.erase(_selectedVar);
    _selectedVar->addSuccessIter();
    _varSet.insert(_selectedVar);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Operator::addTotalIter()
{
    _totalIter++;
    ASSERTD(_selectedVar != nullptr);
    _varSet.erase(_selectedVar);
    _selectedVar->addTotalIter();
    _varSet.insert(_selectedVar);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Operator::initialize(const DataSet*)
{
    ASSERTD(_rng != nullptr);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Operator::init()
{
    _rng = nullptr;
    _stringBase = 0;
    _optimizer = nullptr;
    _successIter = 1;
    _totalIter = 2;
    _selectedVar = nullptr;
    _numChoices = 0;
    _numValidVars = 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Operator::deInit()
{
    deleteCont(_varSet);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

GOP_NS_END;
