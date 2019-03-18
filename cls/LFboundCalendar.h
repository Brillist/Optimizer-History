#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <clp/Bound.h>
#include <cls/BrkActivity.h>
#include <cls/LSbound.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Latest time when a BrkActivity's required DiscreteResource%s are all available for work.

   LFboundCalendar is a mirror of the ESboundCalendar class for backward scheduling.

   \see BrkActivity::calendar
   \see LFbound
   \ingroup cls
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class LFboundCalendar : public clp::Bound
{
    UTL_CLASS_DECL(LFboundCalendar, clp::Bound);

public:
    /**
       Constructor.
       \param act related BrkActivity
       \param ub initial upper bound
    */
    LFboundCalendar(BrkActivity* act, int ub);

    /** Get activity. */
    BrkActivity*
    activity() const
    {
        return _act;
    }

protected:
    virtual int find();

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

private:
    BrkActivity* _act;
    LSbound* _lsBound;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_END;
