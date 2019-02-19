#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <libutl/Mutex.h>
#include <gop/Score.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

GOP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Store status information for an optimization run.

   Particularly:
     - completion status
     - current count of iterations
     - iteration that produced the best Score so far
     - the best Score found so far

   \ingroup gop
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class RunStatus : public utl::Object
{
    UTL_CLASS_DECL(RunStatus, utl::Object);
    UTL_CLASS_NO_COPY;

public:
    /**
       Update the status.
       \param complete completion status
       \param currentIter current iteration
       \param bestIter best iteration so far
       \param bestScore best Score so far
    */
    void update(bool complete, uint_t currentIter, uint_t bestIter, Score* bestScore);

    /**
       Get the status.
       \param complete (out) completion status
       \param currentIter (out) current iteration
       \param bestIter (out) best iteration so far
       \param bestScore (out) best Score so far
    */
    void get(bool& complete, uint_t& currentIter, uint_t& bestIter, Score*& bestScore) const;

private:
    void init();
    void deInit();

private:
    bool _complete;
    uint_t _currentIter;
    uint_t _bestIter;
    Score* _bestScore;
    uint_t _scoreType;
    mutable utl::Mutex _mutex;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

GOP_NS_END;
