#include "libgop.h"
#include "MultistartHC.h"
#include "RevOperator.h"
#include <libutl/Bool.h>
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

UTL_CLASS_IMPL(gop::MultistartHC);

////////////////////////////////////////////////////////////////////////////////////////////////////

GOP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

void
MultistartHC::initialize(const OptimizerConfiguration* config)
{
    Optimizer::initialize(config);
    ASSERTD(_ind != nullptr);
    _indBuilder->initializeInd(_ind, config->dataSet(), _rng);
    _singleStep = true;
    auto objective = _objectives[0];

    iterationRun();
    setInitScore(_newScore->clone());
    setBestScore(_newScore->clone());
    if (_ind->newString())
        _ind->acceptNewString();
    objective->setBestScore(_bestScore->clone());

    _beamWidth = 10;
    auto str = _ind->stringPtr();
    for (uint_t i = 0; i < _beamWidth; i++)
    {
        auto strScore = new StringScore(i, str->clone(), _bestScore->clone());
        _strScores.push_back(strScore);
    }
    _ind->setString(nullptr, false);

#ifdef DEBUG_UNIT
    utl::cout << initString(!_fail) << utl::endlf;
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool
MultistartHC::run()
{
    ASSERTD(!complete());
    ASSERTD(_ind != nullptr);

    auto objective = _objectives[0];
    Bool complete = this->complete();
    int cmpResult;

    while (!complete)
    {
        // reduce the length of the _strScores list
        if (_iteration == roundUp(_minIterations / 20, _beamWidth) || //   5%
            _iteration == roundUp(_minIterations / 4, _beamWidth) ||  //  25%
            _iteration == roundUp(_minIterations, _beamWidth))        // 100%
        {
            std::sort(_strScores.begin(), _strScores.end(), StringScoreOrdering());
            _beamWidth = utl::max((uint_t)1, (_beamWidth / 2));
            stringscore_vector_t::iterator it;
            for (it = _strScores.begin() + _beamWidth; it != _strScores.end(); ++it)
            {
                auto strScore = *it;
                delete strScore;
            }
            _strScores.erase(_strScores.begin() + _beamWidth, _strScores.end());
        }

        for (uint_t i = 0; i < _beamWidth; i++)
        {
            _iteration++;

            //choose an operator
            Operator* op = chooseRandomOp();
            if (op == nullptr)
            {
                _iteration = _maxIterations;
                complete = this->complete();
                break;
            }
            auto rop = utl::cast<RevOperator>(op);
            rop->addTotalIter();
            _ind->setString(_strScores[i]->getString(), false); //
#ifdef DEBUG_UNIT
            utl::cout << "                                " << op->toString() << utl::endl;
#endif

            // generate a schedule
            iterationRun(rop);
            cmpResult = objective->compare(_newScore, _strScores[i]->getScore());
            _accept = (cmpResult >= 0); //_accept
            _sameScore = _newBest = false;
            if (_accept)
            {
                rop->accept();
                if (cmpResult > 0)
                {
                    _strScores[i]->setScore(utl::clone(_newScore));
                    int globalCmpResult = objective->compare(_newScore, _bestScore);
                    _sameScore = (globalCmpResult == 0);
                    _newBest = (globalCmpResult > 0);
                    if (_newBest)
                    {
                        _improvementIteration = _iteration;
                        rop->addSuccessIter();
                        setBestScore(utl::clone(_newScore));
                        objective->setBestScore(utl::clone(_bestScore));
                    }
                }
            }
            else
            {
                rop->undo();
            }
#ifdef DEBUG_UNIT
            utl::cout << "startId:" << _strScores[i]->getId() << "(" << i << "/" << _beamWidth
                      << ", " << Float(_strScores[i]->getScore()->getValue()).toString(0) << "), "
                      << iterationString() << utl::endl;
#endif

            // update run status
            complete = this->complete();
            if (!complete)
            {
                updateRunStatus(complete);
            }
            else
            {
                break;
            }
        }
    }
    ASSERTD(this->complete());

    // re-generate the best schedule and get audit text
    std::sort(_strScores.begin(), _strScores.end(), StringScoreOrdering());
    _ind->setString(_strScores[0]->getString(), false);
    bool scheduleFeasible = iterationRun(nullptr, true);
#ifdef DEBUG
    if (scheduleFeasible)
        ASSERT(*_bestScore == *_newScore);
#endif
    utl::cout << finalString(scheduleFeasible) << utl::endlf;
    updateRunStatus(true);
    return scheduleFeasible;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
MultistartHC::audit()
{
    _ind->setString(_strScores[0]->getString(), false);
    if (!iterationRun(nullptr, true))
        ABORT();
    ASSERTD(*_bestScore == *_newScore);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
MultistartHC::init()
{
    _beamWidth = 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
MultistartHC::deInit()
{
    deleteCont(_strScores);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

GOP_NS_END;
