#include "libcse.h"
#include "TotalCostEvaluatorConfiguration.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;
GOP_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(cse::TotalCostEvaluatorConfiguration);

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

void
TotalCostEvaluatorConfiguration::copy(const Object& rhs)
{
    auto& cf = utl::cast<TotalCostEvaluatorConfiguration>(rhs);
    super::copy(cf);
    _interestRate = cf._interestRate;
    _interestRatePeriod = cf._interestRatePeriod;
    _overheadCost = cf._overheadCost;
    _overheadCostPeriod = cf._overheadCostPeriod;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
TotalCostEvaluatorConfiguration::serialize(Stream& stream, uint_t io, uint_t)
{
    super::serialize(stream, io);
    utl::serialize(_interestRate, stream, io);
    utl::serialize((uint_t&)_interestRatePeriod, stream, io);
    utl::serialize(_overheadCost, stream, io);
    utl::serialize((uint_t&)_overheadCostPeriod, stream, io);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
TotalCostEvaluatorConfiguration::init()
{
    _interestRate = 0.0;
    _interestRatePeriod = period_undefined;
    _overheadCost = 0.0;
    _overheadCostPeriod = period_undefined;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;
