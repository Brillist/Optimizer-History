#include "libcse.h"
#include "ScheduleEvaluator.h"
#include "ScheduleEvaluatorConfiguration.h"
#include "SchedulingContext.h"
#include <libutl/BufferedFDstream.h>

#ifdef DEBUG
#define DEBUG_UNIT
#endif

//////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;
GOP_NS_USE;

//////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL_ABC(cse::ScheduleEvaluator, gop::IndEvaluator);

//////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////

void
ScheduleEvaluator::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(ScheduleEvaluator));
    const ScheduleEvaluator& se = (const ScheduleEvaluator&)rhs;
    IndEvaluator::copy(se);
    delete _schedulerConfig;
    _schedulerConfig = lut::clone(se._schedulerConfig);
}

//////////////////////////////////////////////////////////////////////////////

void
ScheduleEvaluator::initialize(const gop::IndEvaluatorConfiguration* p_cf)
{
    ASSERTD(dynamic_cast<const ScheduleEvaluatorConfiguration*>(p_cf) != nullptr);
    const ScheduleEvaluatorConfiguration& cf =
        (const ScheduleEvaluatorConfiguration&)*p_cf;
    IndEvaluator::initialize(cf);
    delete _schedulerConfig;
    _schedulerConfig = lut::clone(cf.getSchedulerConfig());
}

//////////////////////////////////////////////////////////////////////////////

Score*
ScheduleEvaluator::eval(
    const gop::IndBuilderContext* p_context) const
{
    ASSERTD(dynamic_cast<const SchedulingContext*>(p_context) != nullptr);
    const SchedulingContext* context = (const SchedulingContext*)p_context;
    Score* score = new Score();
    uint_t numUnscheduledOps = context->clevorDataSet()->sops().size()
        - context->numScheduledOps();

    // construction failed?
    if (context->failed())
    {
#ifdef DEBUG_UNIT
        utl::cout << "Unschedulable Ops (jobId,opId):";
        const job_set_id_t& jobs = context->clevorDataSet()->jobs();
        job_set_id_t::const_iterator jobIt, jobLim = jobs.end();
        for (jobIt = jobs.begin(); jobIt != jobLim; ++jobIt)
        {
            Job* job = *jobIt;
            const jobop_set_id_t& ops = job->allSops();
            jobop_set_id_t::const_iterator opIt, opLim = ops.end();
            for (opIt = ops.begin(); opIt != opLim; ++opIt)
            {
                JobOp* op = *opIt;
                cls::Activity* act = op->activity();
                if (!act->esBound().finalized())
                {
                    utl::cout << "(" << job->id() << ","
                              << op->id() << ") ";
                    BREAKPOINT;
                }
            }
        }
        utl::cout << utl::endl;
#endif
        score->setValue(numUnscheduledOps);
        score->setType(score_failed);
        return score;
    }

    // hard constraint violation?
    uint_t ctScore = context->hardCtScore();
    if (ctScore != 0)
    {
        score->setValue(ctScore);
        score->setType(score_ct_violated);
        return score;
    }

    // construction was successful
    score->setType(score_succeeded);
    score->setValue(calcScore(context));
    return score;
}

//////////////////////////////////////////////////////////////////////////////

CSE_NS_END;
