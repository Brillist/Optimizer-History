#include "libgop.h"
#include "MultistartSA.h"
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

UTL_CLASS_IMPL(gop::MultistartSA);

////////////////////////////////////////////////////////////////////////////////////////////////////

GOP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

void
MultistartSA::initialize(const OptimizerConfiguration* config)
{
    Optimizer::initialize(config);

    // initialize _ind
    ASSERTD(_ind != nullptr);
    _indBuilder->initializeInd(_ind, config->dataSet(), _rng);
    _singleStep = false;

    // construct our initial individual and set initial & best scores
    iterationRun();
    setInitScore(utl::clone(_newScore));
    setBestScore(utl::clone(_newScore));
    if (_ind->newString())
        _ind->acceptNewString();
    auto objective = _objectives[0];
    objective->setBestScore(utl::clone(_bestScore));

    // it is necessary to keep a copy of the _bestStrScore for SA
    auto strScore = new StringScore(0, utl::clone(_ind->stringPtr()), utl::clone(_bestScore));
    setBestStrScore(strScore);

#ifdef DEBUG_UNIT
    utl::cout << initString(!_fail) << utl::endlf;
#endif

    // initialize _strScores[], _acceptedScoresMap
    _beamWidth = 10;
    String<uint_t>* str = _ind->stringPtr();
    for (uint_t i = 0; i < _beamWidth; i++)
    {
        auto strScore = new StringScore(i, utl::clone(str), utl::clone(_bestScore));
        _strScores.push_back(strScore);
        _acceptedScoresMap.insert(uint_score_map_t::value_type(i, utl::clone(_bestScore)));
    }

    // forget about _ind's construction string
    _ind->setString(nullptr, false);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool
MultistartSA::run()
{
    ASSERTD(!complete());
    ASSERTD(_ind != nullptr);

    Objective* objective = _objectives[0];
    Bool complete = this->complete();

    while (!complete)
    {
        // halve _beamWidth at 5%, 25%, and 100% of min-iterations
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
                _acceptedScoresMap.erase(strScore->getId());
                delete strScore;
            }
            _strScores.erase(_strScores.begin() + _beamWidth, _strScores.end());
        }

        for (uint_t i = 0; i < _beamWidth; i++)
        {
            _iteration++;
            auto op = chooseRandomOp();
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
            utl::cout << "operator: " << op->toString() << utl::endl;
#endif

            // generate a schedule
            iterationRun(rop);
            setAcceptedScore(utl::clone(acceptedScores(_strScores[i]->getId())));
            auto diff = objective->scoreDiff(_newScore, _acceptedScore);
            auto acceptProb = exp(diff / _currentTemp);
            _accept = (_rng->uniform(0.0, 1.0) < acceptProb); //_accept
            _sameScore = _newBest = false;
#ifdef DEBUG_UNIT
            utl::cout << temperatureString() << utl::endl;
#endif
            if (_accept)
            {
                rop->accept();
                setAcceptedScore(_strScores[i]->getId(), utl::clone(_newScore));
                _strScores[i]->setScore(utl::clone(_newScore));
                int globalCmpResult = objective->compare(_newScore, _bestScore);
                _sameScore = (globalCmpResult == 0);
                _newBest = (globalCmpResult > 0);
                if (globalCmpResult > 0)
                {
                    rop->addSuccessIter();
                    _improvementIteration = _iteration;
                    setBestScore(utl::clone(_newScore));
                    _bestStrScore->setScore(utl::clone(_newScore));
                    _bestStrScore->setString(utl::clone(_ind->stringPtr()));
                    objective->setBestScore(utl::clone(_bestScore));
                }
            }
            else
            {
                rop->undo();
            }

            _currentTemp = (double)pow(_tempDcrRate, (int)(_iteration / 100)) * _initTemp;
#ifdef DEBUG_UNIT
            utl::cout << "startId:" << _strScores[i]->getId() << "(" << i << "/" << _beamWidth
                      << ", " << Float(_strScores[i]->getScore()->getValue()).toString(0) << "), "
                      << iterationString() << utl::endl;
#endif
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
    ASSERT(this->complete());

    // re-generate the best schedule and get audit text
    _ind->setString(_bestStrScore->getString(), false);
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
MultistartSA::audit()
{
    ASSERTD(_bestStrScore != nullptr);
    _ind->setString(_bestStrScore->getString(), false);
    if (!iterationRun(nullptr, true))
        ABORT();
    ASSERTD(*_bestScore == *_newScore);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
MultistartSA::setAcceptedScore(Score* score)
{
    return SAoptimizer::setAcceptedScore(score);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
MultistartSA::setAcceptedScore(uint_t idx, Score* score)
{
    auto it = _acceptedScoresMap.find(idx);
    ASSERTD(it != _acceptedScoresMap.end());
    delete (*it).second;
    (*it).second = score;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Score*
MultistartSA::acceptedScores(uint_t idx)
{
    auto it = _acceptedScoresMap.find(idx);
    ASSERTD(it != _acceptedScoresMap.end());
    return (*it).second;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
MultistartSA::init()
{
    _beamWidth = 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
MultistartSA::deInit()
{
    deleteCont(_strScores);
    deleteMapSecond(_acceptedScoresMap);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

GOP_NS_END;
