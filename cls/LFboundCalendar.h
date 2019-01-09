#ifndef CLS_LFBOUNDCALENDAR_H
#define CLS_LFBOUNDCALENDAR_H

//////////////////////////////////////////////////////////////////////////////

#include <clp/Bound.h>
#include <cls/BrkActivity.h>
#include <cls/LSbound.h>

//////////////////////////////////////////////////////////////////////////////

CLS_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////

/**
   An activity cannot start or end execution during a break
   in resource availability.

   \author Adam McKee
*/

//////////////////////////////////////////////////////////////////////////////

class LFboundCalendar : public clp::Bound
{
    UTL_CLASS_DECL(LFboundCalendar);
public:
    /** Constructor. */
    LFboundCalendar(BrkActivity* act, int ub);

    /** Get activity. */
    BrkActivity* activity() const
    { return _act; }
protected:
    virtual int find();
private:
    void init()
    { ABORT(); }

    void deInit() {}
private:
    BrkActivity* _act;
    LSbound* _lsBound;
};

//////////////////////////////////////////////////////////////////////////////

CLS_NS_END;

//////////////////////////////////////////////////////////////////////////////

#endif
