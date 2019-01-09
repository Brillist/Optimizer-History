#ifndef GOP_MULTISTARTSA_H
#define GOP_MULTISTARTSA_H

//////////////////////////////////////////////////////////////////////////////

#include <libutl/NetServer.h>
#include <gop/IndEvaluator.h>
#include <gop/Objective.h>
#include <gop/SAoptimizer.h>
#include <gop/StringScore.h>
#include <gop/Score.h>

//////////////////////////////////////////////////////////////////////////////

GOP_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////

class MultistartSA : public SAoptimizer
{
    UTL_CLASS_DECL(MultistartSA);
public:
/*    virtual bool requiresRevOp() const */
/*       { return true; } */

   /** Initialize. */
   virtual void initialize(const OptimizerConfiguration* config);

   virtual void setAcceptedScore(Score* score);

   virtual void setAcceptedScore(utl::uint_t idx, Score* score);

   virtual  Score* acceptedScores(utl::uint_t idx);

   /** Run the hill-climber. */
   virtual bool run();
   
   /** Audit the result */
   virtual void audit();
private:
   void init();
   void deInit();

   utl::uint_t _beamWidth;
   stringscore_vector_t _strScores;
   uint_score_map_t _acceptedScoresMap;
};

//////////////////////////////////////////////////////////////////////////////

GOP_NS_END;

//////////////////////////////////////////////////////////////////////////////

#endif
