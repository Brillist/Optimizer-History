#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <gop/SAoptimizer.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

GOP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/** 
    ID_SA (Iterative Deepening Simulated Annealing) originally designed by Yixin Chen. 

    Chen, Yixin, "Optimal Anytime Search For Constrained Nonlinear Programming", Thesis for the
    degree of Master of Science, University of Illinois at Urbana-Champaign, 2001.

    This implementation contains some modifications.
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class ID_SAoptimizer : public SAoptimizer
{
    UTL_CLASS_DECL(ID_SAoptimizer, SAoptimizer);

public:
    /** Initialize. */
    virtual void initialize(const OptimizerConfiguration* config);

    /** Run the ID_SA. */
    virtual bool run();

    /** dump a string of init temperature parameters. */
    utl::String ID_SAinitTempString();
    utl::String ID_SAinitTempString2();

protected:
    double _acceptanceRatio;
    double _ratioDcrRate;

    Score* _targetScore;
    double _scoreStep;

    uint_t _initNumProbes;
    uint_t _numProbes;

    uint_t _numRepeats;
    uint_t _repeatId;

private:
    void init();
    void
    deInit()
    {
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////

GOP_NS_END;
