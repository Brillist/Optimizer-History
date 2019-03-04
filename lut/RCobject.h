#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

LUT_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Reference-counted object (abstract).

   An RCobject keeps a count of the number of objects that hold pointers to it.  When that count
   is reduced to 0, the RCobject is deleted.

   \ingroup lut
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class RCobject : public utl::Object
{
    UTL_CLASS_DECL_ABC(RCobject, utl::Object);

public:
    /** Add a reference. */
    void
    addRef() const
    {
        _refCount.fetch_add(1, std::memory_order_relaxed);
    }

    /** Remove a reference (and delete self if no other reference remains). */
    void
    removeRef() const
    {
        // if _refCount was 1 before we subtracted 1, it's now 0
        if (_refCount.fetch_sub(1, std::memory_order_relaxed) == 1)
        {
            delete this;
        }
    }

private:
    void
    init()
    {
        _refCount.store(0, std::memory_order_relaxed);
    }

    void
    deInit()
    {
    }

private:
    mutable std::atomic_uint _refCount;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

LUT_NS_END;
