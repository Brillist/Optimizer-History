#ifndef CLS_CAPEXPMGR_H
#define CLS_CAPEXPMGR_H

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <clp/IntExp.h>
#include <cls/Schedule.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Capacity expression.

   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class CapExp : public utl::Object
{
    UTL_CLASS_DECL(CapExp);

public:
    typedef std::vector<utl::uint_t> uint_vector_t;

public:
    /** Constructor. */
    CapExp(const uint_vector_t& resCaps)
    {
        _resCaps = resCaps;
        _useCount = 0;
        _capExp = nullptr;
    }

    virtual int compare(const utl::Object& rhs) const;

    /** Initialize the expression. */
    void initExp(const Schedule* schedule);

    /** Increment use-count and get the capacity-expression. */
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
    mutable utl::uint_t _useCount;
    mutable clp::IntExp* _capExp;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Capacity expression manager.

   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class CapExpMgr : public utl::Object
{
    UTL_CLASS_DECL(CapExpMgr);
    UTL_CLASS_DEFID;
    UTL_CLASS_NO_COPY;

public:
    typedef std::vector<utl::uint_t> uint_vector_t;

public:
    CapExpMgr(Schedule* schedule)
    {
        _schedule = schedule;
    }

    /** Add a cap-exp. */
    void add(const uint_vector_t& resCaps);

    /** Find cap-exp. */
    clp::IntExp* find(const uint_vector_t& resCaps) const;

private:
    const Schedule* _schedule;
    utl::RBtree _capExps;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_END;

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
