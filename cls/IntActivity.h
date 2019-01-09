#ifndef CLS_INTACTIVITY_H
#define CLS_INTACTIVITY_H

//////////////////////////////////////////////////////////////////////////////

#include <libutl/RBtree.h>
#include <clp/Bound.h>
#include <cls/PtActivity.h>
#include <clp/RevArray.h>

//////////////////////////////////////////////////////////////////////////////

CLS_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////

class CompositeResourceRequirement;

//////////////////////////////////////////////////////////////////////////////

/**
   Interruptible activity.

   An interruptible activity is an activity whose execution can be
   interrupted by insufficient resource capacity.

   \author Adam McKee
*/

//////////////////////////////////////////////////////////////////////////////

class IntActivity : public Activity
{
    friend class CompositeResourceRequirement;
    UTL_CLASS_DECL(IntActivity);
public:
    typedef clp::RevArray<utl::uint_t> revarray_t;
public:
    /**
       Constructor.
       \param schedule owning schedule
    */
    IntActivity(Schedule* schedule)
        : Activity(schedule)
    { init(); }

    /** Add a requirement. */
    void add(CompositeResourceRequirement* crr);

    /** Initialize requirements. */
    void initRequirements();

    /** Get processing-time. */
    utl::uint_t processingTime() const
    { return _processingTime; }

    /** Get processing-time. */
    utl::uint_t& processingTime()
    { return _processingTime; }

    /** Get the break-list. */
    virtual clp::IntExp* breakList() const;

    /** Get break list. */
    clp::IntExp*& breakList()
    { return _breakList; }

    /** Get requirements. */
    const utl::RBtree& requirements() const
    { return _compositeReqs; }

    /** Add an allocation. */
    void addAllocation(utl::uint_t resId, utl::uint_t min, utl::uint_t max);

    /** Get allocations array. */
    void getAllocations(revarray_t**& allocations, utl::uint_t& size) const
    { allocations = _allocations; size = _allocationsSize; }

    /** Deallocate. */
    void deallocate();

    /** Forward scheduling? */
    bool forward() const;

    /** Backward scheduling? */
    bool backward() const
    { return !forward(); }

    /** Select a resource. */
    virtual bool selectResource(utl::uint_t resId);
private:
    void init();
    void deInit();
private:
    typedef std::set<utl::uint_t> uint_set_t;
    typedef std::vector<utl::uint_t> uint_vector_t;
private:
    utl::uint_t _processingTime;
    clp::IntExp* _breakList;
    revarray_t** _allocations;
    size_t _allocationsSize;
    utl::RBtree _compositeReqs;
};

//////////////////////////////////////////////////////////////////////////////

CLS_NS_END;

//////////////////////////////////////////////////////////////////////////////

#endif
