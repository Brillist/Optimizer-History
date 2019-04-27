#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <cse/ScheduleEvaluatorConfiguration.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Configuration parameters for TotalCostEvaluator.

   \ingroup cse
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class TotalCostEvaluatorConfiguration : public ScheduleEvaluatorConfiguration
{
    UTL_CLASS_DECL(TotalCostEvaluatorConfiguration, ScheduleEvaluatorConfiguration);

public:
    virtual void copy(const utl::Object& rhs);

    virtual void serialize(utl::Stream& stream, uint_t io, uint_t mode = utl::ser_default);

    /// \name Accessors (const)
    //@{
    /** Get the interest rate. */
    double
    interestRate() const
    {
        return _interestRate;
    }

    /** Get the interest compounding period. */
    lut::period_t
    interestRatePeriod() const
    {
        return _interestRatePeriod;
    }

    /** Get the overhead cost. */
    double
    overheadCost() const
    {
        return _overheadCost;
    }

    /** Get the overhead cost period. */
    lut::period_t
    overheadCostPeriod() const
    {
        return _overheadCostPeriod;
    }
    //@}

    /// \name Accessors (non-const)
    //@{
    /** Set the interest rate. */
    void
    setInterestRate(double interestRate)
    {
        _interestRate = interestRate;
    }

    /** Set the interest compounding period. */
    void
    setInterestRatePeriod(lut::period_t interestRatePeriod)
    {
        _interestRatePeriod = interestRatePeriod;
    }

    /** Set the overhead cost. */
    void
    setOverheadCost(double overheadCost)
    {
        _overheadCost = overheadCost;
    }

    /** Set the overhead cost period. */
    void
    setOverheadCostPeriod(lut::period_t overheadCostPeriod)
    {
        _overheadCostPeriod = overheadCostPeriod;
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
