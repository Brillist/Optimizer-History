#ifndef CSE_TIMESLOT_H
#define CSE_TIMESLOT_H

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Time Slot (for serializing timetable).

   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class TimeSlot : public utl::Object
{
    UTL_CLASS_DECL(TimeSlot, utl::Object);
    UTL_CLASS_DEFID;

public:
    /**
       Constructor.
       \param begin begin time
       \param end end time
       \param reqCap required capacity
       \param prvCap provided capacity
    */
    TimeSlot(time_t begin, time_t end, uint_t reqCap, uint_t prvCap)
    {
        _begin = begin;
        _end = end;
        _reqCap = reqCap;
        _prvCap = prvCap;
    }

    /** Copy another instance. */
    virtual void copy(const utl::Object& rhs);

    virtual void
    serialize(utl::Stream& stream, uint_t io, uint_t mode = utl::ser_default);

private:
    time_t _begin;
    time_t _end;
    uint_t _reqCap;
    uint_t _prvCap;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
