#include "libgop.h"
#include "SAoptimizer.h"
#include "RevOperator.h"
#include <libutl/MemStream.h>
#include <libutl/Float.h>
#include <libutl/BufferedFDstream.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef DEBUG
#define DEBUG_UNIT
#endif

// #define ID_SA //Iterative Deepening SA
// #define AR_SA //Automatic Re-annealing SA

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(gop::SAoptimizer, gop::Optimizer);

////////////////////////////////////////////////////////////////////////////////////////////////////

GOP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

void
SAoptimizer::initialize(const OptimizerConfiguration* config)
{
    Optimizer::initialize(config);
    ASSERTD(_ind != nullptr);
    _indBuilder->initializeInd(_ind, config->dataSet(), _rng);
    _singleStep = false;
    Objective* objective = _objectives[0];

    iterationRun();
    setInitScore(_newScore->clone());
    setBestScore(_newScore->clone());
    if (_ind->newString())
        _ind->acceptNewString();
    StringScore* strScore = new StringScore(0, _ind->getString()->clone(), _bestScore->clone());
    setBestStrScore(strScore);
    setAcceptedScore(_bestScore->clone());
    objective->setBestScore(_bestScore->clone());

#ifdef DEBUG_UNIT
    utl::cout << initString(!_fail) << utl::endlf;
#endif
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
    if (!_fixedInitTemp)
    {
        if ((_newScore->getType() == _acceptedScore->getType()) & (diff < 0))
        {
            _totalScoreDiff += -diff; //fabs(diff);
            ++_totalScoreDiffIter;
        }
        else if (_newScore->getType() > _acceptedScore->getType())
        {
            _totalScoreDiff = 0;
            _totalScoreDiffIter = 0;
        }
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
    Objective* objective = _objectives[0];
    int cmpResult;
    _tempIteration++;

    double diff;
    diff = objective->scoreDiff(_newScore, _acceptedScore);
    storeScoreDiff(diff);
    if (_scoreDiff >= 0)
    {
        _accept = true; //_accept
    }
    else
    {
        double acceptProb = exp(_scoreDiff / _currentTemp);
        _accept = (_rng->evalf() < acceptProb); //_accept
    }
#ifdef DEBUG_UNIT
    utl::cout << temperatureString() << utl::endl;
#endif
    if (_accept)
    {
        op->accept();
        setAcceptedScore(utl::clone(_newScore));
        cmpResult = objective->compare(_newScore, _bestScore);
        _newBest = (cmpResult > 0);    //_newBest
        _sameScore = (cmpResult == 0); //_sameScore
        if (_newBest)
        {
            op->addSuccessIter();
            _improvementIteration = _iteration;
            setBestScore(utl::clone(_newScore));
            _bestStrScore->setScore(utl::clone(_bestScore));
            _bestStrScore->setString(utl::clone(_ind->getString()));
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

bool
SAoptimizer::SAiterationRun()
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
    utl::cout << "                                      " << op->toString() << utl::endlf;
#endif

    // generate and evaluate a new schedule
    iterationRun(rop);

    // decide whether accept the new schedule
    acceptanceEval(rop);

    // update run status except the last run
    // in order to delay further calls from the client
    complete = this->complete();
    if (!complete)
        updateRunStatus(complete);
    return complete;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool
SAoptimizer::run()
{
    ASSERTD(!complete());
    ASSERTD(_ind != nullptr);

    bool complete = this->complete();
    while (!complete)
        complete = SAiterationRun();

    //re-generate the best schedule and get audit text
    ASSERT(this->complete());
    _ind->setString(_bestStrScore->getString()->clone());
    bool scheduleFeasible = iterationRun(nullptr, true);
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

utl::String
SAoptimizer::temperatureString() const
{
    utl::MemStream str;
    str << "                     "; //temporary code
    str << "temp:" << Float(_currentTemp).toString("precision:3") << "/"
        << Float(_initTemp).toString("precision:0")
        << ", rate:" << Float(_tempDcrRate).toString("precision:2")
        << ", new:" << _newScore->toString() << ", accepted:" << _acceptedScore->toString()
        << ", diff:";
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
        str << Float(_scoreDiff).toString("precision:2");
    }
    double prob = exp(_scoreDiff / _currentTemp);
    str << ", prob:";
    if (prob > 1)
    {
        str << ">100";
    }
    else
    {
        str << Float(100.0 * prob).toString("precision:2");
    }
    str << "%";
    if (_accept)
        str << ", Accepted.";
    str << '\0';
    return utl::String((char*)str.get());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

// bool
// SAoptimizer::run1()
// {
//     ASSERTD(!complete());
//     ASSERTD(_ind != nullptr);

//     Objective* objective = _objectives[0];
//     double qualityStep = fabs(_bestScore->getValue() * 0.01);
//     Score* nextQualityScore = utl::clone(_bestScore);
//     double nodeAcceptPctg = 1.0;
//     double finalTemp = 0.0001;
//     _initTemp = finalTemp;
//     _currentTemp = finalTemp;

//     double totalDiffScore = 0;
//     uint_t internalIter = 0;
//     uint_t numProbes = 50;
//     bool complete = this->complete();
//     int cmpResult;
//     while (!complete)
//     {
//         //init run with 2*50 iterations
//         numProbes = max((uint_t)50, numProbes / 4);
//         nextQualityScore->setValue(nextQualityScore->getValue()  - qualityStep);
//         nodeAcceptPctg = nodeAcceptPctg * 0.75;

//         //calculate init temp
//         if (internalIter > 0)
//             _initTemp = -1 * (totalDiffScore / internalIter) /
//                 log(nodeAcceptPctg);

//         //to reduce the impact of a big score fall
//         totalDiffScore = 0;
//         internalIter = 0;

// #ifdef DEBUG_UNIT
//         utl::cout << utl::endl << "nextQualityScore:"
//                   << nextQualityScore->toString()
//                   << ", qualityStep:" << qualityStep
//                   << ", initTemp:" << _initTemp;
//         if (internalIter != 0)
//             utl::cout << ", avgDiffScore:" << (totalDiffScore / internalIter);
//         utl::cout << utl::endl << utl::endl;
// #endif
//         while (!complete &&
//                (objective->compare(_bestScore, nextQualityScore) < 0))
//         {
//             //calculate numProbes and temperature_deduct_rate
//             numProbes = 2 * numProbes;
//             ////_tempDeducRate = exp((log(tempN/temp0)) / numProbes);
//             _tempDeducRate = pow((finalTemp / _initTemp), (1.0 / numProbes));

//             //try k times for each initTemp+_tempDeductRate+numProbes setting
//             uint_t k = 3;
//             for (uint_t i = 0; (i < k) &&
//                      (objective->compare(_bestScore, nextQualityScore) < 0);
//                  i++)
//             {
//                 _currentTemp = _initTemp;
//                 uint_t maxIter = min(_iteration + numProbes, _maxIterations);

// #ifdef DEBUG_UNIT
//                 utl::cout << "numProbes:" << numProbes << ", i:" << i
//                           << ", _tempDeducRate:" << _tempDeducRate
//                           << utl::endl << utl::endl;
// #endif

//                 //20% more iterations after the bestScore iteration
//                 while (!complete
//                        && (((_iteration < maxIter)
//                             && (objective->compare(_bestScore, nextQualityScore) < 0))
//                            || ((_iteration - _improvementIteration)
//                                < (uint_t)(numProbes * 0.2))))
//                 {
//                     _iteration++;
//                     Operator* op = chooseSuccessOp();
//                     if (op == nullptr)
//                     {
//                         _iteration = _maxIterations;
//                         complete = this->complete();
//                         break;
//                     }
//                     ASSERTD(dynamic_cast<RevOperator*>(op) != nullptr);
//                     RevOperator* rop = (RevOperator*)op;
//                     rop->addTotalIter();

//                     // generate a schedule
//                     iterationRun(rop);
//                     double diff;
//                     diff = objective->scoreDiff(_newScore, _acceptedScore);
//                     if (_newScore->getType() == _acceptedScore->getType())
//                     {
//                         totalDiffScore += fabs(diff);
//                         ++internalIter;
//                     }
//                     else if (_newScore->getType() > _acceptedScore->getType())
//                     {
//                         totalDiffScore = 0;
//                         internalIter = 0;
//                     }
//                     double acceptProb = exp(diff / _currentTemp);
//                     _currentTemp = _currentTemp * _tempDeducRate;
//                     _accept = (_rng->evalf() < acceptProb);//accept
// #ifdef DEBUG_UNIT
//                     utl::cout << temperatureString() << utl::endl;
// #endif
//                     if (_accept)
//                     {
//                         rop->accept();
//                         setAcceptedScore(utl::clone(_newScore));
//                         cmpResult = objective->compare(_newScore, _bestScore);
//                         _newBest = (cmpResult > 0);//_newBest
//                         _sameScore = (cmpResult == 0);//_sameScore
//                         if (_newBest)
//                         {
//                             rop->addSuccessIter();
//                             _improvementIteration = _iteration;
//                             setBestScore(utl::clone(_newScore));
//                             _bestStrScore->setScore(utl::clone(_bestScore));
//                             _bestStrScore->setString(utl::clone(_ind->getString()));
//                             objective->setBestScore(utl::clone(_bestScore));
//                         }
//                     }
//                     else
//                     {
//                         rop->undo();
//                     }
// #ifdef DEBUG_UNIT
//                     utl::cout << iterationString() << utl::endl;
// #endif
//                     complete = this->complete();
//                     if(!complete)
//                         updateRunStatus(complete);
//                 }
//             }
//         }

//     }

//     ASSERT(this->complete());
//     //re-generate the best schedule and get audit text
//     _ind->setString(_bestStrScore->getString()->clone());
//     bool scheduleFeasible = iterationRun(nullptr, true);
//     ASSERTD(*_bestScore == *_newScore);
//     utl::cout << "ID_" << finalString(scheduleFeasible) << utl::endlf;
//     updateRunStatus(true);
//     return scheduleFeasible;
// }

////////////////////////////////////////////////////////////////////////////////////////////////////

// bool
// SAoptimizer::run2()
// {
//     ASSERTD(!complete());
//     ASSERTD(_ind != nullptr);

//     Objective* objective = _objectives[0];
//     double nodeAcceptPctg = 0.8;
//     _initTemp = 0.001;
//     _currentTemp = _initTemp;
// //     double temp0 = 0.001;
// //     double temp = temp0;

//     double totalDiffScore = 0;
//     uint_t internalIter = 0;
//     double newInitIter = 0;
//     uint_t idleIterGap = 200;
//     bool anyImprovement = false;

//     bool complete = this->complete();
//     int cmpResult;

//     while (!complete)
//     {
//         _iteration++;
//         // heat up for re-annealing when reaching idleIterGap
//         if ((_iteration - _improvementIteration) == idleIterGap)
//         {
//             idleIterGap = (uint_t)(idleIterGap * 1.1);
//             if (anyImprovement || _currentTemp <= 0.001)
//             {
//                 // re-set temp0
//                 nodeAcceptPctg = nodeAcceptPctg * 0.75;
//                 if (internalIter > 0)
//                 {
//                     _initTemp = -1 * (totalDiffScore / internalIter) /
//                         log(nodeAcceptPctg);
//                 }

//                 newInitIter = _iteration;
//                 anyImprovement = false;
// #ifdef DEBUG_UNIT
//                 utl::cout << utl::endl
//                           << "_iteration:" << _iteration
//                           << ",ImprovIter:" << _improvementIteration
//                           << ",initTemp:" << _initTemp
//                           << ",nodeAcceptPctg:" << nodeAcceptPctg
//                           << ",idleIterGap:" << idleIterGap
//                           << ",totalDifScore:" << totalDiffScore
//                           << ",iternalIter:" << internalIter;
//                 if (internalIter != 0)
//                     utl::cout << ", avgDiffScore:"
//                               << (totalDiffScore / internalIter);
//                 utl::cout << utl::endl << utl::endl;
// #endif
//             }
//         }

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

//         //generate a schedule
//         iterationRun(rop);
//         double diff;
//         diff = objective->scoreDiff(_newScore, _acceptedScore);
//         if (_newScore->getType() == _acceptedScore->getType())
//         {
//             totalDiffScore += fabs(diff);
//             internalIter++;
//         }
//         else if (_newScore->getType() > _acceptedScore->getType())
//         {
//             totalDiffScore = 0;
//             internalIter = 0;
//         }
//         double acceptProb = exp((diff) / _currentTemp);
//         //double temp = (double)pow(_tempDeducRate, (int)(internalIter /5)) * temp0;
//         _currentTemp = (double)pow(_tempDeducRate,
//                            (int)((_iteration - newInitIter) / 3))
//             * _initTemp;
//         _accept = (_rng->evalf() < acceptProb);//_accept
// #ifdef DEBUG_UNIT
//         utl::cout << temperatureString() << utl::endl;
// #endif
//         if (_accept)
//         {
//             rop->accept();
//             setAcceptedScore(utl::clone(_newScore));
//             cmpResult = objective->compare(_newScore, _bestScore);
//             _newBest = (cmpResult > 0);//_newBest
//             _sameScore = (cmpResult == 0);//_sameScore
//             if (_newBest)
//             {
//                 anyImprovement = true;
//                 rop->addSuccessIter();
//                 _improvementIteration = _iteration;
//                 setBestScore(utl::clone(_newScore));
//                 _bestStrScore->setScore(utl::clone(_bestScore));
//                 _bestStrScore->setString(utl::clone(_ind->getString()));
//                 objective->setBestScore(utl::clone(_bestScore));
//             }
//         }
//         else
//         {
//             rop->undo();
//         }
// #ifdef DEBUG_UNIT
//         utl::cout << iterationString() << utl::endl;
// #endif
//         complete = this->complete();
//         if (!complete)
//             updateRunStatus(complete);
//     }

//     ASSERT(this->complete());
//     //re-generate the best schedule and get audit text
//     _ind->setString(_bestStrScore->getString()->clone());
//     bool scheduleFeasible = iterationRun(nullptr, true);
//     ASSERTD(*_bestScore == *_newScore);
//     utl::cout << "AR_" << finalString(scheduleFeasible) << utl::endlf;
//     updateRunStatus(true);
//     return scheduleFeasible;
// }

////////////////////////////////////////////////////////////////////////////////////////////////////

GOP_NS_END;
