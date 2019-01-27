#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <gop/SAoptimizer.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

GOP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/** 
    AR_SA (Automatic Re-annealing Simulated Annealing) was designed by 
    Joe Zhou, Clevor Technologies Inc., Canada, in 2005. 

    Joe Zhou
    Dec. 2005
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class AR_SAoptimizer : public SAoptimizer
{
    UTL_CLASS_DECL(AR_SAoptimizer, SAoptimizer);

public:
    /** Initialize. */
    virtual void initialize(const OptimizerConfiguration* config);

    /** Run the AR_SA. */
    virtual bool run();

    /** Dump a string of init temperature parameters. */
    utl::String AR_SAinitTempString();

protected:
    double _acceptanceRatio;
    double _ratioDcrRate;

    uint_t _idleIterGap;
    double _gapIncRate;

private:
    void init();
    void
    deInit()
    {
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////

GOP_NS_END;
