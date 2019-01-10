#ifndef CSE_JOBOPORDERING_H
#define CSE_JOBOPORDERING_H

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <cse/JobOp.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Define JobOrdering and OpOrdering classesthat are inherited from 
   utl::Ordring and their children classes. They are mainly used by 
   RuleBasedScheduler and its children classes to compare and order 
   two Jobs / Ops.

   Joe Zhou
   March 23, 2006
*/

////////////////////////////////////////////////////////////////////////////////////////////////////
// JobOrdering //////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

class JobOrdering : public utl::Ordering
{
    UTL_CLASS_DECL_ABC(JobOrdering);

public:
    virtual void copy(const utl::Object& rhs);

    /** Set _nextOrdering. */
    virtual void
    setNextOrdering(JobOrdering* ordering)
    {
        delete _nextOrdering;
        _nextOrdering = ordering;
    }

    /** Get _nextOrdering. */
    virtual JobOrdering*
    nextOrdering() const
    {
        return _nextOrdering;
    }

private:
    void
    init()
    {
        _nextOrdering = nullptr;
    }
    void
    deInit()
    {
        delete _nextOrdering;
    }

private:
    JobOrdering* _nextOrdering;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// JobOrderingDecSuccessorDepth ////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

class JobOrderingDecSuccessorDepth : public JobOrdering
{
    UTL_CLASS_DECL(JobOrderingDecSuccessorDepth);
    UTL_CLASS_DEFID;

public:
    virtual int cmp(const utl::Object* lhs, const utl::Object* rhs) const;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// JobOrderingDecLatenessCost /////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

class JobOrderingDecLatenessCost : public JobOrdering
{
    UTL_CLASS_DECL(JobOrderingDecLatenessCost);
    UTL_CLASS_DEFID;

public:
    virtual int cmp(const utl::Object* lhs, const utl::Object* rhs) const;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// JobOrderingDecDueTime ///////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

class JobOrderingIncDueTime : public JobOrdering
{
    UTL_CLASS_DECL(JobOrderingIncDueTime);
    UTL_CLASS_DEFID;

public:
    virtual int cmp(const utl::Object* lhs, const utl::Object* rhs) const;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// JobOrderingIncSID ///////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

class JobOrderingIncSID : public JobOrdering
{
    UTL_CLASS_DECL(JobOrderingIncSID);
    UTL_CLASS_DEFID;

public:
    virtual int cmp(const utl::Object* lhs, const utl::Object* rhs) const;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// OpOrdering //////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

class OpOrdering : public utl::Ordering
{
    UTL_CLASS_DECL_ABC(OpOrdering);
    UTL_CLASS_DEFID;

public:
    /** Is the given op suitable for scheduling? */
    virtual bool isSchedulable(const JobOp* op) const;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// OpOrderingFrozenFirst ///////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

class OpOrderingFrozenFirst : public OpOrdering
{
    UTL_CLASS_DECL(OpOrderingFrozenFirst);
    UTL_CLASS_DEFID;

public:
    virtual int cmp(const utl::Object* lhs, const utl::Object* rhs) const;

    virtual bool isSchedulable(const JobOp* op) const;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// OpOrderingIncES /////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

class OpOrderingIncES : public OpOrdering
{
    UTL_CLASS_DECL(OpOrderingIncES);
    UTL_CLASS_DEFID;

public:
    virtual int cmp(const utl::Object* lhs, const utl::Object* rhs) const;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// OpOrderingIncSID ////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

class OpOrderingIncSID : public OpOrdering
{
    UTL_CLASS_DECL(OpOrderingIncSID);
    UTL_CLASS_DEFID;

public:
    virtual int cmp(const utl::Object* lhs, const utl::Object* rhs) const;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// JobOpOrderingIncSID ////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

class JobOpOrderingIncSID : public OpOrdering
{
    UTL_CLASS_DECL(JobOpOrderingIncSID);
    UTL_CLASS_DEFID;

public:
    virtual int cmp(const utl::Object* lhs, const utl::Object* rhs) const;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
