#include "libmrp.h"
#include <libutl/Array.h>
#include <libutl/BufferedFDstream.h>
#include <libutl/MemStream.h>
#include <libutl/Float.h>
#include <libutl/Time.h>
#include "InventoryRecord.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;
CSE_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(mrp::InventoryRecord, utl::Object);

////////////////////////////////////////////////////////////////////////////////////////////////////

MRP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

// initialize method used after all data are populated,
// e.g. after serialization
void
InventoryRecord::initialize()
{
    //init the first Inventory Interval based on onHand
    int_t availQuantity = (int_t)_onHand - (int_t)_safetyStock;
    if (availQuantity > 0)
    {
        InventoryInterval* invInterval = new InventoryInterval();
        invInterval->startTime() = _startDate;
        invInterval->cap() = availQuantity;
        invInterval->net() = availQuantity;
        invInterval->debit() = availQuantity;
        insertNode(invInterval);

        //DEBUG CODE
        utl::cout << invInterval->toString() << utl::endlf;
    }
    inventorytransaction_set_time_t::const_iterator it;
    for (it = _invTransactions.begin(); it != _invTransactions.end(); ++it)
    {
        InventoryInterval* invInterval = new InventoryInterval(*it);
        insertNode(invInterval);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
InventoryRecord::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(InventoryRecord));
    const InventoryRecord& ir = (const InventoryRecord&)rhs;
    _id = ir._id;
    _itemId = ir._itemId;
    _name = ir._name;
    _periodType = ir._periodType;
    _numberOfPeriods = ir._numberOfPeriods;
    _safetyStock = ir._safetyStock;
    _onHand = ir._onHand;
    _startDate = ir._startDate;
    _invTransactions = ir._invTransactions;
    _intervals = ir._intervals;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int
InventoryRecord::compare(const utl::Object& rhs) const
{
    if (!rhs.isA(InventoryRecord))
    {
        return Object::compare(rhs);
    }
    const InventoryRecord& ir = (const InventoryRecord&)rhs;
    return utl::compare(_id, ir._id);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
InventoryRecord::serialize(Stream& stream, uint_t io, uint_t mode)
{
    utl::serialize(_id, stream, io);
    utl::serialize(_itemId, stream, io);
    lut::serialize(_name, stream, io);
    utl::serialize((uint_t&)_periodType, stream, io);
    utl::serialize(_numberOfPeriods, stream, io);
    utl::serialize(_safetyStock, stream, io);
    utl::serialize(_onHand, stream, io);
    utl::serialize(_startDate, stream, io);
    if (io == io_rd)
    {
        deleteCont(_invTransactions);
        Array array;
        array.serializeIn(stream);
        forEachIt(Array, array, InventoryTransaction, transaction)
            _invTransactions.insert(transaction);
        endForEach array.setOwner(false);

        // Initialize other fields
        initialize();
    }
    else
    {
        Array array(false);
        inventorytransaction_set_time_t::const_iterator it;
        for (it = _invTransactions.begin(); it != _invTransactions.end(); ++it)
        {
            array += *it;
        }
        array.serializeOut(stream);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

inline int_t
InventoryRecord::getNet(const InventoryInterval* intvl)
{
    if (intvl == nullptr)
        return 0;
    return intvl->net();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

inline void
InventoryRecord::setNet(InventoryInterval* intvl)
{
    if (intvl != nullptr && intvl != _leaf)
    {
        intvl->net() = intvl->cap() + getNet((InventoryInterval*)intvl->left()) +
                       getNet((InventoryInterval*)intvl->right());
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

inline int_t
InventoryRecord::getDebit(const InventoryInterval* intvl)
{
    if (intvl == nullptr)
        return 0;
    return intvl->debit();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

inline void
InventoryRecord::setDebit(InventoryInterval* intvl)
{
    //     if (intvl != nullptr && intvl != _leaf)
    //     {
    //         if (intvl->left() == _leaf && intvl->right() == _leaf)
    //         {
    //             intvl->debit() = intvl->cap();
    //         }
    //         else
    //         {
    //             intvl->debit() =
    //                 min(0,
    //                     min(getDebit((InventoryInterval*)intvl->left()),
    //                         min(getNet((InventoryInterval*)intvl->left()) +
    //                             intvl->cap(),
    //                             getNet((InventoryInterval*)intvl->left()) +
    //                             intvl->cap() +
    //                             getDebit((InventoryInterval*)intvl->right()))));
    //         }
    //     }
    if (intvl != nullptr)
    {
        intvl->debit() = min(0, min(getDebit((InventoryInterval*)intvl->left()),
                                    min(getNet((InventoryInterval*)intvl->left()) + intvl->cap(),
                                        getNet((InventoryInterval*)intvl->left()) + intvl->cap() +
                                            getDebit((InventoryInterval*)intvl->right()))));
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int_t
InventoryRecord::availableCapacity(time_t t)
{
    if (_root == nullptr)
        return 0;

    // step 1: find the node in which t falls
    //         plus calculate leftCap and rightCap on its way.
    int_t leftCap = 0;
    int_t rightCap = 0;
    InventoryInterval* node = (InventoryInterval*)_root;
    while (t < node->startTime() || t >= node->endTime())
    {
        if (t >= node->endTime())
        {
            leftCap += netCapIncrement(node);
            node = (InventoryInterval*)node->right();
        }
        else
        {
            rightCap = min(0, node->cap() + minDebitCapacity(node, rightCap));
            node = (InventoryInterval*)node->left();
        }
    }

    // get available capacity of the node.
    return nodeAvailableCapacity(node, leftCap, rightCap);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

inline int_t
InventoryRecord::netCapIncrement(InventoryInterval* intvl)
{
    return getNet((InventoryInterval*)intvl->left()) + intvl->cap();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

inline int_t
InventoryRecord::minDebitCapacity(InventoryInterval* intvl, int_t prevDebit)
{
    return min(0, min(getDebit((InventoryInterval*)intvl->right()),
                      getNet((InventoryInterval*)intvl->right()) + prevDebit));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

inline int_t
InventoryRecord::nodeAvailableCapacity(InventoryInterval* intvl, int_t leftCap, int_t rightCap)
{
    utl::int_t leftCapacity = leftCap + netCapIncrement(intvl);
    utl::int_t rightCapacity = minDebitCapacity(intvl, rightCap);
    return (leftCapacity + rightCapacity);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

inline void
InventoryRecord::propagateDebitChange(InventoryInterval* x)
{
    ASSERT(x != nullptr);
    InventoryInterval* y = (InventoryInterval*)x->parent();
    // propagator debit changes to root
    while (y != nullptr)
    {
        setDebit(y);
        y = (InventoryInterval*)y->parent();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

inline void
InventoryRecord::propagateNetChange(InventoryInterval* x)
{
    ASSERT(x != nullptr);
    InventoryInterval* y = (InventoryInterval*)x->parent();
    while (y != nullptr)
    {
        setNet(y);
        y = (InventoryInterval*)y->parent();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
InventoryRecord::leftRotate(lut::RBtreeNode* x)
{
    // call standard leftRoate method
    RBtree::leftRotate(x);
    InventoryInterval* y = (InventoryInterval*)x->parent();
    InventoryInterval* z = (InventoryInterval*)x;
    setNet(z);
    setDebit(z);
    setNet(y);
    propagateDebitChange(z);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
InventoryRecord::rightRotate(lut::RBtreeNode* y)
{
    // call standard rightRotate method
    RBtree::rightRotate(y);
    InventoryInterval* x = (InventoryInterval*)y->parent();
    InventoryInterval* z = (InventoryInterval*)y;
    setNet(z);
    setDebit(z);
    setNet(x);
    propagateDebitChange(z);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
InventoryRecord::insertNode(InventoryInterval* intvl)
{
    // tempory code
    intvl->left() = _leaf;
    intvl->right() = _leaf;

    InventoryInterval* y = nullptr;
    InventoryInterval* x = (InventoryInterval*)_root;
    // step 1: find the position of itvl, which is between x (leaf) and
    //         y (x's parent).
    while (x != nullptr && x != _leaf)
    {
        y = x;
        if (intvl->startTime() == x->startTime())
        {
            x->cap() += intvl->cap();
            for (jobop_set_id_t::iterator it = intvl->ops().begin(); it != intvl->ops().end(); ++it)
            {
                x->addJobOp(*it);
            }
            for (purchaseorder_set_id_t::iterator it = intvl->pos().begin();
                 it != intvl->pos().end(); ++it)
            {
                x->addPO(*it);
            }
            x->net() += intvl->cap();
            setDebit(x);
            propagateDebitChange(x);
            delete intvl;
            return;
        }
        else if (intvl->startTime() < x->startTime())
        {
            x->net() += intvl->cap();
            intvl->endTime() = min(intvl->endTime(), x->startTime());
            x = (InventoryInterval*)x->left();
        }
        else
        {
            x->net() += intvl->cap();
            x->endTime() = min(x->endTime(), intvl->startTime());
            x = (InventoryInterval*)x->right();
        }
    }

    // step 2: insert intvl between x (leaf) and y
    intvl->parent() = y;
    if (y == nullptr)
    {
        _root = intvl;
    }
    else if (intvl->startTime() < y->startTime())
    {
        y->left() = intvl;
    }
    else
    {
        y->right() = intvl;
    }
    propagateDebitChange(intvl);

    RBtree::afterInsert(intvl);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
InventoryRecord::add(InventoryTransaction* trans)
{
    _invTransactions.insert(trans);
    InventoryInterval* node = new InventoryInterval(trans);
    node->left() = _leaf;
    node->right() = _leaf;
    insertNode(node);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
InventoryRecord::add(JobOp* op)
{
    InventoryInterval* node = new InventoryInterval(op);
    node->addJobOp(op);
    node->left() = _leaf;
    node->right() = _leaf;

    insertNode(node);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
InventoryRecord::add(PurchaseOrder* po)
{
    InventoryInterval* node = new InventoryInterval(po);
    node->addPO(po);
    node->left() = _leaf;
    node->right() = _leaf;

    insertNode(node);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
InventoryRecord::deleteNode(InventoryInterval* z)
{
    ASSERTD(z != nullptr);
    // step 0: set y's predecessor's endTime
    //         this is a non-standard step
    InventoryInterval* w = (InventoryInterval*)predecessor(z);
    if (w != nullptr)
        w->endTime() = z->endTime();

    InventoryInterval* y;
    InventoryInterval* x = nullptr;
    // step 1: determine a node y to splice out.
    //         if z has two children, we splice its successor.
    // note: y cannot be nullptr, and can only have zero or one child
    if (z->left() == _leaf || z->right() == _leaf)
    {
        y = z;
    }
    else
    {
        y = (InventoryInterval*)successor(z);
    }

    // step 2: x is set to the non-nil child of y,
    //         or nil if y has no children
    if (y->left() != _leaf)
    {
        x = (InventoryInterval*)y->left();
    }
    else
    {
        x = (InventoryInterval*)y->right();
    }

    // step 3: y is spliced out
    x->parent() = y->parent();
    if (y->parent() == nullptr)
    {
        _root = x;
    }
    else if (y == (InventoryInterval*)y->parent()->left())
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
        z->copy(*y);
    }
    // step 5: propagate net and debit changes to the root
    propagateNetChange(x);
    propagateDebitChange(x);
    if (y->color() == nodecolor_black)
    {
        deleteFixup(x);
    }

    delete y;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
InventoryRecord::checkTree()
{
    // this function checks
    // 1. red-black properties of a RBtree
    // 2. net and debit calculation
    // 3. node bounds connection between neighbouring nodes

    // check red-black properties:
    // 1. Every node is either red or black
    // 2. Every leaf is black
    // 3. If a node is red, then both its children are black
    // 4. Every simple path from a node to a descendant leaf contains the same
    //    number of black nodes.
    if (_root != nullptr)
    {
        checkNode((InventoryInterval*)_root, 0);
        InventoryInterval* x = (InventoryInterval*)minimum(_root);
        while (x != nullptr && x != _leaf)
        {
            InventoryInterval* y = (InventoryInterval*)successor(x);
            if (y != nullptr && x->endTime() != y->startTime())
            {
                utl::cout << "Error: node (" << x->toString() << ") and its successor ("
                          << y->toString() << ") are not next to each other." << utl::endlf;
            }
            x = y;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
InventoryRecord::checkNode(InventoryInterval* node, uint_t numBlackNodes)
{
    static uint_t pathNum = 1;
    if (node == _leaf)
    {
        utl::cout << "# of black nodes on path " << pathNum++ << " is " << numBlackNodes + 1
                  << utl::endlf;
        return;
    }

    if (node->color() != nodecolor_red && node->color() != nodecolor_black)
    {
        utl::cout << "Error: node (" << node->toString() << ") is neither red or black"
                  << utl::endlf;
        return;
    }

    int_t net, debit;
    net = getNet(node);
    debit = getDebit(node);
    setNet(node);
    setDebit(node);
    if (net != getNet(node))
    {
        utl::cout << "Error: node (" << node->toString() << ") had a wrong net value: " << net
                  << utl::endlf;
    }
    if (debit != getDebit(node))
    {
        utl::cout << "Error: node (" << node->toString() << ") had a wrong debit value: " << debit
                  << utl::endlf;
    }

    if (node->color() == nodecolor_red)
    {
        if (node->left() != nullptr && node->left()->color() != nodecolor_black)
        {
            utl::cout << "Error: node (" << node->toString() << ") has non-black left child."
                      << utl::endlf;
        }
        if (node->right() != nullptr && node->right()->color() != nodecolor_black)
        {
            utl::cout << "Error: node (" << node->toString() << ") has non-black right child."
                      << utl::endlf;
        }
        checkNode((InventoryInterval*)node->left(), numBlackNodes);
        checkNode((InventoryInterval*)node->right(), numBlackNodes);
    }
    else
    {
        checkNode((InventoryInterval*)node->left(), numBlackNodes + 1);
        checkNode((InventoryInterval*)node->right(), numBlackNodes + 1);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

String
InventoryRecord::toString() const
{
    MemStream str;
    str << "id:" << _id << ", itemId:" << _itemId << ", name:" << _name.c_str()
        << ", periodType:" << _periodType << ", #periods:" << _numberOfPeriods
        << ", safetyStock:" << _safetyStock << ", onHand:" << _onHand
        << ", startDate:" << Time(_startDate).toString()
        << ", #transactions:" << _invTransactions.size();
    inventorytransaction_set_time_t::iterator it;
    for (it = _invTransactions.begin(); it != _invTransactions.end(); ++it)
    {
        InventoryTransaction* transaction = *it;
        str << '\n' << "    " << transaction->toString();
    }
    str << '\0';
    return String((char*)str.get());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
InventoryRecord::dumpTree() const
{
    if (_root != nullptr)
    {
        InventoryInterval* root = (InventoryInterval*)_root;
        root->dump(utl::cout, (uint_t)1);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
InventoryRecord::init()
{
    _id = uint_t_max;
    _itemId = uint_t_max;
    _periodType = inventoryperiod_undefined;
    _numberOfPeriods = 8;
    _safetyStock = 0;
    _onHand = 0;
    _startDate = -1;
    // lut::RBtree's _leaf is a RBtreeNode
    delete _leaf;
    _leaf = new InventoryInterval();
    _leaf->left() = _leaf;
    _leaf->right() = _leaf;
    _leaf->color() = nodecolor_black;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
InventoryRecord::deInit()
{
    deleteCont(_invTransactions);
    deleteCont(_intervals);
    delete _leaf;
    _leaf = nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

MRP_NS_END;
