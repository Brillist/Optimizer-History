#include "libgop.h"
#include <libutl/BufferedFDstream.h>
#include <libutl/Float.h>
#include "RevOperator.h"
#include "HillClimber.h"

//////////////////////////////////////////////////////////////////////////////

#ifdef DEBUG
#define DEBUG_UNIT
#endif

//////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;

//////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(gop::HillClimber, gop::Optimizer);

//////////////////////////////////////////////////////////////////////////////

GOP_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////
/// HillClimber //////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

void
HillClimber::initialize(const OptimizerConfiguration* config)
{
    Optimizer::initialize(config);
    ASSERTD(_ind != nullptr);
    _indBuilder->initializeInd(_ind, config->dataSet(), _rng);
    _singleStep = true;
    Objective* objective = _objectives[0];

    iterationRun();
    setInitScore(_newScore->clone());
    setBestScore(_newScore->clone());
    if (_ind->newString()) _ind->acceptNewString();
    objective->setBestScore(_bestScore->clone());

#ifdef DEBUG_UNIT
    utl::cout << initString(!_fail) << utl::endlf;
#endif
}

//////////////////////////////////////////////////////////////////////////////

void
HillClimber::acceptanceEval(RevOperator* op)
{
    Objective* objective = _objectives[0];
    int cmpResult;
    cmpResult = objective->compare(_newScore, _bestScore);
    _accept = (cmpResult >= 0);//_accept
    _sameScore = (cmpResult == 0);//_sameScore
    _newBest = (cmpResult > 0);//_newScore
    if (_accept)
    {
        op->accept();
        if (_newBest)
        {
            _improvementIteration = _iteration;
            op->addSuccessIter();
            setBestScore(utl::clone(_newScore));
            objective->setBestScore(utl::clone(_bestScore));
        }
    }
    else
    {
        op->undo();
    }
#ifdef DEBUG_UNIT
    utl::cout << iterationString() << utl::endl;
#endif
}

//////////////////////////////////////////////////////////////////////////////

bool
HillClimber::HCiterationRun()
{
    bool complete;
    _iteration++;

    // choose an operator
    Operator* op = chooseSuccessOp();
    if (op == nullptr)
    {
        _iteration = _maxIterations;
        complete = this->complete();
        return complete;
    }
    ASSERTD(dynamic_cast<RevOperator*>(op) != nullptr);
    RevOperator* rop = (RevOperator*)op;
    rop->addTotalIter();
#ifdef DEBUG_UNIT
    utl::cout
        << "                                      "
        << op->toString() << utl::endl;
#endif

    // generate a schedule
    iterationRun(rop);

    //decide whether accept the new schedule
    acceptanceEval(rop);

    // update run status
    complete = this->complete();
    if (!complete)
        updateRunStatus(complete);
    return complete;
}

//////////////////////////////////////////////////////////////////////////////

bool
HillClimber::run()
{
    ASSERTD(!complete());
    ASSERTD(_ind != nullptr);

//     Objective* objective = _objectives[0];
    bool complete = this->complete();
//     int cmpResult;

    while (!complete)
        complete = HCiterationRun();
//     {
//         _iteration++;

//         // choose an operator
//         Operator* op = chooseSuccessOp();
//         if (op == nullptr)
//         {
//             _iteration = _maxIterations;
//             complete = this->complete();
//             break;
//         }
//         ASSERTD(dynamic_cast<RevOperator*>(op) != nullptr);
//         RevOperator* rop = (RevOperator*)op;
//         rop->addTotalIter();
// #ifdef DEBUG_UNIT
//         utl::cout
//             << "                                      "
//             << "operator: " << op->toString() << utl::endl;
// #endif

//         // generate a schedule
//         iterationRun(rop);
//         cmpResult = objective->compare(_newScore, _bestScore);
//         _accept = (cmpResult >= 0);//_accept
//         _sameScore = (cmpResult == 0);//_sameScore
//         _newBest = (cmpResult > 0);//_newScore
//         if (_accept)
//         {
//             rop->accept();
//             if (_newBest)
//             {
//                 _improvementIteration = _iteration;
//                 rop->addSuccessIter();
//                 setBestScore(_newScore->clone());
//                 objective->setBestScore(_bestScore->clone());
//             }
//         }
//         else
//         {
//             rop->undo();
//         }
// #ifdef DEBUG_UNIT
//         utl::cout << iterationString() << utl::endl;
// #endif
//         // update run status
//         complete = this->complete();
//         if (!complete)
//             updateRunStatus(complete);
//     }

    ASSERT(this->complete());
    //re-generate the best schedule and get audit text
    bool scheduleFeasible = iterationRun(nullptr, true);
#ifdef DEBUG
    if (scheduleFeasible)
        ASSERTD(*_bestScore == *_newScore);
#endif
    utl::cout << finalString(scheduleFeasible) << utl::endlf;
    updateRunStatus(true);
    return scheduleFeasible;
}

//////////////////////////////////////////////////////////////////////////////

void
HillClimber::audit()
{
    if (!iterationRun(nullptr, true)) ABORT();
    ASSERTD(*_bestScore == *_newScore);
}

//////////////////////////////////////////////////////////////////////////////

GOP_NS_END;
