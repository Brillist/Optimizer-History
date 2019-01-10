#ifndef CLS_ESBOUND_H
#define CLS_ESBOUND_H

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <clp/RevArray.h>
#include <cls/BrkActivity.h>
#include <cls/EFbound.h>
#include <cls/SchedulableBound.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Earliest valid time for an activity to begin execution.

   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class ESbound : public SchedulableBound
{
    friend class EFbound;
    UTL_CLASS_DECL(ESbound);

public:
    typedef clp::RevArray<clp::Bound*> bound_array_t;

public:
    /** Constructor. */
    ESbound(clp::Manager* mgr, int lb);

    /** Register for events. */
    virtual void registerEvents();

    /** Allocate capacity. */
    virtual void allocateCapacity();

    /** Allocate capacity from time t1 to t2. */
    virtual void allocateCapacity(int t1, int t2);

    /** Deallocate capacity. */
    virtual void deallocateCapacity();

    /** Deallocate capacity from time t1 to t2. */
    virtual void deallocateCapacity(int t1, int t2);

    /** Add a lower-bound. */
    void add(Bound* bound);

    /// \name Accessors
    //@{
    /** Get the activity. */
    const BrkActivity*
    act() const
    {
        return _act;
    }

    /** Get the activity. */
    BrkActivity*&
    act()
    {
        return _act;
    }

    /** Get the ef-bound. */
    const EFbound*
    efBound() const
    {
        return _efBound;
    }

    /** Get the ef-bound. */
    EFbound*&
    efBound()
    {
        return _efBound;
    }
    //@}

    /** Do something special for allocated bound. */
    virtual void setAllocatedLB(int oldBound, int newBound);

    /** Set new bound. */
    virtual void setLB(int lb);

protected:
    virtual int find();

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

private:
    BrkActivity* _act;
    EFbound* _efBound;
    bound_array_t _bounds;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_END;

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
