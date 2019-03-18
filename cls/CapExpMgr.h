#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <clp/IntExp.h>
#include <cls/Schedule.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Capacity expression.

   A capacity expression is an IntVar that tracks the time when a set of CompositeResource%s all
   have the required capacity.

   \see IntActivity
   \see CompositeResource
   \see CompositeTimetable
   \ingroup cls
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class CapExp : public utl::Object
{
    UTL_CLASS_DECL(CapExp, utl::Object);

public:
    /**
       Constructor.
       \param resCaps array of `[resourceSerialId,capacity]` tuples
    */
    CapExp(const uint_vector_t& resCaps)
    {
        _resCaps = resCaps;
        _useCount = 0;
        _capExp = nullptr;
    }

    virtual int compare(const utl::Object& rhs) const;

    /** Initialize the capacity expression. */
    void initExp(const Schedule* schedule);

    /** Get the capacity expression. */
    clp::IntExp* get(const Schedule* schedule) const;

private:
    void
    init()
    {
        ABORT();
    }
    void deInit();

private:
    uint_vector_t _resCaps;
    mutable uint_t _useCount;
    mutable clp::IntExp* _capExp;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Capacity expression manager.

   CapExpMgr provides central management of CapExp%s.  When an IntActivity has more than one
   CompositeResourceRequirement, it uses a CapExp to track times when all of its required
   CompositeResource%s have at least the minimum required capacity.  In the case where an
   IntActivity has a single CompositeResourceRequirement, it uses a capacity expression
   for that CompositeResource (see CompositeTimetable::addCapExp).

   \see CapExp
   \see CompositeResource
   \see CompositeResourceRequirement
   \see IntActivity
   \ingroup cls
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class CapExpMgr : public utl::Object
{
    UTL_CLASS_DECL(CapExpMgr, utl::Object);
    UTL_CLASS_DEFID;
    UTL_CLASS_NO_COPY;

public:
    /**
       Constructor.
       \param schedule related Schedule
    */
    CapExpMgr(Schedule* schedule)
    {
        _schedule = schedule;
    }

    /**
       Add a capacity expression.
       \param resCaps array of `[resourceSerialId,capacity]` tuples
    */
    void add(const uint_vector_t& resCaps);

    /**
       Find capacity expression.
       \param resCaps array of `[resourceSerialId,capacity]` tuples
    */
    clp::IntExp* find(const uint_vector_t& resCaps) const;

private:
    const Schedule* _schedule;
    utl::RBtree _capExps;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_END;
