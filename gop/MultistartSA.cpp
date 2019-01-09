#include "libgop.h"
#include "MultistartSA.h"
#include "RevOperator.h"
#include <libutl/Bool.h>
#include <libutl/Float.h>
#include <libutl/BufferedFDstream.h>

//////////////////////////////////////////////////////////////////////////////

#ifdef DEBUG
#define DEBUG_UNIT
#endif

//////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;

//////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(gop::MultistartSA, gop::SAoptimizer);

//////////////////////////////////////////////////////////////////////////////

GOP_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////

void
MultistartSA::initialize(const OptimizerConfiguration* config)
{
    Optimizer::initialize(config);
    ASSERTD(_ind != nullptr);
    _indBuilder->initializeInd(_ind, config->dataSet(), _rng);
    _singleStep = false;
    Objective* objective = _objectives[0];

    iterationRun();
    setInitScore(utl::clone(_newScore));
    setBestScore(utl::clone(_newScore));
    if (_ind->newString()) _ind->acceptNewString();
    objective->setBestScore(utl::clone(_bestScore));

    // it is necessary to keep a copy of the _bestStrScore for SA
    StringScore* strScore = new StringScore(
        0, utl::clone(_ind->getString()),
        utl::clone(_bestScore));
    setBestStrScore(strScore);

#ifdef DEBUG_UNIT
    utl::cout << initString(!_fail) << utl::endlf;
#endif

//     _ind->setScore(0, _bestScore); //???
    _beamWidth = 10;
    String<uint_t>* str = _ind->getString();
    for (uint_t i = 0; i < _beamWidth; i++)
    {
        StringScore* strScore = new StringScore(
            i,
            utl::clone(str),
            utl::clone(_bestScore));
        _strScores.push_back(strScore);
        _acceptedScoresMap.insert(uint_score_map_t::
                                  value_type(i, utl::clone(_bestScore)));
    }
    _ind->setString(nullptr, false);
}

//////////////////////////////////////////////////////////////////////////////

bool
MultistartSA::run()
{
    ASSERTD(!complete());
    ASSERTD(_ind != nullptr);

    Objective* objective = _objectives[0];
    Bool complete = this->complete();
//     int cmpResult;

    while(!complete)
    {

        // reduce the length of the _strScore list
        if (_iteration == roundUp(_minIterations / 20, _beamWidth) || //5%
            _iteration == roundUp(_minIterations / 4, _beamWidth) || // 25%
            _iteration == roundUp(_minIterations, _beamWidth)) // 100%
        {
            std::sort(_strScores.begin(), _strScores.end(), stringScoreOrdering());
            _beamWidth = utl::max((uint_t)1, (_beamWidth / 2));
            stringscore_vector_t::iterator it;
            for (it = _strScores.begin() + _beamWidth;
                 it != _strScores.end();
                 ++it)
            {
                StringScore* strScore = *it;
                _acceptedScoresMap.erase(strScore->getId());
                delete strScore;
            }
            _strScores.erase(
                _strScores.begin() + _beamWidth,
                _strScores.end());
        }

        for (uint_t i = 0; i < _beamWidth; i++)
        {
            _iteration++;
            // choose an operator
            Operator* op = chooseRandomOp();
            if (op == nullptr)
            {
                _iteration = _maxIterations;
                complete = this->complete();
                break;
            }
            ASSERTD(dynamic_cast<RevOperator*>(op) != nullptr);
            RevOperator* rop = (RevOperator*)op;
            rop->addTotalIter();
            _ind->setString(_strScores[i]->getString(), false);//
#ifdef DEBUG_UNIT
            utl::cout
                << "                                "
                << "operator: " << op->toString() << utl::endl;
#endif

            // generate a schedule
            iterationRun(rop);
            setAcceptedScore(utl::clone(
                                 acceptedScores(
                                     _strScores[i]->getId())));
            double diff;
            diff = objective->scoreDiff(
                _newScore,
                _acceptedScore);
            double acceptProb = exp(diff / _currentTemp);
            _accept = (_rng->evalf() < acceptProb);//_accept
            _sameScore = _newBest = false;
#ifdef DEBUG_UNIT
            utl::cout << temperatureString() << utl::endl;
#endif
            if (_accept)
            {
                rop->accept();
                setAcceptedScore(_strScores[i]->getId(),
                                 utl::clone(_newScore));
                _strScores[i]->setScore(utl::clone(_newScore));
                int globalCmpResult = objective->
                    compare(_newScore, _bestScore);
                _sameScore = (globalCmpResult == 0);//_sameScore
                _newBest = (globalCmpResult > 0);//_newBest
                if (globalCmpResult > 0)
                {
                    rop->addSuccessIter();
                    _improvementIteration = _iteration;
                    setBestScore(utl::clone(_newScore));
                    _bestStrScore->setScore(utl::clone(_newScore));
                    _bestStrScore->setString(
                        utl::clone(_ind->getString()));
                    objective->setBestScore(utl::clone(_bestScore));
                }
            }
            else
            {
                rop->undo();
            }
            _currentTemp =
                (double)pow(
                    _tempDcrRate,
                    (int)(_iteration /100)) * _initTemp;
#ifdef DEBUG_UNIT
            utl::cout << "startId:" << _strScores[i]->getId()
                      << "(" << i
                      << "/" << _beamWidth
                      << ", "
                      << Float(_strScores[i]->getScore()->getValue()).
                toString("precision:0")
                      << "), " << iterationString() << utl::endl;
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
    //re-generate the best schedule and get audit text
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

//////////////////////////////////////////////////////////////////////////////

void
MultistartSA::audit()
{
    ASSERTD(_bestStrScore != nullptr);
    _ind->setString(_bestStrScore->getString(), false);
    if (!iterationRun(nullptr, true)) ABORT();
    ASSERTD(*_bestScore == *_newScore);
}

//////////////////////////////////////////////////////////////////////////////

void
MultistartSA::setAcceptedScore(Score* score)
{
    return SAoptimizer::setAcceptedScore(score);
}

//////////////////////////////////////////////////////////////////////////////

void
MultistartSA::setAcceptedScore(uint_t idx, Score* score)
{
    uint_score_map_t::iterator it =
        _acceptedScoresMap.find(idx);
    ASSERTD(it != _acceptedScoresMap.end());
    delete (*it).second;
    (*it).second = score;
}

//////////////////////////////////////////////////////////////////////////////

Score*
MultistartSA::acceptedScores(uint_t idx)
{
    uint_score_map_t::iterator it =
        _acceptedScoresMap.find(idx);
    ASSERTD(it != _acceptedScoresMap.end());
    return (*it).second;
}

//////////////////////////////////////////////////////////////////////////////

void
MultistartSA::init()
{
    _beamWidth = 0;
}

//////////////////////////////////////////////////////////////////////////////

void
MultistartSA::deInit()
{
    deleteCont(_strScores);
    deleteMapSecond(_acceptedScoresMap);
}

//////////////////////////////////////////////////////////////////////////////

GOP_NS_END;
