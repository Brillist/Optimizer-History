#ifndef CLS_TIMETABLEBOUND_H
#define CLS_TIMETABLEBOUND_H

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <clp/ConstrainedBound.h>
#include <clp/IntExp.h>
#include <cls/BrkActivity.h>
#include <cls/DiscreteResource.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   An activity's execution time cannot overlap with any period of
   insufficient available resource capacity.

   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class TimetableBound : public clp::Bound
{
    UTL_CLASS_DECL_ABC(TimetableBound);

public:
    /** Constructor. */
    TimetableBound(
        BrkActivity* act, ResourceCapPts* rcp, const CapPt* capPt, clp::bound_t type, int lb);

    /** Allocate capacity. */
    virtual void allocateCapacity() = 0;

    /** Deallocate capacity. */
    virtual void deallocateCapacity() = 0;

    /** Exclude this capacity. */
    void exclude();

    /** Register for events. */
    void registerEvents(clp::ConstrainedBound* bound);

    /** Deregister for events. */
    void deregisterEvents(clp::ConstrainedBound* bound);

    /// \name Accessors
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
    utl::uint_t
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
    utl::uint_t _ttv0;
    utl::uint_t _ttv1;

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

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
