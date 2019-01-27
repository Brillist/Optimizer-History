#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Invalidatible object.

   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class Invalidatible : public utl::Object
{
    UTL_CLASS_DECL(Invalidatible, utl::Object);
    UTL_CLASS_NO_COPY;

public:
    /** Invalidate the bound. */
    virtual void invalidate();

    uint_t
    invalidateListIdx() const
    {
        return _invalidateListIdx;
    }

    uint_t&
    invalidateListIdx()
    {
        return _invalidateListIdx;
    }

private:
    void
    init()
    {
        _invalidateListIdx = uint_t_max;
        _parent = nullptr;
    }

    void
    deInit()
    {
    }

private:
    uint_t _invalidateListIdx;
    Invalidatible* _parent;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_END;
