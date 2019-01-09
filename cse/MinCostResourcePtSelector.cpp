#include "libcse.h"
#include <cls/BrkActivity.h>
#include <cls/DiscreteResource.h>
#include <gop/ConfigEx.h>
#include <gop/StringInd.h>
#include "ForwardScheduler.h"
#include "MinCostResourcePtSelector.h"
#include "ResourceCost.h"
#include "SchedulingContext.h"
#include "MinCostHeuristics.h"

//////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;
CLP_NS_USE;
CLS_NS_USE;
GOP_NS_USE;

//////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(cse::MinCostResourcePtSelector, cse::Scheduler);

//////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////

void
MinCostResourcePtSelector::run(
    Ind* ind,
    IndBuilderContext* p_context) const
{
    ASSERTD(dynamic_cast<SchedulingContext*>(p_context) != nullptr);
    SchedulingContext* context = (SchedulingContext*)p_context;
    const MinCostHeuristics* minCostHeuristics
        = context->clevorDataSet()->minCostHeuristics();
    Manager* mgr = context->manager();

    // handle inactive jobs
    const job_set_id_t& jobs = context->clevorDataSet()->jobs();
    job_set_id_t::const_iterator jobIt;
    for (jobIt = jobs.begin(); jobIt != jobs.end(); jobIt++)
    {
        Job* job = *jobIt;
        if (!job->active())
        {
            job->finalize(mgr);
        }
    }

    // iterate over ops
    const jobop_set_id_t& ops = context->clevorDataSet()->sops();
    jobop_set_id_t::const_iterator it;
    for (it = ops.begin(); it != ops.end(); ++it)
    {
        // identify op
        JobOp* op = *it;
        if (!op->job()->active()) continue;
        const MinCostAltResPt* minCostAltResPt
            = minCostHeuristics->getMinCostAltResPt(op);
        if (minCostAltResPt == nullptr) continue;
        Activity* act = op->activity();
        ASSERTD(act->isA(PtActivity));
        PtActivity* ptact = (PtActivity*)act;
        uint_t minCostPt = minCostAltResPt->pt();
        uint_vector_t minCostAltResIdxs = minCostAltResPt->altResIdxs();

        // set lowest cost pt (and associated resources)
        ptact->selectPt(minCostPt);
        setSelectedResources(op, minCostAltResIdxs);
        mgr->propagate();
    }

    ASSERTD(_nestedScheduler != nullptr);
    _nestedScheduler->run(ind, context);
}

//////////////////////////////////////////////////////////////////////////////

void
MinCostResourcePtSelector::setSelectedResources(
    JobOp* op,
    const uint_vector_t& altResIdx) const
{
    uint_t numResGroupReqs = altResIdx.size();
    for (uint_t i = 0; i < numResGroupReqs; ++i)
    {
        const cse::ResourceGroupRequirement* cseResGroupReq
            = op->getResGroupReq(i);
        cls::DiscreteResourceRequirement* clsResReq
            = cseResGroupReq->clsResReq();
        const ResourceCapPts* resCapPts
            = clsResReq->resIdxCapPts(altResIdx[i]);
        clsResReq->selectResource(resCapPts->resourceId());
//         std::cout << "op:" << op->id()
//                   << ", resGrp:" << cseResGroupReq->resourceGroupId()
//                   << ", res:" << resCapPts->resourceId()
//                   << std::endl;
    }
}

//////////////////////////////////////////////////////////////////////////////

CSE_NS_END;
