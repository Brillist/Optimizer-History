#include "libcls.h"
#include "ResourceCalendar.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;
CLP_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(cls::ResourceCalendarSpec);
UTL_CLASS_IMPL(cls::ResourceCalendar);

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// ResourceCalendarSpec ///////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

ResourceCalendarSpec::ResourceCalendarSpec(rescal_t type, const uint_set_t& calIds)
{
    _type = type;
    _calIds = calIds;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ResourceCalendarSpec::copy(const Object& rhs)
{
    auto& rcs = utl::cast<ResourceCalendarSpec>(rhs);
    _type = rcs._type;
    _calIds = rcs._calIds;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool
ResourceCalendarSpec::operator<(const ResourceCalendarSpec& rhs) const
{
    int res = lut::compare(_type, rhs._type);
    if (res != 0)
        return (res < 0);
    res = compareItems(_calIds, rhs._calIds);
    return (res < 0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// ResourceCalendar ///////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

int
ResourceCalendar::compare(const Object& rhs) const
{
    auto& cal = utl::cast<ResourceCalendar>(rhs);
    return super::compare(cal);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ResourceCalendar::serialize(Stream& stream, uint_t io, uint_t mode)
{
    super::serialize(stream, io, mode);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool
ResourceCalendar::isMergeable(const Object& lhs, const Object& rhs) const
{
    auto& lhsRCS = utl::cast<ResourceCalendarSpan>(lhs);
    auto& rhsRCS = utl::cast<ResourceCalendarSpan>(rhs);
    return (lhsRCS.status() == rhsRCS.status()) && (lhsRCS.capacity() == rhsRCS.capacity());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ResourceCalendar::dump(Stream& os, time_t originTime, uint_t timeStep)
{
    for (auto rcs_ : self)
    {
        auto rcs = utl::cast<ResourceCalendarSpan>(rcs_);
        os << rcs->toString(originTime, timeStep) << endl;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ResourceCalendar::addBreak(int begin, int end)
{
    auto breakSpan = new ResourceCalendarSpan(begin, end, rcs_exception, rcss_onBreak);
    add(breakSpan);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ResourceCalendar::makeBreakList(Manager* mgr)
{
    ASSERTD(_breakList == nullptr);
    _breakList = new IntVar(mgr);
    _breakList->setFailOnEmpty(false);
    _breakList->remove(int_t_min, -1);
    int horizonTS = int_t_min;
    for (auto it = _spans; it != _spansLim; ++it)
    {
        auto rcs = *it;
        if (rcs->status() == rcss_available)
        {
            horizonTS = rcs->end();
        }
        else
        {
            _breakList->remove(rcs->begin(), rcs->end() - 1);
        }
    }
    _breakList->remove(horizonTS, int_t_max);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ResourceCalendar::compile(int horizonTS)
{
    ResourceCalendarSpan* lastRSS = nullptr;
    uint_t pt = 0;
    int ts = 0;
    _minBreakTime = int_t_max;
    _maxBreakTime = int_t_min;
    _minNonBreakTime = int_t_max;
    _maxNonBreakTime = int_t_min;
    iterator it;
    for (it = begin(); it != end();)
    {
        auto rss = utl::cast<ResourceCalendarSpan>(*it);

        // delete rss if it is not a break
        if (rss->status() != rcss_onBreak)
        {
            removeIt(it);
            continue;
        }
        ++it;

        int rssBegin = rss->begin();
        int rssEnd = rss->end();

        // update minBreakTime, maxBreakTime
        _minBreakTime = utl::min(_minBreakTime, rssBegin);
        _maxBreakTime = utl::max(_maxBreakTime, rssEnd - 1);

        // fill gap before current break with a working span
        if (ts < rssBegin)
        {
            auto workingSpan =
                new ResourceCalendarSpan(ts, rssBegin, rcs_exception, rcss_available);
            _minNonBreakTime = utl::min(_minNonBreakTime, workingSpan->begin());
            _maxNonBreakTime = utl::max(_maxNonBreakTime, workingSpan->end() - 1);
            if (lastRSS != nullptr)
            {
                lastRSS->setNext(workingSpan);
                workingSpan->setPrev(lastRSS);
            }
            lastRSS = workingSpan;
            add(workingSpan);

            pt += workingSpan->size();
            ts = rssBegin;
            workingSpan->setCumPt(pt);
        }

        // set next, prev ptrs
        if (lastRSS != nullptr)
        {
            lastRSS->setNext(rss);
            rss->setPrev(lastRSS);
        }
        lastRSS = rss;

        // break adds no processing-time
        rss->setCumPt(pt);
        ts = rssEnd;
    }

    // handle working time until horizon
    if (ts < horizonTS)
    {
        auto workingSpan = new ResourceCalendarSpan(ts, horizonTS, rcs_exception, rcss_available);
        _minNonBreakTime = utl::min(_minNonBreakTime, workingSpan->begin());
        _maxNonBreakTime = utl::max(_maxNonBreakTime, workingSpan->end() - 1);
        if (lastRSS != nullptr)
        {
            lastRSS->setNext(workingSpan);
            workingSpan->setPrev(lastRSS);
        }
        add(workingSpan);

        // increment pt, ts
        pt += workingSpan->size();
        ts = horizonTS;
        workingSpan->setCumPt(pt);
    }

    // note: ts = horizonTS
    _maxPT = pt;
    _maxTS = ts - 1;

    // put spans from _spanCol into _spans array
    _spans = new ResourceCalendarSpan*[size()];
    _spansLim = _spans + size();
    auto ptr = _spans;
    for (it = begin(); it != end(); ++it)
    {
        auto rss = utl::cast<ResourceCalendarSpan>(*it);
        rss->setCapacity(uint_t_max);
        *(ptr++) = rss;
    }
    ASSERTD(ptr == _spansLim);

    // forget spanCol
    setOwner(false);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ResourceCalendar::addCompiledSpansTo(ResourceCalendar* cal, rcs_status_t status) const
{
    for (auto it = _spans; it != _spansLim; ++it)
    {
        auto span = *it;
        if (span->status() != status)
            continue;
        auto spanCopy = span->clone();
        spanCopy->setCapacity(uint_t_max);
        cal->add(spanCopy);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

uint_t
ResourceCalendar::id() const
{
    ASSERTD(_spec.type() == rc_simple);
    ASSERTD(_spec.calIds().size() == 1);
    return *(_spec.calIds().begin());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ResourceCalendar::findForward(int& es, int& ef, uint_t pt) const
{
    // no working time at all?
    if (_minNonBreakTime > _maxNonBreakTime)
    {
        es = ef = int_t_max;
        return;
    }

    es = utl::max(es, _minNonBreakTime);

    // past the last working time?
    if (es > _maxNonBreakTime)
    {
        es = ef = int_t_max;
        return;
    }

    // note: _minNonBreakTime <= es <= _maxNonBreakTime
    auto rcs = findSpanByTime(es);
    if (rcs->status() == rcss_onBreak)
    {
        rcs = rcs->next(); // next span must be working
        ASSERTD(rcs->status() == rcss_available);
        es = rcs->begin();
    }

    // find EF s.t.
    //     pt(EF) - pt(ES) = (pt - 1)
    uint_t esPT = (rcs->cumPt() - (rcs->end() - 1 - es));
    uint_t efPT = esPT + pt - 1;
    if (efPT > _maxPT)
    {
        es = ef = int_t_max;
        return;
    }
    ef = ptTS(efPT);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ResourceCalendar::findBackward(int& lf, int& ls, uint_t pt) const
{
    // no working time at all?
    if (_minNonBreakTime > _maxNonBreakTime)
    {
        lf = ls = int_t_min;
        return;
    }

    lf = utl::min(lf, _maxNonBreakTime);

    // past the last working time?
    if (lf < _minNonBreakTime)
    {
        lf = ls = int_t_min;
        return;
    }

    // note: _minNonBreakTime <= lf <= _maxNonBreakTime
    auto rcs = findSpanByTime(lf);
    if (rcs->status() == rcss_onBreak)
    {
        rcs = rcs->prev(); // prev span must be working
        ASSERTD(rcs->status() == rcss_available);
        lf = rcs->end() - 1;
    }

    // find LS s.t.
    //     pt(LF) - pt(LS) = (pt - 1)
    uint_t lfPT = (rcs->cumPt() - (rcs->end() - 1 - lf));
    uint_t lsPT = lfPT - pt + 1;
    if (lsPT < 1)
    {
        lf = ls = int_t_min;
        return;
    }
    ls = ptTS(lsPT);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

uint_t
ResourceCalendar::getBreakTime(int begin, int end) const
{
    if (begin > end)
        return 0U;

    // no break time during [begin,end] ?
    if ((begin > _maxBreakTime) || (end < _minBreakTime))
        return 0U;

    // note: can't compute non-break-time as (tsPT(end) - tsPT(begin)) ...

    // start with all break time and subtract working time
    uint_t bt = (end - begin + 1);

    // find first non-break time >= begin
    auto rcsBegin = findSpanByTime(begin);
    if (rcsBegin->status() == rcss_onBreak)
    {
        rcsBegin = rcsBegin->next();
    }
    // no non-break time in [begin,end] ?
    if (rcsBegin->begin() > end)
        return bt;

    // note:
    //     begin <= end
    //     begin <= _maxBreakTime
    //       end >= _minBreakTime

    // adjust begin
    begin = utl::max(begin, rcsBegin->begin());
    uint_t beginPT = rcsBegin->cumPt() - (rcsBegin->end() - 1 - begin);
    uint_t endPT = tsPT(end);
    bt -= (endPT - beginPT + 1);
    return bt;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int
ResourceCalendar::getBreakTimePrev(int t) const
{
    if (t <= _minBreakTime)
        return int_t_min;
    if (t > _maxBreakTime)
        return _maxBreakTime;
    auto rcs = findSpanByTime(--t);
    if (rcs->status() == rcss_available)
        return (rcs->prev()->end() - 1);
    return t;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int
ResourceCalendar::getBreakTimeNext(int t) const
{
    if (t < _minBreakTime)
        return _minBreakTime;
    if (t >= _maxBreakTime)
        return int_t_max;
    auto rcs = findSpanByTime(++t);
    if (rcs->status() == rcss_available)
        return rcs->next()->begin();
    return t;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int
ResourceCalendar::getNonBreakTimePrev(int t) const
{
    if (t <= _minNonBreakTime)
        return int_t_min;
    if (t > _maxNonBreakTime)
        return _maxNonBreakTime;
    auto rcs = findSpanByTime(--t);
    if (rcs->status() == rcss_onBreak)
        return (rcs->prev()->end() - 1);
    return t;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int
ResourceCalendar::getNonBreakTimeNext(int t) const
{
    if (t < _minNonBreakTime)
        return _minNonBreakTime;
    if (t >= _maxNonBreakTime)
        return int_t_max;
    auto rcs = findSpanByTime(++t);
    if (rcs->status() == rcss_onBreak)
        return rcs->next()->begin();
    return t;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int
ResourceCalendar::getStartTimeForEndTime(int endTime, uint_t pt) const
{
    ASSERTD((endTime >= 0) && (endTime <= _maxTS));
    if (pt == 0)
        return (endTime + 1);
    uint_t endPT = tsPT(endTime);
    if (pt > endPT)
        return int_t_min;
    uint_t startPT = endPT - pt + 1;
    return ptTS(startPT);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int
ResourceCalendar::getEndTimeForStartTime(int startTime, uint_t pt) const
{
    ASSERTD((startTime >= 0) && (startTime <= _maxTS));
    if (pt == 0)
        return startTime;
    uint_t startPT = tsPT(startTime);
    uint_t endPT = startPT + pt - 1;
    if (endPT > _maxPT)
        return int_t_max;
    return ptTS(endPT);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ResourceCalendar::init()
{
    _serialId = uint_t_max;
    _spans = _spansLim = nullptr;
    _minBreakTime = int_t_max;
    _maxBreakTime = int_t_min;
    _minNonBreakTime = int_t_max;
    _maxNonBreakTime = int_t_min;
    _maxPT = 0;
    _maxTS = 0;
    _breakList = nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ResourceCalendar::deInit()
{
    if (_spans != nullptr)
    {
        deleteArray(_spans, _spansLim);
        delete[] _spans;
    }
    delete _breakList;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ResourceCalendar::check(const iterator* testIt)
{
    BinTreeNode* testNode;
    if (testIt == nullptr)
        testNode = nullptr;
    else
        testNode = testIt->node();
    auto endIt = end();
    auto endNode = endIt.node();
    ASSERT(endNode->get()->isA(MaxObject));
    bool testNodeFound = false;
    for (auto it = begin();; ++it)
    {
        if (it.node() == testNode)
            testNodeFound = true;
        if (it == end())
            break;
        auto object = *it;
        ASSERT(object != nullptr);
        ASSERT(object->isA(ResourceCalendarSpan));
    }
    ASSERT((testNode == nullptr) || testNodeFound);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

uint_t
ResourceCalendar::tsPT(int ts) const
{
    auto span = findSpanByTime(ts);
    uint_t pt = span->cumPt();
    if (span->status() == rcss_onBreak)
        return pt;
    pt -= (span->end() - 1 - ts);
    return pt;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int
ResourceCalendar::ptTS(uint_t pt) const
{
    auto span = findSpanByPt(pt);
    uint_t beginPt = span->cumPt() - span->size() + 1;
    int ts = span->begin() + (pt - beginPt);
    return ts;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

const ResourceCalendarSpan*
ResourceCalendar::findSpanByTime(int ts) const
{
    _searchSpan.setEnd(ts + 1);
    auto it = std::lower_bound(_spans, _spansLim, &_searchSpan, ResourceCalendarSpanEndOrdering());
    ASSERTD(it != _spansLim);
    auto span = *it;
    return span;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

const ResourceCalendarSpan*
ResourceCalendar::findSpanByPt(int pt) const
{
    _searchSpan.setCumPt(pt);
    auto it =
        std::lower_bound(_spans, _spansLim, &_searchSpan, ResourceCalendarSpanCumPtOrdering());
    ASSERTD(it != _spansLim);
    auto span = *it;
    return span;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_END;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_INSTANTIATE_TPL(utl::TRBtree, utl::Span<int>);
UTL_INSTANTIATE_TPL(utl::TRBtreeIt, utl::Span<int>);
UTL_INSTANTIATE_TPL2(utl::SpanCol, int, int);
