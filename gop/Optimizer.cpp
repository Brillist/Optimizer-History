#include "libgop.h"
#include <libutl/R250.h>
#include <libutl/MemStream.h>
#include <libutl/Float.h>
#include <libutl/Duration.h>
#include "Optimizer.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef DEBUG
#define DEBUG_UNIT
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL_ABC(gop::Optimizer, utl::Object);

////////////////////////////////////////////////////////////////////////////////////////////////////

GOP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

bool
Optimizer::complete() const
{
    if (_iteration == 0)
        return false;
    if (_iteration < _minIterations)
        return false;
    if (_iteration >= _maxIterations)
        return true;
    // note: _minIterations <= _iteration < _maxIterations
    return (((_iteration - 1) - _improvementIteration) >= _improvementGap);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Optimizer::stop()
{
    //     _iteration = _maxIterations;
    // this change is to show correct total iterations
    // when the optimizer is stopped. JZ, April 11, 2008
    _maxIterations = _iteration;
    _minIterations = utl::min(_minIterations, _maxIterations);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

static time_t startTime;

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Optimizer::initialize(const OptimizerConfiguration* config)
{
    //     _timer->start();
    startTime = time(0); //get current time
    _iteration = 0;
    _improvementIteration = 0;
    _minIterations = config->minIterations();
    _maxIterations = config->maxIterations();
    _improvementGap = config->improvementGap();
    if (_improvementGap == 0)
        _improvementGap = 1;
    delete _ind;
    _ind = lut::clone(config->ind());
    delete _indBuilder;
    _indBuilder = lut::clone(config->indBuilder());
    _context = config->context();
    copyVector(_objectives, config->objectives());
    copyVector(_ops, config->getOperators());

    ASSERTD(_rng != nullptr);
    ASSERTD(_minIterations != uint_t_max);
    ASSERTD(_maxIterations != uint_t_max);
    ASSERTD(_improvementGap != uint_t_max);
    ASSERTD(_indBuilder != nullptr);
    ASSERTD(_objectives.size() >= 1);
    ASSERTD(_ops.size() >= 1);

    _indBuilder->initialize(config->dataSet());
    //     if (_calcStatsEnable)
    //     {
    //         initializeStats();
    //     }
    initializeObjectives();
    initializeOps(config->ind());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool
Optimizer::iterationRun(Operator* op, bool audit)
{
    // single objective only function
    ASSERTD(_ind != nullptr);
    ASSERTD(_context != nullptr);

    Objective* objective = _objectives[0];
    try
    {
        _context->clear();
        if (op)
            op->execute(_ind, _context, _singleStep);
        _indBuilder->run(_ind, _context);
        _fail = false;
    }
    catch (...)
    {
        _context->failed() = true;
        _fail = true;
    }

    // audit?
    if (!_fail && audit)
    {
        objective->indEvaluator()->auditNext();
    }

    setNewScore(objective->eval(_context));
    return ((_newScore->getType() == score_succeeded) ||
            (_newScore->getType() == score_ct_violated));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Optimizer::updateRunStatus(bool complete)
{
    _runStatus->update(complete, _iteration, _improvementIteration, _bestScore);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Score*
Optimizer::bestScore(utl::uint_t objectiveIdx) const
{
    ASSERTD(objectiveIdx < _objectives.size());
    return _objectives[objectiveIdx]->getBestScore();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Score*
Optimizer::bestScore(const std::string& objectiveName) const
{
    for (uint_t i = 0; i < _objectives.size(); ++i)
    {
        const Objective* objective = _objectives[i];
        if (objective->name() == objectiveName)
        {
            return objective->getBestScore();
        }
    }
    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int
Optimizer::bestScoreComponent(const std::string& objectiveName,
                              const std::string& componentName) const
{
    for (uint_t i = 0; i < _objectives.size(); ++i)
    {
        const Objective* objective = _objectives[i];
        if (objective->name() == objectiveName)
        {
            return objective->getBestScoreComponent(componentName);
        }
    }
    return uint_t_max;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

const std::string&
Optimizer::bestScoreAudit() const
{
    // single objective only function
    ASSERTD(_objectives.size() >= 1);
    Objective* objective = _objectives[0];
    return objective->indEvaluator()->auditText();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

utl::String
Optimizer::iterationString() const
{
    // single objective only function
    utl::MemStream str;
    str << "iteration:" << _iteration << ", lastImprv:" << _improvementIteration
        << ", bestScore:" << _bestScore->toString() << ", newScore:" << _newScore->toString();
    const char* failstr = _fail ? ":failure" : ":success";
    const char* acceptstr = _accept ? ":accepted" : ":rejected";
    const char* sameScorestr = (_accept && _sameScore) ? ":same_score" : "";
    const char* newBeststr = (_accept && _newBest) ? ":new_best!" : "";
    str << ", " << failstr << acceptstr << sameScorestr << newBeststr << '\0';
    return utl::String((char*)str.get());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

utl::String
Optimizer::initString(bool feasible) const
{
    // single objective only function
    utl::MemStream str;
    str << "Random Seed: " << _rng->getSeed();
    str << ", Init Score: " << _bestScore->toString();
    str << '\0';
    return utl::String((char*)str.get());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

utl::String
Optimizer::finalString(bool feasible) const
{
    // single objective only function
    //     _timer->stop();
    time_t endTime = time(0);
    utl::MemStream str;
    str << getClassName();

    // find whether it uses OpSeqOnly
    bool opSeqOnly = false;
    uint_t numOps = _ops.size();
    for (uint_t i = 0; i < numOps; ++i)
    {
        Operator* op = _ops[i];
        if (op->name() == "OpSeqMutate")
        {
            opSeqOnly = true;
            break;
        }
    }
    if (opSeqOnly)
        str << "(OpSeq_Only)";

    str << ", " << _improvementIteration << "/" << _maxIterations;
    if (feasible)
    {
        str << ", ";
        switch (_initScore->getType())
        {
        case score_failed:
            str << "(failed)";
            break;
        case score_ct_violated:
            str << "(ct_violated)";
            break;
        case score_succeeded:
            str << _initScore->toString();
            break;
        default:
            ABORT();
        }
        ASSERT(_bestScore->getType() == score_ct_violated ||
               _bestScore->getType() == score_succeeded);
        str << "->" << _bestScore->toString();
        if ((_initScore->getType() == score_succeeded) &&
            (_bestScore->getType() == score_succeeded))
            str << ", "
                << Float(100.0 * (_initScore->getValue() - _bestScore->getValue()) /
                         _initScore->getValue())
                       .toString("precision:2")
                << "%";
    }
    else
    {
        str << ", NO SOLUTION CAN BE FOUND.";
    }
    //Joe added 100000.0 because there seems to a bug in OStime class
    //     str << ", "
    //         << Float(_timer->getTotalTime() * 10000.0).toString("precision:2")
    //         << "sec" << '\0';
    str << ", " << Duration(difftime(endTime, startTime)).toString() << '\0';
    return utl::String((char*)str.get());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

// void
// Optimizer::initializeStats()
// {
//     // statistics
//     delete [] _opStats; _opStats = nullptr;
//     if (_ops.size() > 0)
//     {
//         _opStats = new OperatorStats[_ops.size()];
//     }
// }

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Optimizer::initializeObjectives()
{
    uint_t numObjectives = _objectives.size();
    for (uint_t i = 0; i < numObjectives; ++i)
    {
        Objective* objective = _objectives[i];
        objective->setBestScore(objective->worstPossibleScore());
    }
    setInitScore(_objectives[0]->worstPossibleScore());
    setBestScore(_objectives[0]->worstPossibleScore());
    setNewScore(_objectives[0]->worstPossibleScore());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Optimizer::initializeOps(StringInd<utl::uint_t>* ind)
{
    uint_t numOps = _ops.size();

    // set operator cumulative percentages
    //     delete [] _opsCumPct;
    //     _opsCumPct = nullptr;
    //     if (numOps > 0)
    //     {
    //         uint_t i;
    //         double totP = 0.0;
    //         for (i = 0; i < numOps; ++i)
    //         {
    //             totP += _ops[i]->getP();
    //         }
    //         if (totP == 0.0)
    //         {
    //             double p = 1.0 / (double)numOps;
    //             for (i = 0; i < numOps; ++i)
    //             {
    //                 _ops[i]->setP(p);
    //             }
    //         }
    //         _opsCumPct = new double[_ops.size()];
    //         _opsCumPct[0] = _ops[0]->getP() / totP;
    //         for (uint_t i = 1; i < _ops.size(); i++)
    //         {
    //             _opsCumPct[i] = _opsCumPct[i - 1] + (_ops[i]->getP() / totP);
    //         }
    //     }

    for (uint_t i = 0; i < numOps; ++i)
    {
        Operator* op = _ops[i];
        _indBuilder->setStringBase(op);
        ASSERTD(op->getStringBase() != uint_t_max);
        op->setOptimizer(this);
        op->setRNG(_rng);
        op->initialize(_context->dataSet());
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Operator*
Optimizer::chooseSuccessOp() const
{
    uint_t numOps = _ops.size();
    uint_t opIdx = uint_t_max;
    double bestSuccessRate = 0.0;
    uint_t numChoices;
    for (uint_t i = 0; i < numOps; i++)
    {
        uint_t opNumChoices = _ops[i]->getNumChoices();
        if (opNumChoices == 0)
            continue;
        double opSuccessRate = _ops[i]->p();
        if (opSuccessRate >= bestSuccessRate)
        {
            if (opSuccessRate == bestSuccessRate)
            {
                uint_t totalNumChoices = opNumChoices + numChoices;
                uint_t randomNum = _rng->evali(totalNumChoices);
                if (randomNum >= opNumChoices)
                    continue;
            }
            bestSuccessRate = opSuccessRate;
            opIdx = i;
            numChoices = opNumChoices;
        }
    }
    if (opIdx != uint_t_max)
    {
        Operator* op = _ops[opIdx];
        op->selectOperatorVarIdx();
        return op;
    }
    else
    {
        std::cout << "WARNING: This problem has no optimization opportunity!" << std::endl;
        return nullptr;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Operator*
Optimizer::chooseRandomOp() const
{
    uint_t totalChoices = 0;
    uint_t numOps = _ops.size();
    for (uint_t i = 0; i < numOps; i++)
    {
        totalChoices += _ops[i]->getNumChoices();
    }
    uint_t randomNum = _rng->evali(totalChoices);
    uint_t cumNum = 0;
    for (uint_t i = 0; i < numOps; i++)
    {
        if (_ops[i]->getNumChoices() == 0)
        {
            continue;
        }
        cumNum += _ops[i]->getNumChoices();
        if (randomNum < cumNum)
        {
            Operator* op = _ops[i];
            op->selectOperatorVarIdx();
            return op;
        }
    }
    std::cout << "WARNING: This problem has no optimization opportunity!" << std::endl;
    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Operator*
Optimizer::chooseRandomStepOp() const
{
    uint_t totalSteps = 0;
    uint_t numOps = _ops.size();
    for (uint_t i = 0; i < numOps; i++)
    {
        totalSteps += _ops[i]->numVars();
    }
    uint_t randomNum = _rng->evali(totalSteps);
    uint_t cumNum = 0;
    for (uint_t i = 0; i < numOps; i++)
    {
        if (_ops[i]->numVars() == 0)
        {
            continue;
        }
        cumNum += _ops[i]->numVars();
        if (randomNum < cumNum)
        {
            Operator* op = _ops[i];
            op->selectOperatorVarIdx();
            return op;
        }
    }
    std::cout << "WARNING: This problem has no optimization opportunity!" << std::endl;
    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

// void
// Optimizer::calcStats(const Population& pop) const
// {
//     if (_iteration == 0)
//     {
//         return;
//     }

//     // update statistics for all operators
//     uint_t i, popSize = pop.size();
//     OperatorStats* os;
//     for (i = 0; i < popSize; i++)
//     {
//         const Ind& ind = pop(i);
//         if (ind.getOpIdx() == uint_t_max)
//         {
//             continue;
//         }
//         os = &_opStats[ind.getOpIdx()];
//         for (uint_t j = 0; j < _objectives.size(); ++j)
//         {
//             Objective* objective = _objectives[j];
//             os->add(
//                 j,
//                 objective->goal(),
//                 ind.getScore(j),
//                 ind.getParentScore(j));
//         }
//     }
// }

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Optimizer::init()
{
#ifdef DEBUG
    _rng = new R250(1042614900);
#else
    _rng = new R250(1042614900);
    //_rng = new R250();
#endif
    _iteration = 0;
    _improvementIteration = 0;
    _minIterations = uint_t_max;
    _maxIterations = uint_t_max;
    _improvementGap = uint_t_max;
    _runStatus = new RunStatus();
    _initScore = nullptr;
    _bestScore = nullptr;
    _newScore = nullptr;
    _ind = nullptr;
    _indBuilder = nullptr;
    _singleStep = false;
    //     _timer = new OStimer();
    //     _opsCumPct = nullptr;
    //     _calcStatsEnable = false;
    //     _opStats = nullptr;

    _fail = false;
    _accept = false;
    _sameScore = false;
    _newBest = false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Optimizer::deInit()
{
    delete _rng;
    delete _runStatus;
    delete _ind;
    delete _indBuilder;
    delete _initScore;
    delete _bestScore;
    delete _newScore;
    //     delete _timer;
    deleteCont(_objectives);
    deleteCont(_ops);
    //     delete [] _opsCumPct;
    //     delete [] _opStats;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

GOP_NS_END;
