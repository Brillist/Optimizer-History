#include "libgop.h"
#include "RandomWalk.h"
#include "RevOperator.h"
#include <libutl/BufferedFDstream.h>

//////////////////////////////////////////////////////////////////////////////

#ifdef DEBUG
#define DEBUG_UNIT
#endif

//////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;

//////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(gop::RandomWalk, gop::Optimizer);

//////////////////////////////////////////////////////////////////////////////

GOP_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////

void
RandomWalk::initialize(const OptimizerConfiguration* config)
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
    String<uint_t>* bestStr = _ind->getString()->clone();
    StringScore* strScore = new StringScore(0, bestStr, _bestScore->clone());
    setBestStrScore(strScore);
    objective->setBestScore(_bestScore->clone());

#ifdef DEBUG_UNIT
    utl::cout << initString(!_fail) << utl::endlf;
#endif
}

//////////////////////////////////////////////////////////////////////////////

bool
RandomWalk::run()
{
    ASSERTD(!complete());
    ASSERTD(_ind != nullptr);
    ASSERTD(_bestStrScore != nullptr);

    Objective* objective = _objectives[0];
    bool complete = this->complete();
    int cmpResult;

    while (!complete)
    {
        _iteration++;

        // choose an operator
        Operator* op = chooseRandomStepOp();
        if (op == nullptr)
        {
            _iteration = _maxIterations;
            complete = this->complete();
            break;
        }
        ASSERTD(dynamic_cast<RevOperator*>(op) != nullptr);
        RevOperator* rop = (RevOperator*)op;
#ifdef DEBUG_UNIT
        utl::cout
            << "                                      "
            << op->toString() << utl::endlf;
#endif

        // generate a schedule
        iterationRun(rop);
        cmpResult = objective->compare(_newScore, _bestScore);
        _accept = true;
        _sameScore = (cmpResult == 0);
        _newBest = (cmpResult > 0);

        rop->accept();
        if (_newBest)
        {
            _improvementIteration = _iteration;
            setBestScore(_newScore->clone());
            String<uint_t>* str = _ind->getString()->clone();
            _bestStrScore->setScore(_bestScore->clone());
            _bestStrScore->setString(str);
            objective->setBestScore(_bestScore->clone());
        }

#ifdef DEBUG_UNIT
        utl::cout << iterationString() << utl::endl;
#endif
        // update run status
        complete = this->complete();
        if (!complete)
            updateRunStatus(complete);
    }

    ASSERT(this->complete());
    //re-generate the best schedule and get audit text
    _ind->setString(_bestStrScore->getString()->clone());
    bool scheduleFeasible = iterationRun(nullptr, true);
    ASSERTD(*_bestScore == *_newScore);
    utl::cout << finalString(scheduleFeasible)
              << utl::endlf;
    updateRunStatus(true);
    return scheduleFeasible;
}

//////////////////////////////////////////////////////////////////////////////

void
RandomWalk::audit()
{
    ASSERTD(_bestStrScore != nullptr);
    _ind->setString(_bestStrScore->getString()->clone());
    if (!iterationRun(nullptr, true)) ABORT();
    ASSERTD(*_bestScore == *_newScore);
}

//////////////////////////////////////////////////////////////////////////////

void
RandomWalk::init()
{
    _bestStrScore = nullptr;
}

//////////////////////////////////////////////////////////////////////////////

void
RandomWalk::deInit()
{
    delete _bestStrScore;
}

//////////////////////////////////////////////////////////////////////////////

GOP_NS_END;
