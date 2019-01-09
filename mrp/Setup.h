#ifndef MRP_SETUP_H
#define MRP_SETUP_H

//////////////////////////////////////////////////////////////////////////////

/* #include <libutl/Object.h> */
/* #include <cse/PrecedenceCt.h> */
/* #include <mrp/ProcessStep.h> */

//////////////////////////////////////////////////////////////////////////////

MRP_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////

class Setup;

//////////////////////////////////////////////////////////////////////////////

struct SetupOrderingIncId : public std::binary_function<Setup*,Setup*,bool>
{
    bool operator()(Setup* lhs, Setup* rhs) const;
};

//////////////////////////////////////////////////////////////////////////////

typedef std::set<Setup*, SetupOrderingIncId> setup_set_id_t;
typedef std::vector<utl::uint_t> uint_vector_t;
typedef std::vector<double> double_vector_t;
typedef std::map<utl::uint_t, utl::uint_t> uint_uint_map_t;

//////////////////////////////////////////////////////////////////////////////

/**
   Setup class.

   \author Joe Zhou
*/

//////////////////////////////////////////////////////////////////////////////

class Setup : public utl::Object
{
    UTL_CLASS_DECL(Setup);
public:
    /** Constructor. */
    Setup(utl::uint_t id)
    { _id = id; }

    virtual void copy(const utl::Object& rhs);

    virtual void serialize(
        utl::Stream& stream,
        utl::uint_t io,
        utl::uint_t mode = utl::ser_default);

    /// \name Accessors
    //@{
    /** Id. */
    utl::uint_t id() const
    { return _id;}

    /** Id. */
    utl::uint_t& id()
    { return _id; }
    //@}

    /** get prevSetupIds' idx. */
    utl::uint_t getPrevSetupIdx(utl::uint_t prevSetupId) const;

    /** get setup time. */
    utl::uint_t getSetupTime(utl::uint_t idx) const
    { return _setupTimes[idx]; }

    /** get setup cost. */
    double getSetupCost(utl::uint_t idx) const
    { return _setupCosts[idx]; }

    utl::String toString() const;
private:
    void init() {}
    void deInit() {}
private:
    utl::uint_t _id;
    uint_uint_map_t _prevSetupIdsMap; //preSetupId - vectorIdx mapping
    uint_vector_t _setupTimes;
    double_vector_t _setupCosts;
};

//////////////////////////////////////////////////////////////////////////////

MRP_NS_END;

//////////////////////////////////////////////////////////////////////////////

#endif
