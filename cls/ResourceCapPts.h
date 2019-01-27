#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <libutl/Array.h>
#include <libutl/Hashtable.h>
#include <libutl/RBtree.h>
#include <libutl/Uint.h>
#include <clp/IntExp.h>
#include <cls/Resource.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

class Activity;
class BrkActivity;
class DiscreteResourceRequirement;
class IntActivity;

////////////////////////////////////////////////////////////////////////////////////////////////////
// CapPt ///////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

class CapPt : public utl::Object
{
    UTL_CLASS_DECL(CapPt, utl::Object);

public:
    /** Constructor. */
    CapPt(uint_t cap, uint_t pt);

    virtual void copy(const utl::Object& rhs);

    virtual int compare(const utl::Object& rhs) const;

    /** Get the capacity. */
    uint_t
    capacity() const
    {
        return _cap;
    }

    /** Get the processing-time. */
    uint_t
    processingTime() const
    {
        return _pt;
    }

    /** Get the processing-time. */
    uint_t&
    processingTime()
    {
        return _pt;
    }

    /** Get the object. */
    utl::Object*
    object() const
    {
        return _object;
    }

    /** Set the object. */
    void
    setObject(utl::Object* object)
    {
        ASSERTD(_object == nullptr);
        _object = object;
    }

private:
    void init();
    void deInit();

private:
    uint_t _cap;
    uint_t _pt;
    utl::Object* _object;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// ResourceCapPts //////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Capacity/processing-time pairs for execution of an activity on a resource.

   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class ResourceCapPts : public utl::Object
{
    UTL_CLASS_DECL(ResourceCapPts, utl::Object);

public:
    typedef utl::Vector<uint_t> uint_vector_t;
    typedef utl::RBtree::iterator iterator;

public:
    /** Copy another instance. */
    virtual void copy(const Object& rhs);

    /** Get the key. */
    virtual const Object&
    getKey() const
    {
        return _resId;
    }

    virtual void serialize(utl::Stream& stream, uint_t io, uint_t mode = utl::ser_default);

    /** Perform initialization (after cap/pts list is complete). */
    void initialize(Activity* act);

    /** Clear contents. */
    void clear();

    /** Get the manager. */
    clp::Manager*
    manager() const
    {
        if (_res == nullptr)
            return nullptr;
        return _res->manager();
    }

    /** Get the resource-id. */
    uint_t
    resourceId() const
    {
        return _resId;
    }

    /** Get the resource-id. */
    utl::Uint&
    resourceId()
    {
        return _resId;
    }

    /** Get the resource-serial-id. */
    uint_t
    resourceSerialId() const
    {
        ASSERTD(_res != nullptr);
        return _res->serialId();
    }

    /** Get the resource. */
    Resource*
    resource() const
    {
        return _res;
    }

    /** Get the resource. */
    Resource*&
    resource()
    {
        return _res;
    }

    /** Get the res-req. */
    DiscreteResourceRequirement*
    drr() const
    {
        return _drr;
    }

    /** Get the res-req. */
    DiscreteResourceRequirement*&
    drr()
    {
        return _drr;
    }

    /// \name Cap/Pt List
    //@{
    const utl::Array&
    capPtsArray() const
    {
        return _capPtsArray;
    }

    /** Get begin iterator. */
    utl::RBtree::iterator
    begin()
    {
        return _capPts.begin();
    }

    /** Get end iterator. */
    utl::RBtree::iterator
    end()
    {
        return _capPts.end();
    }

    /** Get the number of cap/pt pairs. */
    uint_t
    numCapPts() const
    {
        return _capPts.size();
    }

    /** Get a capacity/processing-time pair (by index). */
    void
    getCapPt(uint_t idx, uint_t& cap, uint_t& pt) const
    {
        const CapPt* capPt = (const CapPt*)_capPtsArray[idx];
        cap = capPt->capacity();
        pt = capPt->processingTime();
    }

    /** Add a capacity/processing-time pair. */
    bool addCapPt(uint_t cap, uint_t pt);

    /** Divide all pts by the given number. */
    void dividePtsBy(uint_t divisor);

    /** Find the CapPt object that matches the given capacity. */
    const CapPt* findCap(uint_t cap) const;

    /** Find the CapPt object that matches the given processing-time. */
    const CapPt* findPt(uint_t pt) const;
    //@}

    /// \name Cap/Pt selection
    //@{
    /** Set the selected processing-time. */
    const CapPt* selectPt(uint_t pt) const;

    /** Decrement the number of possible caps/pt pairs. */
    void decNumPossible() const;

    /** Select this resource. */
    void
    select() const
    {
        saveState();
        _selected = true;
    }

    /** Is this resource selected? */
    bool
    selected() const
    {
        return _selected;
    }

    /** Get the selected cap/pt pair. */
    const CapPt*
    selectedCapPt() const
    {
        return _selectedCapPt;
    }

    /** Get the selected capacity. */
    uint_t
    selectedCap() const
    {
        ASSERTD(_selectedCapPt != nullptr);
        return _selectedCapPt->capacity();
    }

    /** Get the selected processing-time. */
    uint_t
    selectedPt() const
    {
        ASSERTD(_selectedCapPt != nullptr);
        return _selectedCapPt->processingTime();
    }
    //@}

    /** Has non-zero cap/pt? */
    bool hasNonZeroCapPt() const;

private:
    void init();
    void
    deInit()
    {
    }

    void initialize(BrkActivity* act);

    void
    saveState() const
    {
        clp::Manager* mgr = _res->manager();
        if (_stateDepth < mgr->depth())
        {
            mgr->revSet(&_stateDepth, 3);
            _stateDepth = mgr->depth();
        }
    }

private:
    utl::Uint _resId;
    Resource* _res;
    DiscreteResourceRequirement* _drr;
    utl::RBtree _capPts;
    utl::Array _capPtsArray;
    utl::Hashtable _capPtsHTcap;
    utl::Hashtable _capPtsHTpt;
    mutable uint_t _stateDepth;
    mutable uint_t _numPossible;
    mutable bool _selected;
    mutable const CapPt* _selectedCapPt;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

typedef utl::TRBtree<ResourceCapPts> resCapPts_set_t;

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_END;
