#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <clp/ConstrainedBound.h>
#include <clp/IntExp.h>
#include <cls/BrkActivity.h>
#include <cls/DiscreteResource.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Earliest (or latest) time when a DiscreteResource has sufficient capacity (abstract).

   ESboundTimetable and LFboundTimetable are concrete specializations of this class for the
   lower- and upper-bound cases, respectively.

   \see ESbound
   \see ESboundTimetable
   \see LFbound
   \see LFboundTimetable
   \ingroup cls
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class TimetableBound : public clp::Bound
{
    UTL_CLASS_DECL_ABC(TimetableBound, clp::Bound);

public:
    /** 
       Constructor.
       \param act activity this bound is computed for
       \param rcp all of the resource's `[capacity,processing-time]` pairings
       \param capPt `[capacity,processing-time]` pair this bound is computed for
       \param type bound type (lower or upper)
       \param bound initial bound value
    */
    TimetableBound(
        BrkActivity* act, ResourceCapPts* rcp, const CapPt* capPt, clp::bound_t type, int bound);

    /** Exclude this `[capacity,processing-time]` pair. */
    void exclude();

    /// \name Capacity Allocation
    //@{
    /** Allocate capacity. */
    virtual void allocateCapacity() = 0;

    /** Deallocate capacity. */
    virtual void deallocateCapacity() = 0;
    //@}

    /// \name Events
    //@{
    /** Register the provided bound for events. */
    void registerEvents(clp::ConstrainedBound* bound);

    /** Deregister the provided bound for events. */
    void deregisterEvents(clp::ConstrainedBound* bound);
    //@}

    /// \name Accessors (const)
    //@{
    /** Possible? */
    bool
    possible() const
    {
        return _possible;
    }

    /** Get the manager. */
    clp::Manager*
    manager() const
    {
        return _act->manager();
    }

    /** Get activity. */
    BrkActivity*
    activity() const
    {
        return _act;
    }

    /** Get capacity. */
    uint_t
    capacity() const
    {
        ASSERTD(_capPt != nullptr);
        return _capPt->capacity();
    }
    //@}
protected:
    bool _possible;
    BrkActivity* _act;
    DiscreteResource* _res;
    DiscreteTimetable* _timetable;
    ResourceCapPts* _rcp;
    const CapPt* _capPt;
    clp::IntExp* _capExp;
    const clp::RevIntSpanCol* _tt;
    uint_t _ttv0;
    uint_t _ttv1;

private:
    void
    init()
    {
        ABORT();
    }

    void
    deInit()
    {
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_END;
