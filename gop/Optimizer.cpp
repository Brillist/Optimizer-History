#include "libgop.h"
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

UTL_CLASS_IMPL_ABC(gop::Optimizer);

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
    _maxIterations = _iteration;
    _minIterations = utl::min(_minIterations, _maxIterations);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

static time_t startTime;

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Optimizer::initialize(const OptimizerConfiguration* config)
{
    startTime = time(0);
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
    copyVector(_ops, config->operators());

    ASSERTD(_rng != nullptr);
    ASSERTD(_minIterations != uint_t_max);
    ASSERTD(_maxIterations != uint_t_max);
    ASSERTD(_improvementGap != uint_t_max);
    ASSERTD(_indBuilder != nullptr);
    ASSERTD(_objectives.size() >= 1);
    ASSERTD(_ops.size() >= 1);

    _indBuilder->initialize(config->dataSet());
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
        _context->setFailed();
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
Optimizer::bestScore(uint_t objectiveIdx) const
{
    ASSERTD(objectiveIdx < _objectives.size());
    return _objectives[objectiveIdx]->getBestScore();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Score*
Optimizer::bestScore(const std::string& objectiveName) const
{
    for (auto objective : _objectives)
    {
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
    for (auto objective : _objectives)
    {
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
    auto objective = _objectives[0];
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
    str << "Init Score: " << _bestScore->toString();
    str << '\0';
    return utl::String((char*)str.get());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

utl::String
Optimizer::finalString(bool feasible) const
{
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
                       .toString(2)
                << "%";
    }
    else
    {
        str << ", NO SOLUTION CAN BE FOUND.";
    }
    str << ", " << Duration(difftime(endTime, startTime)).toString() << '\0';
    return utl::String((char*)str.get());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Optimizer::initializeObjectives()
{
    for (auto objective : _objectives)
    {
        objective->setBestScore(objective->worstPossibleScore());
    }
    setInitScore(_objectives[0]->worstPossibleScore());
    setBestScore(_objectives[0]->worstPossibleScore());
    setNewScore(_objectives[0]->worstPossibleScore());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Optimizer::initializeOps(StringInd<uint_t>* ind)
{
    for (auto op : _ops)
    {
        _indBuilder->setStringBase(op);
        ASSERTD(op->stringBase() != uint_t_max);
        op->setOptimizer(this);
        op->setRNG(_rng);
        op->initialize(_context->dataSet());
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Operator*
Optimizer::chooseSuccessOp() const
{
    double bestSuccessRate = 0.0;
    uint_t bestNumChoices = 0;
    uint_t opIdx = uint_t_max;
    for (auto op : _ops)
    {
        ++opIdx;
        uint_t opNumChoices = op->numChoices();
        if (opNumChoices == 0)
            continue;
        double opSuccessRate = op->p();
        if (opSuccessRate >= bestSuccessRate)
        {
            // same success rate as current best?
            if (opSuccessRate == bestSuccessRate)
            {
                // give each op a fair chance based on its number of choices
                uint_t totalNumChoices = opNumChoices + bestNumChoices;
                uint_t randomNum = _rng->uniform((uint_t)0, totalNumChoices - 1);
                if (randomNum >= opNumChoices)
                    continue;
            }
            bestSuccessRate = opSuccessRate;
            bestNumChoices = opNumChoices;
        }
    }

    // we found an operator?
    if (bestNumChoices != 0)
    {
        // direct the chosen operator to select a variable
        auto op = _ops[opIdx];
        op->selectVar();

        // return the chosen operator
        return op;
    }

    std::cout << "WARNING: This problem has no optimization opportunity!" << std::endl;
    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Operator*
Optimizer::chooseRandomOp() const
{
    // count total choices
    uint_t totalChoices = 0;
    for (auto op : _ops)
    {
        totalChoices += op->numChoices();
    }

    // at least one choice available -> choose an operator
    if (totalChoices != 0)
    {
        uint_t randomNum = _rng->uniform((uint_t)0, totalChoices - 1);
        uint_t cumNum = 0;
        for (auto op : _ops)
        {
            cumNum += op->numChoices();
            if (randomNum < cumNum)
            {
                op->selectVar();
                return op;
            }
        }
    }

    std::cout << "WARNING: This problem has no optimization opportunity!" << std::endl;
    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Operator*
Optimizer::chooseRandomStepOp() const
{
    // count total vars
    uint_t totalSteps = 0;
    for (auto op : _ops)
    {
        totalSteps += op->numVars();
    }

    // randomly choose an operator (giving each one a chance based on its number of vars)
    if (totalSteps != 0)
    {
        uint_t randomNum = _rng->uniform((uint_t)0, totalSteps - 1);
        uint_t cumNum = 0;
        for (auto op : _ops)
        {
            uint_t opNumVars = op->numVars();
            cumNum += opNumVars;
            if (randomNum < cumNum)
            {
                op->selectVar();
                return op;
            }
        }
    }

    std::cout << "WARNING: This problem has no optimization opportunity!" << std::endl;
    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Optimizer::init()
{
    _rng = lut::make_rng();
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
    deleteCont(_objectives);
    deleteCont(_ops);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

GOP_NS_END;
