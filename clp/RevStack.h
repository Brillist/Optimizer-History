#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <clp/RevArray.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

class Manager;

////////////////////////////////////////////////////////////////////////////////////////////////////
//// RevStack //////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Reversible stack.

   RevStack is a stack that supports backtracking.

   \see Manager
   \ingroup clp
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

template <class T> class RevStack : public utl::Object
{
    UTL_CLASS_DECL_TPL(RevStack, T, utl::Object);
    UTL_CLASS_NO_COPY;
    UTL_CLASS_DEFID;

public:
    typedef const T* iterator;

public:
    /** Constructor. */
    RevStack(Manager* mgr)
    {
        _pos = 0;
        _array.setManager(mgr);
    }

    /// \name Accessors (const)
    //@{
    /** Get the manager. */
    Manager*
    manager() const
    {
        return _mgr;
    }

    /** Get the size. */
    uint_t
    size() const
    {
        return _array.size();
    }

    /** Array element accessor. */
    const T& operator[](uint_t idx) const
    {
        return _array[idx];
    }

    /** Get begin iterator. */
    iterator
    begin() const
    {
        return _array.begin();
    }

    /** Get end iterator. */
    iterator
    end() const
    {
        return _array.end();
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
    //@}

    /// \name Modification
    //@{
    /** Push an object onto the stack. */
    void
    push(const T& object)
    {
        _array.add(object);
    }
    //@}

private:
    RevArray<T> _array;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_END;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL_TPL(clp::RevStack, T);
