#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <cls/CompositeResource.h>
#include <cls/EFboundInt.h>
#include <cls/IntActivity.h>
#include <cls/PreferredResources.h>
#include <cls/SchedulableBound.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   ES bound for interruptible activity.

   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class ESboundInt : public SchedulableBound
{
    friend class EFboundInt;
    UTL_CLASS_DECL(ESboundInt, SchedulableBound);

public:
    /** Constructor. */
    ESboundInt(IntActivity* act, int lb);

    /** Initialize. */
    void initialize();

    /** Register for events. */
    virtual void registerEvents();

    /** Allocate capacity. */
    virtual void allocateCapacity();

    /** Deallocate capacity. */
    virtual void deallocateCapacity();

    /** Get minimum capacity multiple. */
    uint_t
    minMultiple() const
    {
        return _minMultiple;
    }

    /** Get maximum capacity multiple. */
    uint_t
    maxMultiple() const
    {
        return _maxMultiple;
    }

    /** Set maximum capacity multiple. */
    void setMaxMultiple(uint_t maxMultiple);

    /** Get activity. */
    IntActivity*
    activity() const
    {
        return _act;
    }

    /** Get ef-bound. */
    const EFboundInt*
    efBound() const
    {
        return _efBound;
    }

    /** Get ef-bound. */
    EFboundInt*&
    efBound()
    {
        return _efBound;
    }

protected:
    virtual int find();

private:
    void
    init()
    {
        ABORT();
    }

    void deInit();

    void initMinMaxMultiple();

    void findForward(int& es, int& ef, bool allocate = false);

    void findBackward(int& es, int& ef, bool allocate = false);

private:
    IntActivity* _act;
    uint_t _numResources;
    uint_t* _caps;
    CompositeResource** _resources;
    const PreferredResources** _prs;
    CompositeSpan** _cspans;
    DiscreteResource** _visited;
    uint_t _numVisited;
    EFboundInt* _efBound;
    double _baseWorkPerTS;
    uint_t _minMultiple;
    uint_t _maxMultiple;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_END;
