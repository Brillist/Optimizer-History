#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <clp/Goal.h>
#include <lut/RCobject.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

class Manager;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Search goal (abstract).

   The purpose of goals is to guide the solution search, through a process
   of decision-making.  Override the execute() method to define your own
   goal's behavior.

   \see Manager
   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class Goal : public lut::RCobject
{
    UTL_CLASS_DECL_ABC(Goal, lut::RCobject);

public:
    /** Constructor. */
    Goal(Manager* mgr)
    {
        _mgr = mgr;
    }

    /** Copy another instance. */
    virtual void copy(const utl::Object& rhs);

    /** Execute the goal. */
    virtual void execute() = 0;

    /** Get the manager. */
    Manager*
    manager() const
    {
        return _mgr;
    }

    /** Set the manager. */
    void
    setManager(Manager* mgr)
    {
        _mgr = mgr;
    }

    /** Get the general-purpose flag. */
    bool
    getFlag() const
    {
        return _flag;
    }

    /** Set the general-purpose flag. */
    void
    setFlag(bool flag)
    {
        _flag = flag;
    }

private:
    void
    init()
    {
        _mgr = nullptr;
        _flag = false;
    }
    void
    deInit()
    {
    }

    Manager* _mgr;
    bool _flag;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_END;
