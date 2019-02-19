#include "liblut.h"
#include "RBtreeNode.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL_ABC(lut::RBtreeNode);

////////////////////////////////////////////////////////////////////////////////////////////////////

LUT_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

void
RBtreeNode::init()
{
    _parent = _left = _right = nullptr;
    _color = nodecolor_black;
    ;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
RBtreeNode::deInit()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////

LUT_NS_END;