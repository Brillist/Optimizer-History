#ifndef CLP_REVSETDELTA_H
#define CLP_REVSETDELTA_H

//////////////////////////////////////////////////////////////////////////////

#include <libutl/Hashtable.h>

//////////////////////////////////////////////////////////////////////////////

CLP_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////

class Manager;

//////////////////////////////////////////////////////////////////////////////

enum rsd_t
{
    rsd_add,       /**< only add objects */
    rsd_remove,    /**< only remove objects */
    rsd_both,      /**< add and remove objects */
    rsd_undefined
};

//////////////////////////////////////////////////////////////////////////////

/**
   Delta record for reversible sets.

   \author Adam McKee
*/

//////////////////////////////////////////////////////////////////////////////

class RevSetDelta : public utl::Object
{
    UTL_CLASS_DECL(RevSetDelta);
    UTL_CLASS_NO_COPY;
public:
    /**
       Constructor.
       \param depth search-tree depth.
    */
    RevSetDelta(rsd_t type, utl::uint_t depth);

    /** Get the state-depth. */
    utl::uint_t depth() const
    { return _depth; }

    /** Record the addition of an item into the set. */
    void add(const utl::Object* object);

    /** Record the removal of an item from the set. */
    void remove(const utl::Object* object);

    /** Get a list of all added items. */
    const utl::Hashtable& addedItems() const
    { ASSERTD(_addedItems != nullptr); return *_addedItems; }

    /** Get a list of all removed items. */
    const utl::Hashtable& removedItems() const
    { ASSERTD(_removedItems != nullptr); return *_removedItems; }
private:
    void init()
    { ABORT(); }
    void deInit();
private:
    utl::uint_t _depth;
    rsd_t _type;
    utl::Hashtable* _addedItems;
    utl::Hashtable* _removedItems;
};

//////////////////////////////////////////////////////////////////////////////

CLP_NS_END;

//////////////////////////////////////////////////////////////////////////////

#endif
