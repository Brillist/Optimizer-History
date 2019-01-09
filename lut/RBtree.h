#ifndef LUT_RBTREE_H
#define LUT_RBTREE_H


//////////////////////////////////////////////////////////////////////////////

#include "RBtreeNode.h"

//////////////////////////////////////////////////////////////////////////////

LUT_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////

/**
   RB Tree.

   A binary tree is a red-black tree if it satisfies the following properties:
   1) Every node is either red or black.
   2) Every leaf (NIL) is black.
   3) If a node is red, then both its children are black.
   4) Every simple path from a node to a descendant leaf contains the same 
      number of black nodes.

   \author Joe Zhou
*/

//////////////////////////////////////////////////////////////////////////////

class RBtree : public utl::Object
{
    UTL_CLASS_DECL_ABC(RBtree);
public:
    virtual void leftRotate(RBtreeNode* x);

    virtual void rightRotate(RBtreeNode* y);

    virtual void insertNode(RBtreeNode* x);

    virtual void afterInsert(RBtreeNode* x);

    virtual RBtreeNode* deleteNode(RBtreeNode* x);

    virtual void deleteFixup(RBtreeNode* x);

    RBtreeNode* successor(RBtreeNode* x);

    RBtreeNode* predecessor(RBtreeNode* x);

    RBtreeNode* minimum(RBtreeNode* x);

    RBtreeNode* maximum(RBtreeNode* x);

    virtual void dumpTree(utl::Stream& os) const
    { return _root->dump(os, 0); }
private:
    void init();
    void deInit();
protected:
    RBtreeNode* _root;
    RBtreeNode* _leaf;
};

//////////////////////////////////////////////////////////////////////////////

LUT_NS_END;

//////////////////////////////////////////////////////////////////////////////

#endif
