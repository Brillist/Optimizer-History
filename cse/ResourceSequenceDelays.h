#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////
// ResourceSequenceDelay ///////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Associate delay/cost with operation sequence.

   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class ResourceSequenceDelay : public utl::Object
{
    UTL_CLASS_DECL(ResourceSequenceDelay, utl::Object);

public:
    /** Constructor. */
    ResourceSequenceDelay(uint_t lhsOpSequenceId,
                          uint_t rhsOpSequenceId,
                          int delay,
                          double cost)
    {
        _lhsOpSequenceId = lhsOpSequenceId;
        _rhsOpSequenceId = rhsOpSequenceId;
        _delay = delay;
        _cost = cost;
    }

    /** Copy another instance. */
    virtual void copy(const utl::Object& rhs);

    /** Compare with another instance. */
    int compare(const utl::Object& rhs) const;

    /** Get lhs-op-sequence-id. */
    uint_t
    lhsOpSequenceId() const
    {
        return _lhsOpSequenceId;
    }

    /** Get rhs-op-sequence-id. */
    uint_t
    rhsOpSequenceId() const
    {
        return _rhsOpSequenceId;
    }

    /** Get the delay. */
    int
    delay() const
    {
        return _delay;
    }

    /** Get the cost. */
    double
    cost() const
    {
        return _cost;
    }

private:
    void init();
    void
    deInit()
    {
    }

private:
    uint_t _lhsOpSequenceId;
    uint_t _rhsOpSequenceId;
    int _delay;
    double _cost;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// ResourceSequenceDelays //////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Delays/costs that depend on sequence of operations scheduled on a resource.

   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class ResourceSequenceDelays : public utl::Object
{
    UTL_CLASS_DECL(ResourceSequenceDelays, utl::Object);

public:
    virtual void copy(const utl::Object& rhs);

    /** Compare with another instance. */
    int compare(const utl::Object& rhs) const;

    /** Add a delay/cost for the given sequencing. */
    void add(uint_t lhsOpSequenceId, uint_t rhsOpSequenceId, int delay, double cost);

private:
    typedef std::set<ResourceSequenceDelay*, lut::ObjectSTLordering<ResourceSequenceDelay>>
        rsd_set_t;

private:
    void init();
    void deInit();

private:
    uint_t _resourceSequenceId;
    rsd_set_t _resourceSequenceDelays;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;
