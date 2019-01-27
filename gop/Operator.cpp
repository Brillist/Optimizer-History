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
    _name = op._name;
    _p = op._p;
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
    lut::serialize(_name, stream, io);
    utl::serialize(_p, stream, io);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

utl::String
Operator::toString() const
{
    utl::MemStream str;
    str << name().c_str() << ": numChoices:" << getNumChoices() << ", numValidVars:" << numVars()
        << ", sucRate:" << Float(p()).toString(2) << ", successIter:" << _successIter
        << ", totalIter:" << _totalIter << '\0';
    return utl::String((char*)str.get());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Operator::initialize(const DataSet*)
{
    ASSERTD(_rng != nullptr);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

double
Operator::p() const
{
    // pick up the first var from the sorted var list,
    // return its p() as the op's p().
    if (_varSet.size() == 0)
    {
        return 0.0;
    }
    else
    {
        opvar_set_t::const_iterator it;
        for (it = _varSet.begin(); it != _varSet.end(); it++)
        {
            OperatorVar* var = (*it);
            if (var->active())
                return var->p();
        }
        return 0.0;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

double
Operator::getSelectedVarP() const
{
    ASSERTD(_selectedVar != nullptr);
    return _selectedVar->p();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int_t
Operator::getSelectedVarIdx() const
{
    ASSERTD(_selectedVar != nullptr);
    return _selectedVar->idx();
}
////////////////////////////////////////////////////////////////////////////////////////////////////

void
Operator::addOperatorVar(uint_t idx,
                         uint_t initSuccessIter,
                         uint_t initTotalIter,
                         bool* active)
{
    //Note: this method is used for initialization.
    //      currently there are two ways for initialization
    //      addOperatorVar(i,0,2)  p = 0
    //      addOperatorVar(i,1,2)  p = 0.5
    OperatorVar* var = new OperatorVar(idx, active, initSuccessIter, initTotalIter);
    _varSet.insert(var);
    if (var->p() > 0)
        _numValidVars++;
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
Operator::addSuccessIter()
{
    _successIter++;
    ASSERTD(_selectedVar != nullptr);
    _varSet.erase(_selectedVar);
    _selectedVar->addSuccessIter();
    _varSet.insert(_selectedVar);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

uint_t
Operator::selectOperatorVarIdx()
{
    ASSERTD(_varSet.size() > 0);
    std::vector<OperatorVar*> vars;
    double successRate = 0; //(*_varSet.begin())->p();
    opvar_set_t::const_iterator it;

#ifdef DEBUG_UNIT
    utl::cout << "SelectOperatorVarIdx  OpVarSize:" << (uint_t)_varSet.size();
    for (it = _varSet.begin(); it != _varSet.end(); it++)
    {
        OperatorVar* var = *it;
        utl::cout << " (" << var->idx() << ", active:" << (Bool)var->active()
                  << ", p:" << Float(var->p()).toString(2) << ")";
    }
    utl::cout << utl::endlf;
#endif

    for (it = _varSet.begin(); it != _varSet.end(); ++it)
    {
        OperatorVar* var = *it;
        if (!var->active())
            continue; //deals with active
        if (successRate == 0)
            successRate = var->p();
        if (var->p() < successRate)
            break;
        vars.push_back(var);
    }
    ASSERTD(vars.size() > 0);
    uint_t idx = _rng->uniform((size_t)0, vars.size() - 1);
    _selectedVar = vars[idx];

    return _selectedVar->idx();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Operator::init()
{
    _rng = nullptr;
    _stringBase = 0;
    _optimizer = nullptr;
    _p = 0.0;
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
