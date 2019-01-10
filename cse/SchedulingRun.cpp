#include "libcse.h"
#include <libutl/Bool.h>
#include <libutl/Float.h>
#include <libutl/Duration.h>
#include <clp/FailEx.h>
#include "SchedulingRun.h"
#include "TotalCostEvaluator.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;
CLP_NS_USE;
CLS_NS_USE;
GOP_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(cse::SchedulingRun, utl::Object);

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

void
SchedulingRun::initialize(ClevorDataSet* dataSet,
                          Scheduler* scheduler,
                          const objective_vector_t& objectives)
{
    delete _optimizer;
    _optimizer = nullptr;

    // context
    delete _context;
    _context = new SchedulingContext();

    // scheduler
    delete _scheduler;
    _scheduler = scheduler;

    // objectives
    deleteCont(_objectives);
    _objectives = objectives;

    // initialize context
    _context->initialize(dataSet);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
SchedulingRun::initialize(ClevorDataSet* dataSet,
                          Optimizer* optimizer,
                          OptimizerConfiguration* optimizerConfig)
{
    delete _scheduler;
    _scheduler = nullptr;
    deleteCont(_objectives);

    // context
    delete _context;
    _context = new SchedulingContext();
    optimizerConfig->context() = _context;

    // optimizer
    delete _optimizer;
    _optimizer = optimizer;

    // initialize context
    _context->initialize(dataSet);
    _optimizer->initialize(optimizerConfig);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool
SchedulingRun::run()
{
    ASSERTD((_optimizer != nullptr) || (_scheduler != nullptr));

    bool res = true;
    if (_optimizer == nullptr)
    {
        time_t startTime = time(0);
        _context->clear();
        _scheduler->run(nullptr, _context);
        time_t endTime = time(0);
        Score* score = nullptr;
        if (res)
        {
            uint_t numObjectives = this->numObjectives();
            for (uint_t i = 0; i < numObjectives; ++i)
            {
                Objective* obj = _objectives[i];
                obj->indEvaluator()->auditNext();
                score = obj->eval(_context);
                obj->setBestScore(score);
            }
            // there should be only one objective.
            utl::cout << "Forward, ";
            if (score != nullptr)
            {
                switch (score->getType())
                {
                case score_failed:
                    utl::cout << "(failed), ";
                    break;
                case score_ct_violated:
                    utl::cout << "(ct_vialated), ";
                    break;
                case score_succeeded:
                    utl::cout << score->toString() << ", ";
                    break;
                default:
                    ABORT();
                }
            }
            // see note in Optimizer.cpp
            utl::cout << Duration(difftime(endTime, startTime)).toString() << utl::endlf;
            _context->store();
        }
    }
    else
    {
        res = _optimizer->run();
        if (res)
        {
            _context->store();
        }
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
SchedulingRun::stop()
{
    if (_optimizer)
        _optimizer->stop();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
SchedulingRun::audit()
{
    if (_optimizer == nullptr)
    {
        return;
    }
    _optimizer->audit();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

uint_t
SchedulingRun::numObjectives() const
{
    if (_optimizer == nullptr)
    {
        return _objectives.size();
    }
    else
    {
        return _optimizer->objectives().size();
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

time_t
SchedulingRun::makespan() const
{
    return _context->makespan();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

Score*
SchedulingRun::bestScore(uint_t objectiveIdx) const
{
    if (_optimizer == nullptr)
    {
        ASSERTD(objectiveIdx < _objectives.size());
        return _objectives[objectiveIdx]->getBestScore();
    }
    else
    {
        return _optimizer->bestScore(objectiveIdx);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

Score*
SchedulingRun::bestScore(const std::string& objectiveName) const
{
    if (_optimizer == nullptr)
    {
        for (uint_t i = 0; i < _objectives.size(); ++i)
        {
            Objective* objective = _objectives[i];
            if (objective->name() == objectiveName)
            {
                return objective->getBestScore();
            }
        }
        return nullptr;
    }
    else
    {
        return _optimizer->bestScore(objectiveName);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

int
SchedulingRun::bestScoreComponent(const std::string& objectiveName,
                                  const std::string& componentName) const
{
    if (_optimizer == nullptr)
    {
        for (uint_t i = 0; i < _objectives.size(); ++i)
        {
            Objective* objective = _objectives[i];
            if (objective->name() == objectiveName)
            {
                return objective->getBestScoreComponent(componentName);
            }
        }
        return int_t_max;
    }
    else
    {
        return _optimizer->bestScoreComponent(objectiveName, componentName);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

const std::string&
SchedulingRun::bestScoreAudit() const
{
    if (_optimizer == nullptr)
    {
        Objective* objective = _objectives[0];
        const std::string& auditText = objective->indEvaluator()->auditText();
        return auditText;
    }
    else
    {
        const std::string& auditText = _optimizer->bestScoreAudit();
        return auditText;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

const AuditReport*
SchedulingRun::bestScoreAuditReport() const
{
    AuditReport* report = nullptr;
    gop::Objective* objective = nullptr;
    if (_optimizer == nullptr)
    {
        objective = _objectives[0];
    }
    else
    {
        objective = _optimizer->objectives()[0];
    }
    const TotalCostEvaluator* eval =
        dynamic_cast<const TotalCostEvaluator*>(objective->indEvaluator());
    if (eval != nullptr)
    {
        report = eval->auditReport();
    }
    return report;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
SchedulingRun::init()
{
    _context = nullptr;
    _optimizer = nullptr;
    _scheduler = nullptr;

#ifdef UTL_GBLNEW_DEBUG
    atexit(memReportLeaks);
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
SchedulingRun::deInit()
{
    delete _context;
    delete _optimizer;
    delete _scheduler;
    deleteCont(_objectives);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;
