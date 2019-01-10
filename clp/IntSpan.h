#ifndef CLP_INTSPAN_H
#define CLP_INTSPAN_H

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
    UTL_CLASS_DECL(IntSpan);

public:
    IntSpan(int min, int max, utl::uint_t v0, utl::uint_t v1, utl::uint_t level = utl::uint_t_max);

    virtual void copy(const utl::Object& rhs);

    virtual utl::String toString() const;

    virtual bool canMergeWith(const IntSpan* rhs) const;

    void clearPointers();

    virtual utl::uint_t capacity() const;

    utl::uint_t
    getStateDepth() const
    {
        return _stateDepth;
    }

    void
    setStateDepth(utl::uint_t stateDepth)
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

    utl::uint_t
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

    utl::uint_t
    v0() const
    {
        return _v0;
    }

    utl::uint_t
    v1() const
    {
        return _v1;
    }

    utl::uint_t&
    v0()
    {
        return _v0;
    }

    utl::uint_t&
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
    next(utl::uint_t idx) const
    {
        return _next[idx];
    }

    IntSpan*&
    next(utl::uint_t idx)
    {
        return _next[idx];
    }

    void
    setPrev(IntSpan* prev)
    {
        _prev = prev;
    }

    void
    setNext(utl::uint_t idx, IntSpan* span)
    {
        _next[idx] = span;
    }

    utl::uint_t
    getLevel() const
    {
        return _level;
    }

    void setLevel(utl::uint_t level);

protected:
#ifdef DEBUG
    size_t _id;
#endif
    utl::uint_t _stateDepth;
    int _min;
    int _max;
    utl::uint_t _v0;
    utl::uint_t _v1;
    IntSpan* _prev;
    IntSpan** _next;
    utl::uint_t _level;

private:
    void init();
    void deInit();
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_END;

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
