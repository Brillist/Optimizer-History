#ifndef CLP_REVSTACK_H
#define CLP_REVSTACK_H

//////////////////////////////////////////////////////////////////////////////

#include <clp/RevArray.h>

//////////////////////////////////////////////////////////////////////////////

CLP_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////

class Manager;

//////////////////////////////////////////////////////////////////////////////
//// RevStack ////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

/**
   Reversible stack.

   RevStack is a stack that tracks changes to itself, so they can
   be reversed in the event of backtracking.

   \author Adam McKee
*/

//////////////////////////////////////////////////////////////////////////////

template <class T>
class RevStack : public utl::Object
{
    UTL_CLASS_DECL_TPL(RevStack, T);
    UTL_CLASS_NO_COPY;
    UTL_CLASS_DEFID;
public:
    typedef const T* iterator;
public:
    /** Constructor. */
    RevStack(Manager* mgr)
    { _pos = 0; _array.setManager(mgr); }

    /** Get the manager. */
    Manager* manager() const
    { return _mgr; }

    /** Set the manager. */
    void setManager(Manager* mgr)
    { _mgr = mgr; }

    /** Get the size. */
    utl::uint_t size() const
    { return _array.size(); }

    /** Push the given object onto the stack. */
    void push(const T& object)
    { _array.add(object); }

    /** Get a begin iterator. */
    iterator begin() const
    { return _array.begin(); }

    /** Get an end iterator. */
    iterator end() const
    { return _array.end(); }

    /** Index into the stack. */
    const T& operator[](utl::uint_t idx) const
    { return _array[idx]; }
private:
    RevArray<T> _array;
};

//////////////////////////////////////////////////////////////////////////////

CLP_NS_END;

//////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL_TPL(clp::RevStack, T, utl::Object);

//////////////////////////////////////////////////////////////////////////////

#endif
