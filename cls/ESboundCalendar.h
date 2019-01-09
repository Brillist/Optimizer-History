#ifndef CLS_ESBOUNDCALENDAR_H
#define CLS_ESBOUNDCALENDAR_H

//////////////////////////////////////////////////////////////////////////////

#include <clp/Bound.h>
#include <cls/BrkActivity.h>
#include <cls/EFbound.h>

//////////////////////////////////////////////////////////////////////////////

CLS_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////

/**
   An activity cannot start or end execution during a break
   in resource availability.

   \author Adam McKee
*/

//////////////////////////////////////////////////////////////////////////////

class ESboundCalendar : public clp::Bound
{
    UTL_CLASS_DECL(ESboundCalendar);
public:
    /** Constructor. */
    ESboundCalendar(BrkActivity* act, int lb, EFbound* efBound);

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
    EFbound* _efBound;
};

//////////////////////////////////////////////////////////////////////////////

CLS_NS_END;

//////////////////////////////////////////////////////////////////////////////

#endif
