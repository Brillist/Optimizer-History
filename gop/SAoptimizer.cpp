#include "libgop.h"
#include <libutl/BufferedFDstream.h>
#include <libutl/MemStream.h>
#include <libutl/Float.h>
#include "RevOperator.h"
#include "SAoptimizer.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef DEBUG
#define DEBUG_UNIT
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(gop::SAoptimizer);

////////////////////////////////////////////////////////////////////////////////////////////////////

GOP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

void
SAoptimizer::initialize(const OptimizerConfiguration* config)
{
    super::initialize(config);

    // initialize _ind
    ASSERTD(_ind != nullptr);
    _indBuilder->initializeInd(_ind, config->dataSet(), _rng);

    // disable single-step
    _singleStep = false;

    iterationRun();
    setInitScore(_newScore->clone());
    setBestScore(_newScore->clone());
    if (_ind->newString())
        _ind->acceptNewString();
    auto strScore = new StringScore(0, _ind->stringPtr()->clone(), _bestScore->clone());
    setBestStrScore(strScore);
    setAcceptedScore(_bestScore->clone());
    auto objective = _objectives[0];
    objective->setBestScore(_bestScore->clone());

#ifdef DEBUG_UNIT
    utl::cout << initString(!_fail) << utl::endlf;
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool
SAoptimizer::run()
{
    ASSERTD(!complete());
    ASSERTD(_ind != nullptr);

    auto complete = this->complete();
    while (!complete)
        complete = SAiterationRun();

    // re-generate the best schedule and get audit text
    ASSERT(this->complete());
    _ind->setString(_bestStrScore->getString()->clone());
    auto scheduleFeasible = iterationRun(nullptr, true);
#ifdef DEBUG
    if (scheduleFeasible)
        ASSERTD(*_bestScore == *_newScore);
#endif
    utl::cout << finalString(scheduleFeasible) << utl::endlf;
    updateRunStatus(true);
    return scheduleFeasible;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
SAoptimizer::audit()
{
    ASSERTD(_bestStrScore != nullptr);
    _ind->setString(_bestStrScore->getString()->clone());
    if (!iterationRun(nullptr, true))
        ABORT();
    ASSERTD(*_bestScore == *_newScore);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

utl::String
SAoptimizer::temperatureString() const
{
    utl::MemStream str;
    str << "temp:" << Float(_currentTemp).toString(3) << "/" << Float(_initTemp).toString(0)
        << ", rate:" << Float(_tempDcrRate).toString(2) << ", new:" << _newScore->toString()
        << ", accepted:" << _acceptedScore->toString() << ", diff:";
    if (_scoreDiff == utl::double_t_max)
    {
        str << "inf";
    }
    else if (_scoreDiff == -utl::double_t_max)
    {
        str << "-inf";
    }
    else
    {
        str << Float(_scoreDiff).toString(2);
    }
    double prob = exp(_scoreDiff / _currentTemp);
    str << ", prob:";
    if (prob > 1)
    {
        str << ">100";
    }
    else
    {
        str << Float(100.0 * prob).toString(2);
    }
    str << "%";
    if (_accept)
        str << ", Accepted.";
    str << '\0';
    return utl::String((char*)str.get());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool
SAoptimizer::SAiterationRun()
{
    bool complete;
    _iteration++;

    // choose an operator
    auto op = chooseSuccessOp();
    if (op == nullptr)
    {
        _iteration = _maxIterations;
        complete = this->complete();
        return complete;
    }
    auto rop = utl::cast<RevOperator>(op);
    rop->addTotalIter();
#ifdef DEBUG_UNIT
    utl::cout << "  " << op->toString() << utl::endlf;
#endif

    // generate and evaluate a new schedule
    iterationRun(rop);

    // decide whether accept the new schedule
    acceptanceEval(rop);

    // update run status (except for the last run)
    complete = this->complete();
    if (!complete)
        updateRunStatus(complete);
    return complete;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
SAoptimizer::storeScoreDiff(double diff)
{
    ASSERTD(_newScore != nullptr);
    ASSERTD(_acceptedScore != nullptr);
    ASSERTD(_newScore->getType() != score_undefined);
    ASSERTD(_acceptedScore->getType() != score_undefined);
    _scoreDiff = diff;

    // don't record statistics during initialization
    if (_fixedInitTemp)
        return;

    // update _totalScoreDiff, _totalScoreDiffIter
    if ((_newScore->getType() == _acceptedScore->getType()) & (diff < 0))
    {
        _totalScoreDiff += -diff;
        ++_totalScoreDiffIter;
    }
    else if (_newScore->getType() > _acceptedScore->getType())
    {
        _totalScoreDiff = 0;
        _totalScoreDiffIter = 0;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
SAoptimizer::resetInitTemp(double acceptanceRatio, bool reinit)
{
    if (_totalScoreDiffIter == 0)
        return;
    if (acceptanceRatio == 1.0)
    {
        _initTemp = double_t_max;
    }
    else
    {
        _initTemp = -1 * (_totalScoreDiff / _totalScoreDiffIter) / log(acceptanceRatio);
    }
    if (reinit)
    {
        _totalScoreDiff = 0;
        _totalScoreDiffIter = 0;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
SAoptimizer::acceptanceEval(RevOperator* op)
{
    auto objective = _objectives[0];
    int cmpResult;
    _tempIteration++;

    double diff;
    diff = objective->scoreDiff(_newScore, _acceptedScore);
    storeScoreDiff(diff);
    if (_scoreDiff >= 0)
    {
        _accept = true;
    }
    else
    {
        auto acceptProb = exp(_scoreDiff / _currentTemp);
        _accept = (_rng->uniform(0.0, 1.0) < acceptProb);
    }
#ifdef DEBUG_UNIT
    utl::cout << temperatureString() << utl::endl;
#endif
    if (_accept)
    {
        op->accept();
        setAcceptedScore(utl::clone(_newScore));
        cmpResult = objective->compare(_newScore, _bestScore);
        _newBest = (cmpResult > 0);
        _sameScore = (cmpResult == 0);
        if (_newBest)
        {
            op->addSuccessIter();
            _improvementIteration = _iteration;
            setBestScore(utl::clone(_newScore));
            _bestStrScore->setScore(utl::clone(_bestScore));
            _bestStrScore->setString(utl::clone(_ind->stringPtr()));
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
    if (_tempIteration == _populationSize)
    {
        _tempIteration = 0;
        _currentTemp = _currentTemp * _tempDcrRate;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
SAoptimizer::init()
{
    _bestStrScore = nullptr;
    _acceptedScore = nullptr;
    _scoreDiff = 0;
    _tempDcrRate = 0.9;
    _initTemp = 1000.0;
    _currentTemp = 1000.0;
    _stopTemp = 0.0;
    _totalScoreDiff = 0;
    _totalScoreDiffIter = 0;

    _fixedInitTemp = true;
    _populationSize = 100;
    _tempIteration = 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
SAoptimizer::deInit()
{
    delete _bestStrScore;
    delete _acceptedScore;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

GOP_NS_END;
