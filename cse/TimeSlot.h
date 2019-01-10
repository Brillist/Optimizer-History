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
    UTL_CLASS_DECL(TimeSlot);
    UTL_CLASS_DEFID;

public:
    /**
       Constructor.
       \param begin begin time
       \param end end time
       \param reqCap required capacity
       \param prvCap provided capacity
    */
    TimeSlot(time_t begin, time_t end, utl::uint_t reqCap, utl::uint_t prvCap)
    {
        _begin = begin;
        _end = end;
        _reqCap = reqCap;
        _prvCap = prvCap;
    }

    /** Copy another instance. */
    virtual void copy(const utl::Object& rhs);

    virtual void
    serialize(utl::Stream& stream, utl::uint_t io, utl::uint_t mode = utl::ser_default);

private:
    time_t _begin;
    time_t _end;
    utl::uint_t _reqCap;
    utl::uint_t _prvCap;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
