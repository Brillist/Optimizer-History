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

   The purpose of goals is to guide the solution search through a process of decision-making.
   Override the execute() method to define your own goal's behavior.

   \see Manager
   \ingroup clp
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

    /// \name Accessors (const)
    //@{
    /** Get the manager. */
    Manager*
    manager() const
    {
        return _mgr;
    }

    /** Get the general-purpose flag. */
    bool
    flag() const
    {
        return _flag;
    }
    //@}

    /// \name Accessors (non-const)
    //@{
    /** Set the manager. */
    void
    setManager(Manager* mgr)
    {
        _mgr = mgr;
    }

    /** Set the general-purpose flag. */
    void
    setFlag(bool flag)
    {
        _flag = flag;
    }
    //@}

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

private:
    Manager* _mgr;
    bool _flag;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   A \c std::vector of Goal pointers.
   \ingroup clp
*/
using goal_vector_t = std::vector<Goal*>;

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_END;
