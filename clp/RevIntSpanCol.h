#ifndef CLP_REVINTSPANCOL_H
#define CLP_REVINTSPANCOL_H

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <lut/SkipListDepthArray.h>
#include <clp/ConstrainedVar.h>
#include <clp/IntSpan.h>
#include <clp/IntSpanArray.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

class Manager;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Reversible collection of non-overlapping integer spans.

   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class RevIntSpanCol
{
public:
    /** Constructor. */
    RevIntSpanCol()
    {
        init();
    }

    /** Constructor. */
    RevIntSpanCol(Manager* mgr)
    {
        init(mgr);
    }

    /** Destructor. */
    virtual ~RevIntSpanCol()
    {
        deInit();
    }

    /** Copy another instance. */
    virtual void copy(const RevIntSpanCol& rhs);

    /// \name Accessors
    //@{
    /** Get the manager. */
    Manager*
    manager() const
    {
        return _mgr;
    }

    /** Set the manager. */
    void setManager(Manager* mgr);

    /** Find the last span s.t. span.min <= val. */
    const IntSpan*
    find(int val) const
    {
        return ((RevIntSpanCol*)this)->find(val);
    }

    /** Find the span that contains val. */
    IntSpan* find(int val);

    /** Get the revision number. */
    uint_t
    rev() const
    {
        return _rev;
    }

    /** Get the head. */
    const IntSpan*
    head() const
    {
        return _head;
    }

    /** Get the tail. */
    const IntSpan*
    tail() const
    {
        return _tail;
    }
    //@}
private:
    void init(Manager* mgr = nullptr);
    void deInit();

protected:
    void clear();

    void
    saveState()
    {
        if (_stateDepth < _mgr->depth())
            _saveState();
        else
            ++_rev;
    }

    virtual void _saveState();

    virtual IntSpan* newIntSpan(
        int min, int max, uint_t v0, uint_t v1, uint_t level = uint_t_max);

    virtual void set(IntSpan* span);

    void set(int min, int max, uint_t v0, uint_t v1);

    /** Find the span that contains val. */
    IntSpan* findPrev(int val, IntSpan** prev) const;

    /** Find the span that contains val. */
    IntSpan* findNext(int val, IntSpan** next) const;

    /** Find the span that contains val. */
    IntSpan* findPrevForward(int val, IntSpan** prev) const;

    /** Find the span that contains val. */
    IntSpan* findNextForward(int val, IntSpan** next) const;

    void backward(IntSpan** next);

    void eclipseNext(IntSpan** prev);

    uint_t insertAfter(IntSpan* span, IntSpan** prevSpans);

    void link(IntSpan** prev, IntSpan** next);

    void prevToNext(const IntSpan* span, IntSpan** prev, IntSpan** next);

    virtual uint_t validate(bool initialized = true) const;

protected:
    Manager* _mgr;
    const lut::SkipListDepthArray* _slda;
    uint_t _sldaIdx;

    /// reversible //////////////////////////////////////
    uint_t _stateDepth;
    uint_t _level;
    uint_t _rev;
    /// reversible //////////////////////////////////////

    IntSpan* _head;
    IntSpan* _tail;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_END;

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
