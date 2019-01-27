#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <clp/Manager.h>
#include <libutl/Span.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

#define CLP_INTSPAN_MAXDEPTH 20U

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

class Manager;

////////////////////////////////////////////////////////////////////////////////////////////////////

class IntSpan : public utl::Object
{
    UTL_CLASS_DECL(IntSpan, utl::Object);

public:
    IntSpan(int min, int max, uint_t v0, uint_t v1, uint_t level = uint_t_max);

    virtual void copy(const utl::Object& rhs);

    virtual String toString() const;

    virtual bool canMergeWith(const IntSpan* rhs) const;

    void clearPointers();

    virtual uint_t capacity() const;

    uint_t
    getStateDepth() const
    {
        return _stateDepth;
    }

    void
    setStateDepth(uint_t stateDepth)
    {
        _stateDepth = stateDepth;
    }

    void
    saveState(Manager* mgr)
    {
        if (_stateDepth < mgr->depth())
            _saveState(mgr);
    }

    void _saveState(Manager* mgr);

    uint_t
    size() const
    {
        return (_max - _min + 1);
    }

    int
    min() const
    {
        return _min;
    }

    int
    max() const
    {
        return _max;
    }

    int&
    min()
    {
        return _min;
    }

    int&
    max()
    {
        return _max;
    }

    utl::Span<int>
    span() const
    {
        return utl::Span<int>(_min, _max + 1);
    }

    uint_t
    v0() const
    {
        return _v0;
    }

    uint_t
    v1() const
    {
        return _v1;
    }

    uint_t&
    v0()
    {
        return _v0;
    }

    uint_t&
    v1()
    {
        return _v1;
    }

    bool
    isHead() const
    {
        return (_prev == this);
    }

    bool
    isTail() const
    {
        return (_next[0] == this);
    }

    bool
    contains(int val) const
    {
        return (_min <= val) && (val <= _max);
    }

    IntSpan*
    prev() const
    {
        return _prev;
    }

    IntSpan*&
    prev()
    {
        return _prev;
    }

    IntSpan*
    next() const
    {
        return _next[0];
    }

    IntSpan*&
    next()
    {
        return _next[0];
    }

    IntSpan*
    next(uint_t idx) const
    {
        return _next[idx];
    }

    IntSpan*&
    next(uint_t idx)
    {
        return _next[idx];
    }

    void
    setPrev(IntSpan* prev)
    {
        _prev = prev;
    }

    void
    setNext(uint_t idx, IntSpan* span)
    {
        _next[idx] = span;
    }

    uint_t
    getLevel() const
    {
        return _level;
    }

    void setLevel(uint_t level);

protected:
#ifdef DEBUG
    size_t _id;
#endif
    uint_t _stateDepth;
    int _min;
    int _max;
    uint_t _v0;
    uint_t _v1;
    IntSpan* _prev;
    IntSpan** _next;
    uint_t _level;

private:
    void init();
    void deInit();
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_END;
