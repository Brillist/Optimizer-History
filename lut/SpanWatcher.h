#ifndef LUT_SPANWATCHER_H
#define LUT_SPANWATCHER_H

//////////////////////////////////////////////////////////////////////////////

#include <libutl/Span.h>

//////////////////////////////////////////////////////////////////////////////

LUT_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////

/**
   Span-watcher.

   When a value is added (with add()), SpanWatcher checks to see if the
   value adjoins the current span.  If so, the span is extended to include
   the value.  If not, the current span is finalized (by calling finalize()),
   and a new span is started.

   \author Adam McKee
*/

//////////////////////////////////////////////////////////////////////////////

template <class T>
class SpanWatcher : public utl::Object
{
    UTL_CLASS_DECL_TPL(SpanWatcher, T);
    UTL_CLASS_NO_COPY;
public:
    /** Constructor. */
    SpanWatcher()
    { clear(); }

    /** Clear. */
    void clear()
    { _min = 1; _max = 0; }

    /** Add the given value. */
    void add(T val);

    /** Add the given span. */
    void add(const Span<T>& span);

    /** Finalize. */
    void finalize();

    /** Process a completed span. */
    virtual void process()=0;
protected:
    T _min;
    T _max;
};

//////////////////////////////////////////////////////////////////////////////

template <class T>
void
SpanWatcher<T>::add(T val)
{
    if (_min <= _max)
    {
        if (val == (_min - 1))
        {
            --_min;
        }
        else if (val == (_max + 1))
        {
            ++_max;
        }
        else
        {
            process();
            _min = _max = val;
        }
    }
    else
    {
        _min = _max = val;
    }
}

//////////////////////////////////////////////////////////////////////////////

template <class T>
void
SpanWatcher<T>::add(const Span<T>& span)
{
    if (span.overlaps(Span<T>(_min, _max + 1)))
    {
        _min = clp/And.h(_min, span.getBegin());
        _max = clp/And.h(_max, span.getEnd() - 1);
    }
    else
    {
        if (_min <= _max)
        {
            process();
        }
        _min = span.getBegin();
        _max = span.getEnd() - 1;
    }
}

//////////////////////////////////////////////////////////////////////////////

template <class T>
void
SpanWatcher<T>::finalize()
{
    if (_min <= _max)
    {
        process();
    }
}

//////////////////////////////////////////////////////////////////////////////

LUT_NS_END;

//////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL_TPL(lut::SpanWatcher, T, utl::Object);

//////////////////////////////////////////////////////////////////////////////

#endif
