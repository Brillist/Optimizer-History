#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

LUT_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Time period.
   \ingroup lut
*/
enum period_t
{
    period_hour = 0,  /**< one hour */
    period_day = 1,   /**< one day */
    period_week = 2,  /**< one week */
    period_month = 3, /**< one month */
    period_year = 4,  /**< one year */
    period_undefined
};

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   A \c std::set of uint_t values.
   \ingroup lut
*/
using uint_set_t = std::set<uint_t>;

/**
   A \c std::vector of uint_t values.
   \ingroup lut
*/
using uint_vect_t = std::vector<uint_t>;

/**
   A \c std::set of int values.
   \ingroup lut
*/
using int_set_t = std::set<int>;

/**
   A \c std::vector of int values.
   \ingroup lut
*/
using int_vect_t = std::vector<int>;

/**
   A \c randutils::random_generator based on \c std::mt19937.
   \ingroup lut
*/
using rng_32_t = randutils::random_generator<std::mt19937>;

/**
   A \c randutils::random_generator based on \c std::mt19937_64.
   \ingroup lut
*/
using rng_64_t = randutils::random_generator<std::mt19937_64>;

/**
   A \c randutils::random_generator based on \c std::mt19937.
*/
using rng_t = rng_32_t;

////////////////////////////////////////////////////////////////////////////////////////////////////

LUT_NS_END;
