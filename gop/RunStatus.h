#ifndef GOP_RUNSTATUS_H
#define GOP_RUNSTATUS_H

//////////////////////////////////////////////////////////////////////////////

#include <libutl/Mutex.h>
#include <gop/Score.h>

//////////////////////////////////////////////////////////////////////////////

GOP_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////

/**
   Time Slot (for serializing timetable).

   \author Adam McKee
*/

//////////////////////////////////////////////////////////////////////////////

class RunStatus : public utl::Object
{
    UTL_CLASS_DECL(RunStatus);
    UTL_CLASS_NO_COPY;
public:
    void update(
        bool complete,
        utl::uint_t currentIter,
        utl::uint_t bestIter,
        Score* bestScore);

    void get(
        bool& complete,
        utl::uint_t& currentIter,
        utl::uint_t& bestIter,
        Score*& bestScore);
private:
    void init();
    void deInit();
private:
    bool _complete;
    utl::uint_t _currentIter;
    utl::uint_t _bestIter;
    Score* _bestScore;
    utl::uint_t _scoreType;
    utl::Mutex _mutex;
};

//////////////////////////////////////////////////////////////////////////////

GOP_NS_END;

//////////////////////////////////////////////////////////////////////////////

#endif
