#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <clp/IntVar.h>
#include <cls/Activity.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Activity with processing-time.

   \see Activity
   \ingroup cls
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class PtActivity : public Activity
{
    UTL_CLASS_DECL_ABC(PtActivity, Activity);

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

    /// \name Processing-time selection/exclusion
    //@{
    /** Select a processing-time. */
    virtual void selectPt(uint_t pt) = 0;

    /** Exclude a processing-time. */
    void excludePt(uint_t pt);
    //@}

    /// \name Accessors (const)
    //@{
    /** Get frozen processing-time. */
    uint_t
    frozenPt() const
    {
        return _frozenPt;
    }

    /** Get possible processing-times. */
    const clp::IntVar&
    possiblePts() const
    {
        return *_possiblePts;
    }
    //@}

    /// \name Accessors (non-const)
    //@{
    /** Set frozen processing-time. */
    void
    setFrozenPt(uint_t frozenPt)
    {
        _frozenPt = frozenPt;
    }
    //@}

protected:
    uint_t _frozenPt;
    clp::IntVar* _possiblePts;

private:
    void init();
    void deInit();
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_END;
