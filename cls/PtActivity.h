#ifndef CLS_PTACTIVITY_H
#define CLS_PTACTIVITY_H

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <clp/IntVar.h>
#include <cls/Activity.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Activity with processing-time.

   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class PtActivity : public Activity
{
    UTL_CLASS_DECL_ABC(PtActivity);

public:
    /**
       Constructor.
       \param schedule owning schedule
    */
    PtActivity(Schedule* schedule)
        : Activity(schedule)
    {
        init();
    }

    /** Select a processing-time. */
    virtual void
    selectPt(utl::uint_t pt)
    {
        ABORT();
    }

    /** Exclude a processing-time. */
    void excludePt(utl::uint_t pt);

    /** Get frozen-pt. */
    utl::uint_t
    frozenPt() const
    {
        return _frozenPt;
    }

    /** Get frozen-pt. */
    utl::uint_t&
    frozenPt()
    {
        return _frozenPt;
    }

    /** Get possible processing-times. */
    const clp::IntVar&
    possiblePts() const
    {
        return *_possiblePts;
    }

protected:
    utl::uint_t _frozenPt;
    clp::IntVar* _possiblePts;

private:
    void init();
    void deInit();
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_END;

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
