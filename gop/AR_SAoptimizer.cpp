#include "libgop.h"
#include "AR_SAoptimizer.h"
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

UTL_CLASS_IMPL(gop::AR_SAoptimizer);

////////////////////////////////////////////////////////////////////////////////////////////////////

GOP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

void
AR_SAoptimizer::initialize(const OptimizerConfiguration* config)
{
    SAoptimizer::initialize(config);
    _populationSize = 1;
    _fixedInitTemp = false;

    _initTemp = _currentTemp = 1000.0;
    _stopTemp = 0.001;
    init();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool
AR_SAoptimizer::run()
{
    ASSERTD(!complete());
    ASSERTD(_ind != nullptr);

    bool complete = this->complete();
    while (!complete)
    {
        // heat up for re-annealing when reaching idleIterGap
        if ((_iteration + 1 - _improvementIteration) == _idleIterGap)
        {
            _idleIterGap = (uint_t)(_idleIterGap * _gapIncRate);
            if (_currentTemp <= _stopTemp)
            {
                _acceptanceRatio = _acceptanceRatio * _ratioDcrRate;
                resetInitTemp(_acceptanceRatio, true);
                _currentTemp = _initTemp;
#ifdef DEBUG_UNIT
                utl::cout << AR_SAinitTempString() << utl::endl;
#endif
            }
        }
        complete = SAiterationRun();
    }

    ASSERT(this->complete());
    //re-generate the best schedule and get audit text
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
AR_SAoptimizer::AR_SAinitTempString()
{
    utl::MemStream str;
    str << "iter:" << _iteration << ", improvIter:" << _improvementIteration
        << ", initTemp:" << Float(_initTemp).toString(2)
        << ", AcceptanceRatio:" << Float(100.0 * _acceptanceRatio).toString(2) << "%"
        << ", idleIterGap:" << _idleIterGap
        << ", totalScoreDiff:" << Float(_totalScoreDiff).toString(2)
        << ", totalScoreDiffiter:" << _totalScoreDiffIter;
    if (_totalScoreDiffIter > 0)
        str << ", avgScoreDiff:" << (_totalScoreDiff / _totalScoreDiffIter);
    str << '\0';
    return utl::String((char*)str.get());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
AR_SAoptimizer::init()
{
    _acceptanceRatio = 0.8;
    _ratioDcrRate = 0.75;
    _idleIterGap = 200;
    _gapIncRate = 1.1;
}
////////////////////////////////////////////////////////////////////////////////////////////////////

GOP_NS_END;
