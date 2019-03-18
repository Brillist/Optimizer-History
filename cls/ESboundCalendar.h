#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <clp/Bound.h>
#include <cls/BrkActivity.h>
#include <cls/EFbound.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Earliest time when a BrkActivity's required DiscreteResource%s are all available for work.

   \see BrkActivity::calendar
   \see ESbound
   \ingroup cls
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class ESboundCalendar : public clp::Bound
{
    UTL_CLASS_DECL(ESboundCalendar, clp::Bound);

public:
    /**
       Constructor.
       \param act related BrkActivity
       \param lb initial lower bound
       \param efBound related EFbound
    */
    ESboundCalendar(BrkActivity* act, int lb, EFbound* efBound);

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
    EFbound* _efBound;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_END;
