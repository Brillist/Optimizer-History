#include "libgop.h"
#include "ID_SAoptimizer.h"
#include "RevOperator.h"
#include <libutl/MemStream.h>
#include <libutl/Float.h>
#include <libutl/BufferedFDstream.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef DEBUG
#define DEBUG_UNIT
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(gop::ID_SAoptimizer);

////////////////////////////////////////////////////////////////////////////////////////////////////

GOP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ID_SAoptimizer::initialize(const OptimizerConfiguration* config)
{
    SAoptimizer::initialize(config);
    _populationSize = 1;
    _fixedInitTemp = false;

    _stopTemp = 0.0001;
    _initTemp = _currentTemp = _stopTemp;
    init();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool
ID_SAoptimizer::run()
{
    ASSERTD(!complete());
    ASSERTD(_ind != nullptr);
    ASSERTD(_targetScore == nullptr);

    Objective* objective = _objectives[0];
    _scoreStep = fabs(_bestScore->getValue() * 0.01);
    _targetScore = utl::clone(_bestScore);
    _targetScore->setValue(_targetScore->getValue() - _scoreStep);

    bool complete = this->complete();
    while (!complete)
    {
#ifdef DEBUG_UNIT
        utl::cout << utl::endl << ID_SAinitTempString() << utl::endl;
#endif
        while (!complete && (objective->compare(_bestScore, _targetScore) < 0))
        {
            _tempDcrRate = pow((_stopTemp / _initTemp), (1.0 / _numProbes));

            // try k times for each initTemp + tempDcrRate + numProbes setting
            for (_repeatId = 0;
                 (_repeatId < _numRepeats) && (objective->compare(_bestScore, _targetScore) < 0);
                 _repeatId++)
            {
                _currentTemp = _initTemp;
                uint_t maxIter = min(_iteration + _numProbes, _maxIterations);

#ifdef DEBUG_UNIT
                utl::cout << utl::endl << ID_SAinitTempString2() << utl::endl;
#endif
                while (!complete && (((_iteration - _improvementIteration) <
                                      (uint_t)(_numProbes * 0.2)) //20% more iters
                                     || ((_iteration < maxIter) &&
                                         (objective->compare(_bestScore, _targetScore) < 0))))
                    complete = SAiterationRun();
            }
            _numProbes = 2 * _numProbes;
        }
        // init run with _initNumProbes or half numProbes of previous run
        _numProbes = max(_initNumProbes, _numProbes / 4);
        ASSERTD(_bestScore->getType() >= _targetScore->getType());
        if (_bestScore->getType() > _targetScore->getType())
        {
            _scoreStep = fabs(_bestScore->getValue() * 0.01);
            _targetScore->setType(_bestScore->getType());
            _targetScore->setValue(_bestScore->getValue() - _scoreStep);
        }
        else
        {
            _targetScore->setValue(_targetScore->getValue() - _scoreStep);
        }
        _acceptanceRatio = _acceptanceRatio * _ratioDcrRate;
        resetInitTemp(_acceptanceRatio, false);
    }

    ASSERT(this->complete());
    // re-generate the best schedule and get audit text
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

utl::String
ID_SAoptimizer::ID_SAinitTempString()
{
    utl::MemStream str;
    str << "targetScore:";
    if (_targetScore)
    {
        str << Float(_targetScore->getValue()).toString(2);
    }
    else
    {
        str << "(no_target_score)";
    }
    str << ", scoreStep:" << Float(_scoreStep).toString(2)
        << ", initTemp:" << Float(_initTemp).toString(2)
        << ", acceptanceRatio:" << Float(_acceptanceRatio).toString(4)
        << ", ratioDcrRate:" << Float(_ratioDcrRate).toString(2)
        << ", initNumProbes:" << _initNumProbes;
    if (_totalScoreDiffIter > 0)
        str << ", avgDiffScore:" << (_totalScoreDiff / _totalScoreDiffIter);
    str << '\0';
    return utl::String((char*)str.get());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

utl::String
ID_SAoptimizer::ID_SAinitTempString2()
{
    utl::MemStream str;
    str << "repeatId:" << _repeatId << ", numProbes:" << _numProbes
        << ", tempDcrRate:" << _tempDcrRate
        << ", targetScore:" << Float(_targetScore->getValue()).toString(2) << '\0';
    return utl::String((char*)str.get());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ID_SAoptimizer::init()
{
    _acceptanceRatio = 0.8;
    _ratioDcrRate = 0.75;
    _targetScore = nullptr;
    _scoreStep = 0;
    _initNumProbes = 100;
    _numProbes = 100;
    _numRepeats = 3;
    _repeatId = 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

GOP_NS_END;
