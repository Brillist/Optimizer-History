#include "libcse.h"
#include <cls/BrkActivity.h>
#include <cls/IntActivity.h>
#include "DiscreteResource.h"
#include "ClevorDataSet.h"
#include "JobOp.h"
#include "MinCostHeuristics.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;
CLP_NS_USE;
CLS_NS_USE;
GOP_NS_USE;

/////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(cse::MinCostHeuristics);
UTL_CLASS_IMPL(cse::MinCostAltResPt);

/////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

/////////////////////////////////////////////////////////////////////////////////////////////////////

void
MinCostHeuristics::initialize(ClevorDataSet* dataSet)
{
    ASSERTD(dataSet != nullptr);
    _timeStep = dataSet->schedulerConfig()->timeStep();
    _rng = make_rng();

    // iterate over ops to set every op's minCostPt and minCostAltResIdxs
    const jobop_set_id_t& ops = dataSet->sops();
    jobop_set_id_t::const_iterator it;
    for (it = ops.begin(); it != ops.end(); ++it)
    {
        JobOp* op = *it;
        if (op->frozen())
            continue;
        Activity* act = op->activity();
        ASSERTD(act != nullptr);
        if (!act->isA(PtActivity))
            continue;
        if (op->interruptible())
        {
            continue;
        }
        else if (op->breakable())
        {
            BrkActivity* act = op->brkact();
            const IntExp& ptExp = act->possiblePts();
            uint_t numResGroupReqs = op->numResGroupReqs();
            if (ptExp.isBound() && numResGroupReqs == 0)
                continue;
            setMinCostAltResPt(op);
        }
        else
        {
            std::cout << "Warning(MinCostHeuristics.cpp): op" << op->id()
                      << " is not interruptible or breakable" << std::endl;
            continue;
        }
    }
    return;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

void
MinCostHeuristics::setMinCostAltResPt(const JobOp* op)
{
    if (op->interruptible())
    {
        return;
    }
    ASSERTD(op->breakable());
    double minCost = utl::double_t_max; //(double)uint_t_max;
    uint_t minCostPt = uint_t_max;
    uint_vector_t minCostAltResIdx;
    BrkActivity* act = op->brkact();
    uint_t numResReqs = op->numResReqs();
    uint_t numResGroupReqs = op->numResGroupReqs();
    const IntExp& ptExp = act->possiblePts();
    minCostAltResIdx.resize(numResGroupReqs);
    uint_vector_t ptCandidates;
    std::vector<uint_vector_t> altResIdxCandidates;

    IntExpDomainIt* ptIt;
    for (ptIt = ptExp.begin(); !ptIt->atEnd(); ptIt->next())
    {
        double cost = 0.0;
        uint_vector_t altResIdx;
        altResIdx.resize(numResGroupReqs);
        uint_t pt = **ptIt;
        // spin thru res-reqs
        for (uint_t i = 0; i < numResReqs; ++i)
        {
            const cse::ResourceRequirement* cseResReq = op->getResReq(i);
            const cls::DiscreteResourceRequirement* clsResReq =
                (cls::DiscreteResourceRequirement*)cseResReq->clsResReq();
            if (clsResReq == nullptr)
                continue;
            const ResourceCapPts* resCapPts = clsResReq->resCapPts();
            cls::DiscreteResource* res =
                dynamic_cast<cls::DiscreteResource*>(resCapPts->resource());
            if (res == nullptr)
                continue;
            ResourceCost* resCost = (ResourceCost*)res->object();
            uint_t cap = findCap(resCapPts, pt);
            ASSERTD(cap != uint_t_max);
            cost += getResCost(cap, pt, resCost);
        }

        // spin thru res-group-reqs
        for (uint_t rgrIdx = 0; rgrIdx < numResGroupReqs; ++rgrIdx)
        {
            const cse::ResourceGroupRequirement* cseResGroupReq = op->getResGroupReq(rgrIdx);
            const cls::DiscreteResourceRequirement* clsResReq = cseResGroupReq->clsResReq();
            const utl::Hashtable& resCapPtsSet = clsResReq->resCapPtsSet();
            uint_t numResources = resCapPtsSet.size();
            double minRgrCost = utl::double_t_max; //(double)uint_t_max;
            uint_vector_t idxCandidates;
            for (uint_t resIdx = 0; resIdx < numResources; ++resIdx)
            {
                const ResourceCapPts* resCapPts = clsResReq->resIdxCapPts(resIdx);
                cls::DiscreteResource* res =
                    dynamic_cast<cls::DiscreteResource*>(resCapPts->resource());
                if (res == nullptr)
                    continue;
                ResourceCost* resCost = (ResourceCost*)res->object();
                uint_t cap = findCap(resCapPts, pt);
                // pt impossible for this resource?
                if (cap == uint_t_max)
                    continue;

                double curResCost = getResCost(cap, pt, resCost);
                int cmpResult = lut::compare(curResCost, minRgrCost);
                if (cmpResult <= 0)
                //                 if (curResCost <= minRgrCost)
                {
                    if (cmpResult < 0)
                    //                     if (curResCost < minRgrCost)
                    {
                        minRgrCost = curResCost;
                        idxCandidates.clear();
                    }
                    idxCandidates.push_back(resIdx);
                }
            }
            ASSERTD(minRgrCost < utl::double_t_max); //(double)uint_t_max);
            ASSERTD(idxCandidates.size() > 0);
            uint_t idx = _rng->uniform((size_t)0, idxCandidates.size() - 1);
            altResIdx[rgrIdx] = idxCandidates[idx];
            cost += minRgrCost;
        }

        if (cost <= minCost)
        {
            if (cost < minCost)
            {
                minCost = cost;
                ptCandidates.clear();
                altResIdxCandidates.clear();
            }
            ptCandidates.push_back(pt);
            altResIdxCandidates.push_back(altResIdx);
        }
    }
    ASSERTD(minCost < (double)uint_t_max);
    ASSERTD(ptCandidates.size() > 0);
    ASSERTD(ptCandidates.size() == altResIdxCandidates.size());
    uint_t idx = _rng->uniform((size_t)0, ptCandidates.size() - 1);
    minCostPt = ptCandidates[idx];
    minCostAltResIdx = altResIdxCandidates[idx];
    delete ptIt;
    MinCostAltResPt* altResPt = new MinCostAltResPt(minCostPt, minCostAltResIdx);
    _opAltResPt.insert(jobop_altrespt_map_t::value_type(op, altResPt));

    return;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

uint_t
MinCostHeuristics::findCap(const ResourceCapPts* resCapPts, uint_t p_pt)
{
    uint_t numCapPts = resCapPts->numCapPts();
    for (uint_t i = 0; i < numCapPts; ++i)
    {
        uint_t cap, pt;
        resCapPts->getCapPt(i, cap, pt);
        if (pt == p_pt)
        {
            return cap;
        }
    }
    return uint_t_max;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

double
MinCostHeuristics::getResCost(uint_t cap, uint_t pt, const ResourceCost* resCost)
{
    uint_t timeStep = _timeStep;
    uint_t tsPerHour = (60 * 60) / timeStep;
    uint_t tsPerDay = (24 * 60 * 60) / timeStep;
    uint_t tsPerWeek = 7 * tsPerDay;
    uint_t tsPerMonth = 30 * tsPerDay;

    // round pt up to nearest day
    //pt = roundUp(pt, tsPerDay);

    //Note: (1) cap should be roundUp based on 'minCap'
    //          (e.g. 100) in the future
    //      (2) tsPerDay shouldn't assume 24 working hours per day.
    // June 11, 2004. -Joe
    double capXpt = (double)cap * (double)pt;

    if ((pt >= tsPerMonth) && (resCost->costPerMonth() != double_t_max))
    {
        return (capXpt / (double)tsPerMonth) * resCost->costPerMonth();
    }
    else if ((pt >= tsPerWeek) && (resCost->costPerWeek() != double_t_max))
    {
        return (capXpt / (double)tsPerWeek) * resCost->costPerWeek();
    }
    else if ((pt >= tsPerDay) && (resCost->costPerDay() != double_t_max))
    {
        return (capXpt / (double)tsPerDay) * resCost->costPerDay();
    }
    else if ((resCost->costPerHour() != double_t_max) && (tsPerHour > 0))
    {
        return (capXpt / (double)tsPerHour) * resCost->costPerHour();
    }
    else
    {
        return 0.0;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

const MinCostAltResPt*
MinCostHeuristics::getMinCostAltResPt(const JobOp* op) const
{
    jobop_altrespt_map_t::const_iterator opMapIt = _opAltResPt.find(op);
    if (opMapIt == _opAltResPt.end())
    {
        return nullptr;
    }
    else
    {
        return (*opMapIt).second;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

void
MinCostHeuristics::init()
{
    _timeStep = 0;
    _rng = nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

void
MinCostHeuristics::deInit()
{
    deleteMapSecond(_opAltResPt);
    delete _rng;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;
