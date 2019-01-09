#include "libgop.h"
#include "RunStatus.h"

//////////////////////////////////////////////////////////////////////////////

#undef new
#include <iomanip>
#include <libutl/gblnew_macros.h>

//////////////////////////////////////////////////////////////////////////////

#ifdef DEBUG
// #define DEBUG_UNIT
#endif

//////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;

//////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(gop::RunStatus, utl::Object);

//////////////////////////////////////////////////////////////////////////////

GOP_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////

// void RunStatus::setBestScore(Score* bestScore)
// {
//     delete _bestScore;
//     _bestScore = bestScore;
// }

//////////////////////////////////////////////////////////////////////////////

void
RunStatus::update(
    bool complete,
    uint_t currentIter,
    uint_t bestIter,
    Score* bestScore)
{
    _mutex.lock();
#ifdef DEBUG_UNIT
    if (_bestScore->compare(*bestScore) != 0)
    {
        std::cout << "RunStatus||currentIter " << currentIter
                  << "||bestIter " << bestIter
                  << "||bestScore " << std::setiosflags(std::ios::fixed)
                  << std::setprecision(0) << bestScore->getValue()
                  << "||resultType " << bestScore->getType() << std::endl;
    }
#endif
    _complete = complete;
    _currentIter = currentIter;
    _bestIter = bestIter;
    _bestScore->copy(*bestScore);
    _mutex.unlock();
}

//////////////////////////////////////////////////////////////////////////////

void
RunStatus::get(
    bool& complete,
    uint_t& currentIter,
    uint_t& bestIter,
    Score*& bestScore)
{
    _mutex.lock();
    complete = _complete;
    currentIter = _currentIter;
    bestIter = _bestIter;
    bestScore = _bestScore;
    _mutex.unlock();
}

//////////////////////////////////////////////////////////////////////////////

void
RunStatus::init()
{
    _complete = false;
    _currentIter = _bestIter = uint_t_max;
    _bestScore = new Score();
}

//////////////////////////////////////////////////////////////////////////////

void
RunStatus::deInit()
{
    delete _bestScore;
}

//////////////////////////////////////////////////////////////////////////////

GOP_NS_END;
