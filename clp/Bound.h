#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <clp/Manager.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Bound type (lower or upper bound).
   \ingroup clp
*/
enum bound_t
{
    bound_lb, /**< lower bound */
    bound_ub, /**< upper bound */
    bound_undefined
};

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Lower or upper bound.

   For example, a lower bound could represent an activity's earliest possible start time (and
   an upper bound could represent its latest possible start time).  Apart from backtracking, bounds
   only move in one direction: lower bounds can only increase, and upper bounds can only decrease.

   The (protected) virtual function `find()` provides a way for a subclass of Bound to perform
   additional calculation to adjust the bound when it moves, ensuring it lands at a valid point.
   *Invalidating* a bound forces `find()` to be called to re-check the bound.

   \see CycleGroup
   \ingroup clp
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class Bound : public utl::Object
{
    UTL_CLASS_DECL(Bound, utl::Object);
    UTL_CLASS_NO_COPY;

public:
    /**
       Constructor.

       If the initial bound isn't specified it will be set to the loosest value based on bound type.

       \param mgr associated Manager
       \param type bound type
       \param bound (optional) initial bound
    */
    Bound(Manager* mgr, bound_t type, int bound = utl::int_t_min);

    /** Get a reference to the value of the bound. */
    virtual const int& get();

    /// \name Accessors (const)
    //@{
    /** Get the manager. */
    Manager*
    manager() const
    {
        return _mgr;
    }

    /** Get the name. */
    const String&
    name() const
    {
        return _name;
    }

    /** Get the debug flag. */
    bool
    debugFlag() const
    {
        return _debugFlag;
    }

    /** Get the bound type. */
    bound_t
    type() const
    {
        return _type;
    }

    /** Get a reference to the value of the bound. */
    const int&
    getRef() const
    {
        return _bound;
    }

    /** Get find-point. */
    int
    findPoint() const
    {
        return _findPoint;
    }
    //@}

    /// \name Accessors (non-const)
    //@{
    /** Set the name. */
    void
    setName(const char* name)
    {
        _name = name;
    }

    /** Set the debug flag. */
    void
    setDebugFlag(bool debugFlag)
    {
        _debugFlag = debugFlag;
    }
    //@}

    /// \name Calculation (const)
    /** Bound is valid? */
    bool
    valid() const
    {
        return (_type == bound_lb) ? (_bound <= _findPoint) : (_bound >= _findPoint);
    }

    /** Get loosest bound. */
    int
    loosest() const
    {
        return (_type == bound_lb) ? utl::int_t_min : utl::int_t_max;
    }

    /** Get tightest bound. */
    int
    tightest() const
    {
        return (_type == bound_lb) ? utl::int_t_max : utl::int_t_min;
    }
    //@}

    /// \name Update
    //@{
    /** Invalidate the bound. */
    void
    invalidate()
    {
        saveState();
        _findPoint = loosest();
    }

    /** Set lower-bound to -INF, upper-bound to +INF. */
    void
    setLoosest()
    {
        set(loosest());
    }

    /** Set lower-bound to +INF, upper-bound to -INF. */
    void
    setTightest()
    {
        set(tightest());
    }

    /** Set lower or upper bound (according to bound type). */
    int
    set(int bound)
    {
        if (_type == bound_lb)
            setLB(bound);
        else
            setUB(bound);
        return _bound;
    }

    /** Set lower bound. */
    virtual void
    setLB(int lb)
    {
        ASSERTD(_type == bound_lb);
        if (lb <= _bound)
            return;
#ifdef DEBUG
        if (_debugFlag)
        {
            BREAKPOINT;
        }
#endif
        saveState();
        _bound = lb;
    }

    /** Set upper bound. */
    virtual void
    setUB(int ub)
    {
        ASSERTD(_type == bound_ub);
        if (ub >= _bound)
            return;
#ifdef DEBUG
        if (_debugFlag)
        {
            BREAKPOINT;
        }
#endif
        saveState();
        _bound = ub;
    }
    //@}
protected:
    virtual int find();

    void
    saveState()
    {
        uint_t d = _mgr->depth();
        if (d <= _stateDepth)
            return;
        _saveState();
    }

    virtual void _saveState();

protected:
    Manager* _mgr;
    String _name;
    bool _debugFlag;
    bound_t _type;

    // reversible ///////////////////////////////////////
    uint_t _stateDepth;
    int _bound;
    int _findPoint;
    // reversible ///////////////////////////////////////
private:
    void
    init()
    {
        ABORT();
    }
    void
    deInit()
    {
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_END;
