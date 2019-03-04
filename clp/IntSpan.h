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

    /// \name Accessors (const)
    //@{
    uint_t
    level() const
    {
        return _level;
    }

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

    IntSpan*
    prev() const
    {
        return _prev;
    }

    IntSpan*
    next() const
    {
        return _next[0];
    }

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

    void
    saveState(Manager* mgr)
    {
        if (_stateDepth < mgr->depth())
            _saveState(mgr);
    }

    void _saveState(Manager* mgr);
    //@}

    virtual uint_t capacity() const;

    bool
    contains(int val) const
    {
        return (_min <= val) && (val <= _max);
    }

    virtual bool canMergeWith(const IntSpan* rhs) const;

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
    uint_t _level;
    IntSpan* _prev;
    IntSpan** _next;

private:
    void init();
    void deInit();
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_END;
