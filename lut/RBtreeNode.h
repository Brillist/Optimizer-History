#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

LUT_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   RB tree node.

   \author Joe Zhou
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

enum nodecolor_t
{
    nodecolor_black = 0,
    nodecolor_red = 1,
};

////////////////////////////////////////////////////////////////////////////////////////////////////

class RBtreeNode : public utl::Object
{
    UTL_CLASS_DECL_ABC(RBtreeNode, utl::Object);

public:
    /// \name Accessors
    //@{
    /** Parent node. */
    utl::int_t
    key() const
    {
        return _key;
    }

    utl::int_t&
    key()
    {
        return _key;
    }

    const RBtreeNode*
    parent() const
    {
        return _parent;
    }

    /** Parent node. */
    RBtreeNode*&
    parent()
    {
        return _parent;
    }

    /** Left node. */
    const RBtreeNode*
    left() const
    {
        return _left;
    }

    /** Left node. */
    RBtreeNode*&
    left()
    {
        return _left;
    }

    /** Right node. */
    const RBtreeNode*
    right() const
    {
        return _right;
    }

    /** Right node. */
    RBtreeNode*&
    right()
    {
        return _right;
    }

    /** Color. */
    nodecolor_t
    color() const
    {
        return _color;
    }

    /** Color. */
    nodecolor_t&
    color()
    {
        return _color;
    }
    //@}

    virtual void
    dump(utl::Stream& os, uint_t level) const
    {
    }

private:
    void init();
    void deInit();

private:
    utl::int_t _key;
    RBtreeNode* _parent;
    RBtreeNode* _left;
    RBtreeNode* _right;
    nodecolor_t _color;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

LUT_NS_END;
