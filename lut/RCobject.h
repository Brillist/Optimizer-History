#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

LUT_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Reference-counted class (abstract).
   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class RCobject : public utl::Object
{
    UTL_CLASS_DECL_ABC(RCobject, utl::Object);

public:
    void
    addRef() const
    {
        _refCount.fetch_add(1, std::memory_order_relaxed);
    }

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
