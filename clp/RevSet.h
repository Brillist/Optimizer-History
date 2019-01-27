#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <libutl/RBtree.h>
#include <clp/ConstrainedVar.h>
#include <clp/Manager.h>
#include <clp/RevSetDelta.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

class Manager;

////////////////////////////////////////////////////////////////////////////////////////////////////
//// RevSet ////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Reversible set.

   RevSet is a set of objects that tracks changes to itself, so they can be reversed in the event
   of backtracking.

   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

template <class T> class RevSet : public ConstrainedVar
{
    UTL_CLASS_DECL_TPL(RevSet, T, ConstrainedVar);
    UTL_CLASS_NO_COPY;

public:
    typedef utl::TRBtree<T> set_t;
    typedef typename set_t::iterator iterator;

public:
    /** Constructor. */
    RevSet(Manager* mgr, rsd_t type)
    {
        init();
        _mgr = mgr;
        _type = type;
    }

    virtual void backtrack();

    /** Get the manager. */
    Manager*
    manager() const
    {
        return _mgr;
    }

    /** Initialize. */
    void initialize(Manager* mgr, rsd_t type = rsd_both);

    /** Get the size. */
    uint_t
    size() const
    {
        return _set.size();
    }

    /** Empty? */
    bool
    empty() const
    {
        return _set.empty();
    }

    /** Intersection size. */
    uint_t
    intersectCard(const RevSet<T>& rhs) const
    {
        return _set.intersectCard(rhs._set);
    }

    /** Add the given object to the set. */
    bool add(const utl::Object* object);

    /** Remove the given object from the set. */
    bool remove(const utl::Object* object);

    /** Contains the given object? */
    bool
    has(const utl::Object* key) const
    {
        return (_set.has(*key));
    }

    /** Find the given object in the set. */
    iterator
    find(const utl::Object* key) const
    {
        return _set.findIt(*key);
    }

    /** Get a begin iterator. */
    iterator
    begin() const
    {
        return _set.begin();
    }

    /** Get an end iterator. */
    iterator
    end() const
    {
        return _set.end();
    }

private:
    void init();

    void deInit();

    void clearDeltaStack();

    RevSetDelta* getDelta();

private:
    typedef typename std::stack<RevSetDelta*> delta_stack_t;

private:
    Manager* _mgr;
    rsd_t _type;
    set_t _set;
    delta_stack_t _deltaStack;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

template <class T>
void
RevSet<T>::backtrack()
{
    ASSERTD(!_deltaStack.empty());
    auto delta = _deltaStack.top();
    _deltaStack.pop();

    // so we can safely call add() and remove()...
    auto saveMgr = _mgr;
    _mgr = nullptr;

    // add removed items
    auto& removedItems = delta->removedItems();
    for (auto obj_ : removedItems)
    {
        auto obj = utl::cast<T>(obj_);
        add(obj);
    }

    // remove added items
    auto& addedItems = delta->addedItems();
    for (auto obj_ : addedItems)
    {
        auto obj = utl::cast<T>(obj_);
        remove(obj);
    }

    delete delta;
    _mgr = saveMgr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

template <class T>
void
RevSet<T>::initialize(Manager* mgr, rsd_t type)
{
    _mgr = mgr;
    _type = type;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

template <class T>
bool
RevSet<T>::add(const utl::Object* obj_)
{
    auto obj = utl::cast<T>(obj_);
    if (_set.add(obj))
    {
        auto delta = getDelta();
        if (delta != nullptr)
        {
            delta->add(obj);
        }
        return true;
    }

    return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

template <class T>
bool
RevSet<T>::remove(const utl::Object* obj_)
{
    auto obj = utl::cast<T>(obj_);
    if (_set.remove(*obj))
    {
        auto delta = getDelta();
        if (delta != nullptr)
        {
            delta->remove(obj);
        }
        return true;
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

template <class T>
void
RevSet<T>::init()
{
    _mgr = nullptr;
    _set.setOwner(false);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

template <class T>
void
RevSet<T>::deInit()
{
    clearDeltaStack();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

template <class T>
void
RevSet<T>::clearDeltaStack()
{
    while (!_deltaStack.empty())
    {
        auto delta = _deltaStack.top();
        _deltaStack.pop();
        delete delta;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

template <class T>
RevSetDelta*
RevSet<T>::getDelta()
{
    if (_mgr == nullptr)
    {
        return nullptr;
    }

    RevSetDelta* delta;
    if (_deltaStack.empty())
    {
        delta = nullptr;
    }
    else
    {
        delta = _deltaStack.top();
    }

    // create a new domain delta ?
    if ((delta == nullptr) || (delta->depth() < _mgr->depth()))
    {
        delta = new RevSetDelta(_type, _mgr->depth());
        _deltaStack.push(delta);
        _mgr->revSetVar(this);
    }

    return delta;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_END;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL_TPL(clp::RevSet, T);
