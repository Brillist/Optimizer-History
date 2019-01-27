#include "liblut.h"
#include <libutl/BufferedFDstream.h>
#include "RBtree.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL_ABC(lut::RBtree);

////////////////////////////////////////////////////////////////////////////////////////////////////

LUT_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
           |                          |
           y       rightRotate        x
          / \      ==========>       / \
         x   c                      a   y
        / \        leftRotate          / \
       a   b       <=========         b   c

note: leftRotate algorithm is from the page 266 of Thomas Cormen's book,
      "Introduction to Algorithms", 1990.
      Both leftRotate and rightRotate run in O(1) time.
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

void
RBtree::leftRotate(RBtreeNode* x)
{
    utl::cout << "in RBtree::leftRotate..." << utl::endlf;
    // step 1: set y
    RBtreeNode* y = x->right();
    // step 2: turn y's left subtree (b) into x's right subtree
    x->right() = y->left();
    if (y->left() != _leaf)
    {
        y->left()->parent() = x;
    }
    // step 3: link x's parent to y
    y->parent() = x->parent();
    if (x->parent() == nullptr)
    {
        // x has no parent
        _root = y;
    }
    else if (x == x->parent()->left())
    {
        // x is its parent's left child
        x->parent()->left() = y;
    }
    else
    {
        // x is its parent's right child
        x->parent()->right() = y;
    }
    // step 4: put x on y's left
    y->left() = x;
    x->parent() = y;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
RBtree::rightRotate(RBtreeNode* y)
{
    // step 1: set x
    RBtreeNode* x = y->left();
    // step 2: turn x's right subtree (b) into y's left subtree
    y->left() = x->right();
    if (x->right() != _leaf)
    {
        x->right()->parent() = y;
    }
    // step 3: link y's parent to x
    x->parent() = y->parent();
    if (y->parent() == nullptr)
    {
        // y has no parent
        _root = x;
    }
    else if (y == y->parent()->left())
    {
        // y is its parent's left child
        y->parent()->left() = x;
    }
    else
    {
        // y is its parent's right child
        y->parent()->right() = x;
    }
    // step 4: put y on x's right
    x->right() = y;
    y->parent() = x;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Insertion of a node into an red-black tree.

   The insertion consists of two steps:
   1. insert the node into the tree as if the tree is an ordinary binary tree.
   2. fix up the tree by recoloring nodes and performing rotations.

   note: insertNode algorithm is from the page 268 of Thomas Cormen's book,
         "Introduction to Algorithms", 1990.
         The complexity of the algorithm is O(lgn).
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

void
RBtree::insertNode(RBtreeNode* x)
{
    // step 1: insert x into the tree.
    //         It's also a two-step method for binary tree insert
    RBtreeNode* y = _root;
    RBtreeNode* z = nullptr;
    // step 1.1: find the position of x, which is between y (leaf) and z.
    while (y != nullptr && y != _leaf)
    {
        z = y;
        if (x->key() < y->key())
        {
            y = y->left();
        }
        else
        {
            y = y->right();
        }
    }
    // step 1.2: put x between y (leaf) and z
    x->parent() = z;
    if (z == nullptr)
    {
        _root = x;
    }
    else if (x->key() < z->key())
    {
        z->left() = x;
    }
    else
    {
        z->right() = x;
    }

    // step 2: fix up the modified tree by recoloring nodes and rotations
    afterInsert(x);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   afterInsert method is called by RBtree::insertNode and other insertNode
   methods of derived classes of RBtree.
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

void
RBtree::afterInsert(RBtreeNode* x)
{
    RBtreeNode* y = nullptr;
    // step 1: set x's color to red, because its children (leaf nodes)
    //         are black.
    x->color() = nodecolor_red;
    // step 2: If x's parent is red, change it to black and
    //         change its parent to red
    while (x != _root && x->parent()->color() == nodecolor_red)
    {
        if (x->parent() == x->parent()->parent()->left())
        {
            // x's parent is x's grandparent's left child
            // y is x's uncle
            y = x->parent()->parent()->right();
            // change x's uncle's color to black if it is red
            // note: the tree is still balanced, so no rotation is needed
            if (y != _leaf && y->color() == nodecolor_red)
            {
                x->parent()->color() = nodecolor_black;
                y->color() = nodecolor_black;
                x->parent()->parent()->color() = nodecolor_red;
                x = x->parent()->parent(); // x becomes its grandparent
            }
            // if y is black, we need a right rotate, because the tree
            // is unbalanced now
            else
            {
                if (x == x->parent()->right())
                {
                    x = x->parent();
                    leftRotate(x);
                }
                x->parent()->color() = nodecolor_black;
                x->parent()->parent()->color() = nodecolor_red;
                rightRotate(x->parent()->parent());
            }
        }
        else
        {
            // x's parent is x's grandparent's right child
            y = x->parent()->parent()->left();
            if (y != _leaf && y->color() == nodecolor_red)
            {
                x->parent()->color() = nodecolor_black;
                y->color() = nodecolor_black;
                x->parent()->parent()->color() = nodecolor_red;
                x = x->parent()->parent();
            }
            else
            {
                if (x == x->parent()->left())
                {
                    x = x->parent();
                    rightRotate(x);
                }
                x->parent()->color() = nodecolor_black;
                x->parent()->parent()->color() = nodecolor_red;
                leftRotate(x->parent()->parent());
            }
        }
    }
    _root->color() = nodecolor_black;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   deleteNode algorithm is from the page 273 Thomas Cormen's book, "Introduction
   to Algorithms", 1990.

   Deleting a node from a red-black tree is only slightly more complicated
   than inserting a node.

   It takes time O(lgn).
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

RBtreeNode*
RBtree::deleteNode(RBtreeNode* z)
{
    ASSERTD(z != nullptr);
    RBtreeNode* y;
    RBtreeNode* x = nullptr;
    // step 1: determine a node y to splice out.
    //         if z has two children, we splice its successor.
    // note: y cannot be nullptr, and can only have zero or one child
    if (z->left() == _leaf || z->right() == _leaf)
    {
        y = z;
    }
    else
    {
        y = successor(z);
    }
    // step 2: x is set to the non-nil child of y,
    //         or nil if y has no children
    if (y->left() != _leaf)
    {
        x = y->left();
    }
    else
    {
        x = y->right();
    }
    // step 3: y is spliced out
    x->parent() = y->parent();
    if (y->parent() == nullptr)
    {
        _root = x;
    }
    else if (y == y->parent()->left())
    {
        y->parent()->left() = x;
    }
    else
    {
        y->parent()->right() = x;
    }
    // step 4: If y is z's successor, the contents of z are moved from y to z.
    //         so y can be deleted or recycled.
    if (y != z)
    {
        z->key() = y->key();
    }
    if (y->color() == nodecolor_black)
    {
        deleteFixup(x);
    }
    // return y for recycling
    return y;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   deleteFixup restores the red-black properties to the search tree after
   deleting a node.
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

void
RBtree::deleteFixup(RBtreeNode* x)
{
    utl::cout << "In deleteFixup ... " << utl::endlf;
    while (x != _root && x->color() == nodecolor_black)
    {
        RBtreeNode* w;
        if (x == x->parent()->left())
        {
            w = x->parent()->right();
            if (w->color() == nodecolor_red)
            {
                w->color() = nodecolor_black;
                x->parent()->color() = nodecolor_red;
                leftRotate(x->parent());
                w = x->parent()->right();
            }
            if (w->left()->color() == nodecolor_black && w->right()->color() == nodecolor_black)
            {
                w->color() = nodecolor_red;
                x = x->parent();
            }
            else
            {
                if (w->right()->color() == nodecolor_black)
                {
                    w->left()->color() = nodecolor_black;
                    w->color() = nodecolor_red;
                    rightRotate(w);
                    w = x->parent()->right();
                }
                w->color() = x->parent()->color();
                x->parent()->color() = nodecolor_black;
                w->right()->color() = nodecolor_black;
                leftRotate(x->parent());
                x = _root;
            }
        }
        else // x == x->parent()->right()
        {
            w = x->parent()->left();
            if (w->color() == nodecolor_red)
            {
                w->color() = nodecolor_black;
                x->parent()->color() = nodecolor_red;
                rightRotate(x->parent());
                w = x->parent()->left();
            }
            if (w->right()->color() == nodecolor_black && w->left()->color() == nodecolor_black)
            {
                w->color() = nodecolor_red;
                x = x->parent();
            }
            else
            {
                if (w->left()->color() == nodecolor_black)
                {
                    w->right()->color() = nodecolor_black;
                    w->color() = nodecolor_red;
                    leftRotate(w);
                    w = x->parent()->left();
                }
                w->color() = x->parent()->color();
                x->parent()->color() = nodecolor_black;
                w->left()->color() = nodecolor_black;
                rightRotate(x->parent());
            }
        }
    }
    x->color() = nodecolor_black;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   minimum gives the node with the minimum key in node x's subtree.
   It will give the minimum of the whole tree if x is the root.
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

RBtreeNode*
RBtree::minimum(RBtreeNode* x)
{
    ASSERT(x != nullptr);
    ASSERT(x != _leaf);
    while (x->left() != _leaf)
    {
        x = x->left();
    }
    return x;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   maximum is symmetric to minimum.
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

RBtreeNode*
RBtree::maximum(RBtreeNode* x)
{
    ASSERT(x != nullptr);
    ASSERT(x != _leaf);
    while (x->right() != _leaf)
    {
        x = x->right();
    }
    return x;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   successor is broken into two cases.
   1) If the right subtree of node x is nonempty, then the successor of x is
      just the most node in the right subtree.
   2) If the right subtree of node x is empty and x has a successor y, then
      y is the lowest ancestor of x whose left child is also an ancestor of x.
   note: if x is the right most node, it will return nullptr from the case 2.

   The running time of successor is O(h).
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

RBtreeNode*
RBtree::successor(RBtreeNode* x)
{
    ASSERT(x != nullptr);
    ASSERT(x != _leaf);
    if (x->right() != _leaf)
    {
        return minimum(x->right());
    }
    RBtreeNode* y = x->parent();
    while (y != nullptr && x == y->right())
    {
        x = y;
        y = y->parent();
    }
    return y;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   The predecessor method is symmetric to the successor method.
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

RBtreeNode*
RBtree::predecessor(RBtreeNode* x)
{
    ASSERT(x != nullptr);
    ASSERT(x != _leaf);
    if (x->left() != _leaf)
    {
        return maximum(x->left());
    }
    RBtreeNode* y = x->parent();
    while (y != nullptr && x == y->left())
    {
        x = y;
        y = y->parent();
    }
    return y;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
RBtree::init()
{
    _root = nullptr;
    _leaf = new RBtreeNode();
    _leaf->left() = _leaf;
    _leaf->right() = _leaf;
    _leaf->color() = nodecolor_black;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
RBtree::deInit()
{
    delete _leaf;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

LUT_NS_END;
