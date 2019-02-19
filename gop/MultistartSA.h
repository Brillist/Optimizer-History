#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <libutl/NetServer.h>
#include <gop/IndEvaluator.h>
#include <gop/Objective.h>
#include <gop/SAoptimizer.h>
#include <gop/StringScore.h>
#include <gop/Score.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

GOP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Multi-start version of SAoptimizer.

   MultistartSA can be thought of as multiple instances of SAoptimizer operating in parallel.

   \ingroup gop
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class MultistartSA : public SAoptimizer
{
    UTL_CLASS_DECL(MultistartSA, SAoptimizer);

public:
    /** Initialize. */
    virtual void initialize(const OptimizerConfiguration* config);

    /** Run the hill-climber. */
    virtual bool run();

    /** Audit the result */
    virtual void audit();

private:
    void init();
    void deInit();
    virtual void setAcceptedScore(Score* score);
    virtual void setAcceptedScore(uint_t idx, Score* score);
    virtual Score* acceptedScores(uint_t idx);

private:
    uint_t _beamWidth;
    stringscore_vector_t _strScores;
    uint_score_map_t _acceptedScoresMap;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

GOP_NS_END;
