#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <libutl/RBtree.h>
#include <clp/Bound.h>
#include <cls/PtActivity.h>
#include <clp/RevArray.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

class CompositeResourceRequirement;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Interruptible activity.

   An IntActivity requires a specified minimum of capacity in each of its required
   CompositeResource%s to execute, and its execution can be paused during periods of insufficient
   available capacity.

   In each time slot where an IntActivity executes:
   
   - the capacity used in each required CompositeResource is at least the minimum specified
     and no more than the maximum specified (CompositeResourceRequirement::minCapacity,
     CompositeResourceRequirement::maxCapacity)
   - the total of capacities used in all required CompositeResource%s is counted against
     the required processing time (\ref processingTime)

   An IntActivity has a "break list" (see \ref breakList) that specifies when all of its required
   DiscreteResource%s are available to work and have at least the minimum required capacity.
   If only one CompositeResourceRequirement is present, the break list is obtained from that
   resource's timetable by calling CompositeTimetable::addCapExp to create/use a "capacity
   expression" that specifies when the resource has the required capacity.  In the multiple
   CompositeResourceRequirement%s case, CapExpMgr::add is called to combine the availability of
   all required CompositeResource%s into one expression (IntVar) that specifies the time slots
   when the activity can execute on *all* of its CompositeResource%s (computed as an intersection
   of "capacity expression" domains).
   
   \see CapExpMgr
   \see CompositeResource
   \see CompositeResourceRequirement
   \see ESboundInt
   \ingroup cls
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class IntActivity : public Activity
{
    friend class CompositeResourceRequirement;
    UTL_CLASS_DECL(IntActivity, Activity);

public:
    using revarray_uint_t = clp::RevArray<uint_t>;

public:
    /**
       Constructor.
       \param schedule owning schedule
    */
    IntActivity(Schedule* schedule)
        : Activity(schedule)
    {
        init();
    }

    /** Select a resource. */
    virtual bool selectResource(uint_t resId);

    /// \name Resource Requirements
    //@{
    /** Add a resource requirement. */
    void add(CompositeResourceRequirement* crr);

    /** Initialize requirements. */
    void initRequirements();
    //@}

    /// \name Capacity Allocation
    //@{
    /** Add an allocation. */
    void addAllocation(uint_t resId, uint_t min, uint_t max);

    /** Deallocate. */
    void deallocate();
    //@}

    /// \name Accessors (const)
    //@{
    /** Forward scheduling? */
    bool forward() const;

    /** Backward scheduling? */
    bool
    backward() const
    {
        return !forward();
    }

    /** Get processing-time. */
    uint_t
    processingTime() const
    {
        return _processingTime;
    }

    /** Get requirements. */
    const utl::RBtree&
    requirements() const
    {
        return _compositeReqs;
    }

    /** Get the break-list. */
    virtual clp::IntExp* breakList() const;

    /** Get allocations array. */
    void
    getAllocations(revarray_uint_t**& allocations, uint_t& size) const
    {
        allocations = _allocations;
        size = _allocationsSize;
    }
    //@}

    /// \name Accessors (non-const)
    //@{
    /** Set processing-time. */
    void
    setProcessingTime(uint_t pt)
    {
        _processingTime = pt;
    }
    //@}

private:
    void init();
    void deInit();

private:
    uint_t _processingTime;
    clp::IntExp* _breakList;
    revarray_uint_t** _allocations;
    size_t _allocationsSize;
    utl::RBtree _compositeReqs;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_END;
