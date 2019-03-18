#pragma once

/////////////////////////////////////////////////////////////////////////////////////////////////////

#include <cls/ResourceCapPts.h>
#include "ResourceCost.h"
#include "JobOp.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

/////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   MinCostHeuristics is a heuristics designed for the selection of 
   scheduling variables' initial values in order to minimize total cost.
   These scheduling variables are, for example, alternative resources defined \
   in a resource group, or different pt (processing time) defined in ptExp.

   MinCostHeristics can be used by simple foward / backward scheduler, or by
   initial run of all optimizers as an initial start point for optimizatioin.

   -Joe, August 24, 2005.  
*/

/////////////////////////////////////////////////////////////////////////////////////////////////////

class MinCostAltResPt : public utl::Object
{
    UTL_CLASS_DECL(MinCostAltResPt, utl::Object);
    UTL_CLASS_NO_COPY;

public:
    MinCostAltResPt(uint_t pt, uint_vector_t altResIdxs)
    {
        _pt = pt;
        _altResIdxs = altResIdxs;
    }

    /** get pt. */
    const uint_t
    pt() const
    {
        return _pt;
    }

    /** get altResIdxs. */
    const uint_vector_t&
    altResIdxs() const
    {
        return _altResIdxs;
    }

private:
    void
    init()
    {
        _pt = 0;
    }
    void deInit(){};

private:
    uint_t _pt;
    uint_vector_t _altResIdxs;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////

class MinCostHeuristics : public utl::Object
{
    UTL_CLASS_DECL(MinCostHeuristics, utl::Object);
    UTL_CLASS_NO_COPY;

public:
    void initialize(ClevorDataSet* dataSet);

    void setMinCostAltResPt(const JobOp* op);

    const MinCostAltResPt* getMinCostAltResPt(const JobOp* op) const;

    uint_t findCap(const cls::ResourceCapPts* resCapPts, uint_t p_pt);

    double getResCost(uint_t cap, uint_t pt, const ResourceCost* cost);

private:
    typedef std::map<const JobOp*, MinCostAltResPt*, JobOpIdOrdering> jobop_altrespt_map_t;

private:
    void init();
    void deInit();

    lut::rng_t* _rng;
    uint_t _timeStep;
    jobop_altrespt_map_t _opAltResPt;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;
