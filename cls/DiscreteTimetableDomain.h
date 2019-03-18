#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <clp/IntExp.h>
#include <clp/RevIntSpanCol.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   DiscreteTimetable domain.

   DiscreteTimetableDomain specializes RevIntSpanCol to record required and provided capacity
   in each time slot.

   \ingroup cls
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class DiscreteTimetableDomain : public utl::Object, public clp::RevIntSpanCol
{
    UTL_CLASS_DECL(DiscreteTimetableDomain, utl::Object);
    UTL_CLASS_NO_COPY;

public:
    /**
       Constructor.
       \param mgr related Manager
    */
    DiscreteTimetableDomain(clp::Manager* mgr)
    {
        init();
        initialize(mgr);
    }

    /**
       Initialize.
       \param mgr related Manager
    */
    void initialize(clp::Manager* mgr);

    /**
       Add required and/or provided capacity over a time span.
       \param min start of affected span
       \param max end of affected span
       \param reqCap addition to required capacity (may be negative)
       \param prvCap addition to provided capacity (may be negative)
       \return minimum available capacity in the time span
    */
    uint_t add(int min, int max, int reqCap, int prvCap);

    /// \name Capacity Expressions
    //@{
    /** Add capacity-expression. */
    clp::IntExp* addCapExp(uint_t cap);

    /** Remove capacity-expression. */
    void remCapExp(uint_t cap);
    //@}

    /// \name Events
    //@{
    /** Any event? */
    bool
    anyEvent() const
    {
        return (_events != 0);
    }

    /** Clear event flags. */
    void
    clearEvents()
    {
        _events = 0;
    }

    /** Empty event? */
    bool
    emptyEvent() const
    {
        return ((_events & ef_empty) != 0);
    }

    /** Range event? */
    bool
    rangeEvent() const
    {
        return ((_events & ef_range) != 0);
    }
    //@}
protected:
    enum event_flag_t
    {
        ef_empty = 1,
        ef_range = 2
    };

private:
    void init();
    void deInit();

private:
    clp::IntExp** _capExps;
    uint_t* _capExpCounts;
    size_t _capExpsSize;
    size_t _capExpCountsSize;
    uint_t _events;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_END;
