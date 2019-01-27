#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <clp/Manager.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/** Bound type (lower or upper bound). */
enum bound_t
{
    bound_lb, /**< lower bound */
    bound_ub, /**< upper bound */
    bound_undefined
};

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Lower or upper bound.

   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class Bound : public utl::Object
{
    UTL_CLASS_DECL(Bound, utl::Object);
    UTL_CLASS_NO_COPY;

public:
    /** Constructor. */
    Bound(Manager* mgr, bound_t type, int bound = utl::int_t_min);

    /// \name Accessors
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

    /** Get the name. */
    String&
    name()
    {
        return _name;
    }

    /** Get the debug flag. */
    bool
    debugFlag() const
    {
        return _debugFlag;
    }

    /** Get the debug flag. */
    bool&
    debugFlag()
    {
        return _debugFlag;
    }

    /** Get the bound type. */
    bound_t
    type() const
    {
        return _type;
    }

    /** Get the bound. */
    virtual const int& get();

    /** Get a reference to the bound. */
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

    /** Get loosest bound. */
    int
    loosest()
    {
        return (_type == bound_lb) ? utl::int_t_min : utl::int_t_max;
    }

    /** Get tightest bound. */
    int
    tightest()
    {
        return (_type == bound_lb) ? utl::int_t_max : utl::int_t_min;
    }
    //@}

    /// \name Update
    //@{
    /** Bound is valid? */
    bool
    valid() const
    {
        return (_type == bound_lb) ? (_bound <= _findPoint) : (_bound >= _findPoint);
    }

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

    /** Set the given lower or upper bound. */
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
