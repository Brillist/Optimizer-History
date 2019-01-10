#include "libclp.h"
#include <libutl/R250.h>
#include "Manager.h"
#include "RevIntSpanCol.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef DEBUG
//#define DEBUG_UNIT
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

void
RevIntSpanCol::copy(const RevIntSpanCol& isc)
{
    clear();

    // copy manager, slda
    _mgr = isc._mgr;
    _slda = isc._slda;
    ASSERTD(_mgr != nullptr);
    ASSERTD(_slda != nullptr);

    // copy the list
    IntSpan* prev[CLP_INTSPAN_MAXDEPTH];
    for (uint_t i = 0; i < CLP_INTSPAN_MAXDEPTH; ++i)
    {
        prev[i] = _head;
    }
    for (const IntSpan* span = isc.head()->next(); span != isc.tail(); span = span->next())
    {
        // make a new span
        IntSpan* mySpan = span->clone();
        _mgr->revAllocate(mySpan);

        // link new span to predecessors
        uint_t level = insertAfter(mySpan, prev);

        // update previous span links
        for (uint_t i = 0; i <= level; ++i)
        {
            prev[i] = mySpan;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
RevIntSpanCol::setManager(Manager* mgr)
{
    _mgr = mgr;
    _slda = (_mgr == nullptr) ? nullptr : _mgr->skipListDepthArray();
    _sldaIdx = 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

IntSpan*
RevIntSpanCol::find(int val)
{
    uint_t lvl;
    IntSpan* node = _head;
    IntSpan* next;
    for (lvl = _level; lvl != uint_t_max; --lvl)
    {
        next = node->next(lvl);
        // while (next not far enough)
        while (val > next->max())
        {
            node = next;
            next = node->next(lvl);
        }
        // note: val <= next->max
    }
    return next;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
RevIntSpanCol::init(Manager* mgr)
{
    setManager(mgr);
    _head = nullptr;
    _tail = nullptr;
    clear();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
RevIntSpanCol::deInit()
{
    delete _head;
    delete _tail;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
RevIntSpanCol::clear()
{
    _sldaIdx = 0;
    _stateDepth = 0;
    _level = 0;
    _rev = 0;

    // make head
    delete _head;
    _head = newIntSpan(int_t_min, int_t_min, 0, 0, CLP_INTSPAN_MAXDEPTH - 1);

    // make tail
    delete _tail;
    _tail = newIntSpan(int_t_max, int_t_max, 0, 0, CLP_INTSPAN_MAXDEPTH - 1);

    // link head with tail
    for (uint_t lvl = 0; lvl < CLP_INTSPAN_MAXDEPTH; ++lvl)
    {
        _head->setNext(lvl, _tail);
    }
    _tail->setPrev(_head);

    // link head to itself
    _head->setPrev(_head);

    // link tail to itself
    for (uint_t lvl = 0; lvl < CLP_INTSPAN_MAXDEPTH; ++lvl)
    {
        _tail->setNext(lvl, _tail);
    }

#ifdef DEBUG_UNIT
    RevIntSpanCol::validate(false);
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
RevIntSpanCol::_saveState()
{
    ASSERTD(_stateDepth < _mgr->depth());
    _mgr->revSet(&_stateDepth, 3);
    _stateDepth = _mgr->depth();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

IntSpan*
RevIntSpanCol::newIntSpan(int min, int max, utl::uint_t v0, utl::uint_t v1, utl::uint_t level)
{
    return new IntSpan(min, max, v0, v1, level);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
RevIntSpanCol::set(IntSpan* span)
{
    saveState();
    IntSpan* prev[CLP_INTSPAN_MAXDEPTH];
    findPrev(span->max() + 1, prev);
    insertAfter(span, prev);
#ifdef DEBUG_UNIT
    validate(false);
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
RevIntSpanCol::set(int min, int max, utl::uint_t v0, utl::uint_t v1)
{
    saveState();
    IntSpan* span = newIntSpan(min, max, v0, v1);
    span->setStateDepth(_stateDepth);
    _mgr->revAllocate(span);
    set(span);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

IntSpan*
RevIntSpanCol::findPrev(int val, IntSpan** prev) const
{
    uint_t lvl;
    IntSpan* node = _head;
    IntSpan* nodeNext;
    for (lvl = _level; lvl != uint_t_max; --lvl)
    {
        nodeNext = node->next(lvl);
        // while (next not far enough)
        while (nodeNext->max() < val)
        {
            node = nodeNext;
            nodeNext = node->next(lvl);
        }
        // note: nodeNext->max() >= val
        prev[lvl] = node;
    }
    for (lvl = _level + 1; lvl < CLP_INTSPAN_MAXDEPTH; ++lvl)
    {
        prev[lvl] = _head;
    }
    return nodeNext;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

IntSpan*
RevIntSpanCol::findNext(int val, IntSpan** next) const
{
    uint_t lvl;
    IntSpan* node = _head;
    IntSpan* nodeNext;
    for (lvl = _level; lvl != uint_t_max; --lvl)
    {
        nodeNext = node->next(lvl);
        // while (next not far enough)
        while (nodeNext->max() < val)
        {
            node = nodeNext;
            nodeNext = node->next(lvl);
        }
        // note: nodeNext->max() >= val
        if (nodeNext->min() <= val)
        {
            // next contains val
            next[lvl] = nodeNext->next(lvl);
        }
        else
        {
            // next > val
            next[lvl] = nodeNext;
        }
    }
    for (lvl = _level + 1; lvl < CLP_INTSPAN_MAXDEPTH; ++lvl)
    {
        next[lvl] = _tail;
    }
    return nodeNext;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

IntSpan*
RevIntSpanCol::findPrevForward(int val, IntSpan** prev) const
{
    uint_t lvl;
    for (lvl = 0; lvl < CLP_INTSPAN_MAXDEPTH; ++lvl)
    {
        IntSpan* node = prev[lvl];
        IntSpan* nodeNext = node->next(lvl);
        while (nodeNext->max() < val)
        {
            node = nodeNext;
            nodeNext = node->next(lvl);
        }
        // note: nodeNext->max() >= val
        if (node == prev[lvl])
        {
            break;
        }
        prev[lvl] = node;
    }
    return prev[0]->next();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

IntSpan*
RevIntSpanCol::findNextForward(int val, IntSpan** next) const
{
    uint_t lvl;
    for (lvl = 0; lvl < CLP_INTSPAN_MAXDEPTH; ++lvl)
    {
        IntSpan* node = next[lvl];
        IntSpan* nodeNext = node->next(lvl);
        while (nodeNext->min() <= val)
        {
            node = nodeNext;
            nodeNext = node->next(lvl);
        }
        // note: nodeNext->min() > val
        if (next[lvl] == nodeNext)
        {
            break;
        }
        next[lvl] = nodeNext;
    }
    return next[0]->prev();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
RevIntSpanCol::backward(IntSpan** next)
{
    IntSpan* span = next[0]->prev();
    uint_t lvl;
    uint_t maxLevel = span->getLevel();
    for (lvl = 0; (lvl <= maxLevel) && (span->next(lvl) == next[lvl]); ++lvl)
    {
        next[lvl] = span;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
RevIntSpanCol::eclipseNext(IntSpan** prev)
{
    IntSpan* span = prev[0]->next(0);
    IntSpan* next = span->next(0);
    next->saveState(_mgr);
    next->prev() = prev[0];
    for (uint_t i = 0; (i < CLP_INTSPAN_MAXDEPTH) && (prev[i]->next(i) == span); ++i)
    {
        prev[i]->saveState(_mgr);
        prev[i]->next(i) = span->next(i);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

uint_t
RevIntSpanCol::insertAfter(IntSpan* span, IntSpan** prevSpans)
{
    span->setStateDepth(_stateDepth);
    uint_t level = _slda->getNext(_sldaIdx);
    span->setLevel(level);
    _level = utl::max(_level, level);

    // set prev pointers
    IntSpan* prev0 = prevSpans[0];
    IntSpan* next0 = prev0->next();
    next0->saveState(_mgr);
    next0->prev() = span;
    span->prev() = prev0;

    for (uint_t lvl = 0; lvl <= level; ++lvl)
    {
        IntSpan* prev = prevSpans[lvl];
        IntSpan* next = prevSpans[lvl]->next(lvl);
        prev->saveState(_mgr);
        prev->next(lvl) = span;
        span->next(lvl) = next;
    }
    return level;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
RevIntSpanCol::link(IntSpan** prev, IntSpan** next)
{
    for (uint_t lvl = 0; lvl < CLP_INTSPAN_MAXDEPTH; ++lvl)
    {
        prev[lvl]->saveState(_mgr);
        next[lvl]->saveState(_mgr);
        prev[lvl]->next(lvl) = next[lvl];
    }
    next[0]->prev() = prev[0];
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
RevIntSpanCol::prevToNext(const IntSpan* span, IntSpan** prev, IntSpan** next)
{
    int spanMax = span->max();
    memcpy(next, prev, (_level + 1) * sizeof(IntSpan*));
    uint_t lvl;
    for (lvl = 0; lvl <= _level; ++lvl)
    {
        IntSpan* span = next[lvl];
        while (span->min() <= spanMax)
        {
            span = span->next(lvl);
        }
        next[lvl] = span;
    }
    for (; lvl < CLP_INTSPAN_MAXDEPTH; ++lvl)
    {
        next[lvl] = _tail;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

utl::uint_t
RevIntSpanCol::validate(bool initialized) const
{
    ASSERT(_head->min() == int_t_min);
    ASSERT(_tail->max() == int_t_max);
    ASSERT(_head->min() == _head->max());
    ASSERT(_tail->min() == _tail->max());
    std_hash_set<size_t>* lastSet = nullptr;
    std_hash_set<size_t>* curSet = new std_hash_set<size_t>;
    uint_t lvl;
    uint_t totsize = 0;
    for (lvl = 0; lvl <= _level; ++lvl)
    {
        IntSpan* node = _head->next(lvl);
        while (node != _tail)
        {
            curSet->insert((size_t)node);
            if (lvl > 0)
            {
                ASSERT(lastSet->find((size_t)node) != lastSet->end());
            }
            ASSERT(node->min() <= node->max());
            if (lvl == 0)
            {
                ASSERT(node->next()->prev() == node);
                if ((node != _head) && (node->next() != _tail))
                {
                    ASSERT(!node->canMergeWith(node->next()));
                }
                if (node->v0() != 0)
                {
                    totsize += (node->size() * node->v0());
                }
            }
            if ((lvl == 0) && initialized)
            {
                ASSERT(node->max() == (node->next()->min() - 1));
            }
            else
            {
                ASSERT(node->max() < node->next(lvl)->min());
            }
            node = node->next(lvl);
        }
        delete lastSet;
        lastSet = curSet;
        curSet = new std_hash_set<size_t>;
    }
    for (; lvl < CLP_INTSPAN_MAXDEPTH; ++lvl)
    {
        ASSERT(_head->next(lvl) == _tail);
    }
    delete lastSet;
    delete curSet;
    return totsize;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_END;
