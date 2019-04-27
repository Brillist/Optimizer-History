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

/**
   Integer Span.

   A span of integer values specified as `[min,max]` that maps to a pair of integer values
   specified as `[v0,v1]`.

   RevIntSpanCol stores IntSpan objects in a skip-list.  To support that, an IntSpan has
   a \c prev pointer (pointing to the previous IntSpan at level 0), and an array of \c next
   pointers that link to the next IntSpan at each level of the node.

   \ingroup clp
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class IntSpan : public utl::Object
{
    UTL_CLASS_DECL(IntSpan, utl::Object);

public:
    /**
       Constructor.
       \param min minimum spanned value
       \param max maximum spanned value
       \param v0 first mapped value
       \param v1 second mapped value
       \param level level within skip-list
    */
    IntSpan(int min, int max, uint_t v0, uint_t v1, uint_t level = uint_t_max);

    virtual void copy(const utl::Object& rhs);

    virtual String toString() const;

    /** Get the capacity (v1 - v0). */
    virtual uint_t capacity() const;

    /** Span contains the provided value? */
    bool
    contains(int val) const
    {
        return (_min <= val) && (val <= _max);
    }

    /**
       Can this IntSpan merge with the provided IntSpan?  Two IntSpan%s can merge if they're
       equivalent (apart from their start and end points which are not considered here).
       \return true iff merging is permitted
       \param rhs proposed IntSpan for merging
    */
    virtual bool canMergeWith(const IntSpan* rhs) const;

    /** Set the level in the skip-list. */
    void setLevel(uint_t level);

    /// \name Accessors (const)
    //@{
    /** Get the level within the skip-list. */
    uint_t
    level() const
    {
        return _level;
    }

    /** Get the size of the span. */
    uint_t
    size() const
    {
        return (_max - _min + 1);
    }

    /** Get minimum value (beginning point) of span. */
    int
    min() const
    {
        return _min;
    }

    /** Get maximum value (end point) of span. */
    int
    max() const
    {
        return _max;
    }

    /** Convert to utl::Span<int>. */
    utl::Span<int>
    span() const
    {
        return utl::Span<int>(_min, _max + 1);
    }

    /** Get first mapped value. */
    uint_t
    v0() const
    {
        return _v0;
    }

    /** Get second mapped value. */
    uint_t
    v1() const
    {
        return _v1;
    }

    /** At head of skip-list? */
    bool
    isHead() const
    {
        return (_prev == this);
    }

    /** At tail of skip-list? */
    bool
    isTail() const
    {
        return (_next[0] == this);
    }

    /** Get previous IntSpan in skip-list. */
    IntSpan*
    prev() const
    {
        return _prev;
    }

    /** Get next IntSpan in skip-list. */
    IntSpan*
    next() const
    {
        return _next[0];
    }

    /** Get next IntSpan in skip-list (at specified level). */
    IntSpan*
    next(uint_t idx) const
    {
        ASSERTD(idx <= _level);
        return _next[idx];
    }
    //@}

    /// \name Accessors (non-const)
    //@{
    /** Set minimum value (beginning point) of span. */
    void
    setMin(int min)
    {
        _min = min;
    }

    /** Set maximum value (end point) of span. */
    void
    setMax(int max)
    {
        _max = max;
    }

    /** Set first mapped value. */
    void
    setV0(uint_t v0)
    {
        _v0 = v0;
    }

    /** Set second mapped value. */
    void
    setV1(uint_t v1)
    {
        _v1 = v1;
    }

    /** Set previous span. */
    void
    setPrev(IntSpan* prev)
    {
        _prev = prev;
    }

    /** Set next span (for level 0). */
    void
    setNext(IntSpan* next)
    {
        _next[0] = next;
    }

    /** Set next span for the given level. */
    void
    setNext(uint_t lvl, IntSpan* next)
    {
        ASSERTD(lvl <= _level);
        _next[lvl] = next;
    }
    //@}

    /// \name Backtracking
    //@{
    /** Get the state depth. */
    uint_t
    stateDepth() const
    {
        return _stateDepth;
    }

    void
    setStateDepth(uint_t stateDepth)
    {
        _stateDepth = stateDepth;
    }

    /** Save backtrackable state. */
    void
    saveState(Manager* mgr)
    {
        if (_stateDepth < mgr->depth())
            _saveState(mgr);
    }

    void _saveState(Manager* mgr);
    //@}

protected:
#ifdef DEBUG
    size_t _id;
#endif
    uint_t _stateDepth;
    int _min;
    int _max;
    uint_t _v0;
    uint_t _v1;
    uint_t _level;
    IntSpan* _prev;
    IntSpan** _next;

private:
    void init();
    void deInit();
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_END;
