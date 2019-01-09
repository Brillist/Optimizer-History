#ifndef GOP_MULTISTARTHC_H
#define GOP_MULTISTARTHC_H

//////////////////////////////////////////////////////////////////////////////

#include <libutl/NetServer.h>
#include <gop/IndEvaluator.h>
#include <gop/Objective.h>
#include <gop/HillClimber.h>
#include <gop/StringScore.h>
#include <gop/String.h>

//////////////////////////////////////////////////////////////////////////////

GOP_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////

class MultistartHC : public HillClimber
{
    UTL_CLASS_DECL(MultistartHC);
public:

    /** Initialize. */
    virtual void initialize(const OptimizerConfiguration* config);

   /** Run the multiple_start_hill-climber. */
   virtual bool run();
   
    /** Audit the result */
    virtual void audit();
private:
    void init();
    void deInit();
private:
    utl::uint_t _beamWidth;
    stringscore_vector_t _strScores;
};

//////////////////////////////////////////////////////////////////////////////

GOP_NS_END;

//////////////////////////////////////////////////////////////////////////////

#endif
