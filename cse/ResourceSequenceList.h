#ifndef CSE_RESOURCESEQUENCELIST_H
#define CSE_RESOURCESEQUENCELIST_H

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////
// ResourceSequenceRule ////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Associate delay/cost with sequence of operations.

   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class ResourceSequenceRule : public utl::Object
{
    UTL_CLASS_DECL(ResourceSequenceRule);

public:
    /** Constructor. */
    ResourceSequenceRule(utl::uint_t lhsOpSequenceId,
                         utl::uint_t rhsOpSequenceId,
                         utl::uint_t delay,
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
    virtual int compare(const utl::Object& rhs) const;

    virtual void
    serialize(utl::Stream& stream, utl::uint_t io, utl::uint_t mode = utl::ser_default);

    /** Get lhs-op-sequence-id. */
    utl::uint_t
    lhsOpSequenceId() const
    {
        return _lhsOpSequenceId;
    }

    /** Get rhs-op-sequence-id. */
    utl::uint_t
    rhsOpSequenceId() const
    {
        return _rhsOpSequenceId;
    }

    /** Get the delay. */
    utl::uint_t
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
    utl::uint_t _lhsOpSequenceId;
    utl::uint_t _rhsOpSequenceId;
    utl::uint_t _delay;
    double _cost;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

class ResourceSequenceList;

////////////////////////////////////////////////////////////////////////////////////////////////////

typedef std::set<ResourceSequenceRule*, lut::ObjectSTLordering<ResourceSequenceRule>> rsr_set_t;

////////////////////////////////////////////////////////////////////////////////////////////////////
// ResourceSequenceList ////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Delays/costs that depend on sequence of operations scheduled on a resource.

   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class ResourceSequenceList : public utl::Object
{
    UTL_CLASS_DECL(ResourceSequenceList);

public:
    typedef rsr_set_t::const_iterator iterator;

public:
    virtual void copy(const utl::Object& rhs);

    /** Compare with another instance. */
    virtual int compare(const utl::Object& rhs) const;

    /** Test equality with another instance. */
    virtual bool equals(const ResourceSequenceList& rhs) const;

    virtual void
    serialize(utl::Stream& stream, utl::uint_t io, utl::uint_t mode = utl::ser_default);

    /** Get id. */
    utl::uint_t
    id() const
    {
        return _id;
    }

    /** Get the resource-sequence-id. */
    utl::uint_t&
    id()
    {
        return _id;
    }

    /** Find the first record to match the give op-sequence-ids. */
    const ResourceSequenceRule* findRule(utl::uint_t lhsOpSequenceId,
                                         utl::uint_t rhsOpSequenceId) const;

    /** Add a delay/cost for the given sequencing. */
    void
    add(utl::uint_t lhsOpSequenceId, utl::uint_t rhsOpSequenceId, utl::uint_t delay, double cost);

    /** Get begin iterator. */
    iterator
    begin() const
    {
        return _rsl.begin();
    }

    /** Get end iterator. */
    iterator
    end() const
    {
        return _rsl.end();
    }

private:
    void init();
    void deInit();

    utl::uint_t _id;
    rsr_set_t _rsl;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// ResourceSequenceListIdOrdering //////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Order ResourceSequenceList objects by id.

   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

struct ResourceSequenceListOrdering
    : public std::binary_function<ResourceSequenceList*, ResourceSequenceList*, bool>
{
    /** Compare two individuals. */
    bool
    operator()(const ResourceSequenceList* lhs, const ResourceSequenceList* rhs) const
    {
        return (lhs->id() < rhs->id());
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////

typedef std::set<ResourceSequenceList*, ResourceSequenceListOrdering> rsl_set_t;

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
