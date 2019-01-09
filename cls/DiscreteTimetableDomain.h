#ifndef CLS_DISCRETETIMETABLEDOMAIN_H
#define CLS_DISCRETETIMETABLEDOMAIN_H

//////////////////////////////////////////////////////////////////////////////

#include <clp/IntExp.h>
#include <clp/RevIntSpanCol.h>

//////////////////////////////////////////////////////////////////////////////

CLS_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////

/**
   DiscreteTimetable representation for discrete resources.

   \author Adam McKee
*/

//////////////////////////////////////////////////////////////////////////////

class DiscreteTimetableDomain : public utl::Object, public clp::RevIntSpanCol
{
    UTL_CLASS_DECL(DiscreteTimetableDomain);
    UTL_CLASS_NO_COPY;
public:
    /** Constructor. */
    DiscreteTimetableDomain(clp::Manager* mgr)
    { init(); initialize(mgr); }

    /** Set the manager. */
    void initialize(clp::Manager* mgr);

    /** Add capacity-expression. */
    clp::IntExp* addCapExp(utl::uint_t cap);

    /** Remove capacity-expression. */
    void remCapExp(utl::uint_t cap);

    /** Add required/provided capacity over a given interval. */
    utl::uint_t add(int min, int max, int reqCap, int prvCap);

    /// \name Events
    //@{
    /** Any event? */
    bool anyEvent() const
    { return (_events != 0); }

    /** Clear event flags. */
    void clearEvents()
    { _events = 0; }

    /** Empty event? */
    bool emptyEvent() const
    { return ((_events & ef_empty) != 0); }

    /** Range event? */
    bool rangeEvent() const
    { return ((_events & ef_range) != 0); }
    //@}
protected:
    enum event_flag_t {
        ef_empty = 1,
        ef_range = 2
    };
private:
    void init();
    void deInit();
private:
    clp::IntExp** _capExps;
    utl::uint_t* _capExpCounts;
    size_t _capExpsSize;
    size_t _capExpCountsSize;
    utl::uint_t _events;
};

//////////////////////////////////////////////////////////////////////////////

CLS_NS_END;

//////////////////////////////////////////////////////////////////////////////

#endif
