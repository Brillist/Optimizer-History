#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <cse/ScheduleEvaluatorConfiguration.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Total cost evaluator configuration.

   TotalCostEvaluatorConfiguration stores configuration parameters
   for TotalCostEvaluator.

   \see TotalCostEvaluator
   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class TotalCostEvaluatorConfiguration : public ScheduleEvaluatorConfiguration
{
    UTL_CLASS_DECL(TotalCostEvaluatorConfiguration, ScheduleEvaluatorConfiguration);

public:
    /** Copy another instance. */
    virtual void copy(const utl::Object& rhs);

    virtual void
    serialize(utl::Stream& stream, uint_t io, uint_t mode = utl::ser_default);

    /// \name Accessors
    //@{
    /** Get the interest rate. */
    double
    interestRate() const
    {
        return _interestRate;
    }

    /** Set the interest rate. */
    double&
    interestRate()
    {
        return _interestRate;
    }

    /** Get the interest compounding period. */
    lut::period_t
    interestRatePeriod() const
    {
        return _interestRatePeriod;
    }

    /** Set the interest compounding period. */
    lut::period_t&
    interestRatePeriod()
    {
        return _interestRatePeriod;
    }

    /** Get the overhead cost. */
    double
    overheadCost() const
    {
        return _overheadCost;
    }

    /** Get the overhead cost. */
    double&
    overheadCost()
    {
        return _overheadCost;
    }

    /** Get the overhead cost period. */
    lut::period_t
    overheadCostPeriod() const
    {
        return _overheadCostPeriod;
    }

    /** Get the overhead cost period. */
    lut::period_t&
    overheadCostPeriod()
    {
        return _overheadCostPeriod;
    }
    //@}
private:
    void init();
    void
    deInit()
    {
    }

private:
    double _interestRate;
    lut::period_t _interestRatePeriod;
    double _overheadCost;
    lut::period_t _overheadCostPeriod;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;
