#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

/* #include <libutl/Object.h> */
/* #include <cse/PrecedenceCt.h> */
/* #include <mrp/ProcessStep.h> */

////////////////////////////////////////////////////////////////////////////////////////////////////

MRP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

class Setup;

////////////////////////////////////////////////////////////////////////////////////////////////////

struct SetupOrderingIncId : public std::binary_function<Setup*, Setup*, bool>
{
    bool operator()(Setup* lhs, Setup* rhs) const;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

typedef std::set<Setup*, SetupOrderingIncId> setup_set_id_t;
typedef std::vector<uint_t> uint_vector_t;
typedef std::vector<double> double_vector_t;
typedef std::map<uint_t, uint_t> uint_uint_map_t;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Setup class.

   \author Joe Zhou
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class Setup : public utl::Object
{
    UTL_CLASS_DECL(Setup, utl::Object);

public:
    /** Constructor. */
    Setup(uint_t id)
    {
        _id = id;
    }

    virtual void copy(const utl::Object& rhs);

    virtual void
    serialize(utl::Stream& stream, uint_t io, uint_t mode = utl::ser_default);

    /// \name Accessors
    //@{
    /** Id. */
    uint_t
    id() const
    {
        return _id;
    }

    /** Id. */
    uint_t&
    id()
    {
        return _id;
    }
    //@}

    /** get prevSetupIds' idx. */
    uint_t getPrevSetupIdx(uint_t prevSetupId) const;

    /** get setup time. */
    uint_t
    getSetupTime(uint_t idx) const
    {
        return _setupTimes[idx];
    }

    /** get setup cost. */
    double
    getSetupCost(uint_t idx) const
    {
        return _setupCosts[idx];
    }

    String toString() const;

private:
    void
    init()
    {
    }
    void
    deInit()
    {
    }

private:
    uint_t _id;
    uint_uint_map_t _prevSetupIdsMap; //preSetupId - vectorIdx mapping
    uint_vector_t _setupTimes;
    double_vector_t _setupCosts;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

MRP_NS_END;
