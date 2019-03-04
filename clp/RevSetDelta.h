#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <libutl/Hashtable.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

class Manager;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   RevSetDelta tracked changes.
   \ingroup clp
*/
enum rsd_t
{
    rsd_add,    /**< track added objects */
    rsd_remove, /**< track removed objects */
    rsd_both,   /**< track both added and removed objects */
    rsd_undefined
};

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Record of changes in a RevSet.

   RevSetDelta records changes to a RevSet (additions, removals, or both).

   \see rsd_t
   \ingroup clp
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class RevSetDelta : public utl::Object
{
    UTL_CLASS_DECL(RevSetDelta, utl::Object);
    UTL_CLASS_NO_COPY;

public:
    /**
       Constructor.
       \param type 
       \param depth search-tree depth
    */
    RevSetDelta(rsd_t type, uint_t depth);

    /// \name Accessors (const)
    //@{
    /** Get the state-depth. */
    uint_t
    depth() const
    {
        return _depth;
    }

    /** Get a list of all added items. */
    const utl::Hashtable&
    addedItems() const
    {
        return _addedItems;
    }

    /** Get a list of all removed items. */
    const utl::Hashtable&
    removedItems() const
    {
        return _removedItems;
    }
    //@}

    /// \name Recording Changes
    //@{
    /** Record the addition of an item into a RevSet. */
    void add(const utl::Object* object);

    /** Record the removal of an item from a RevSet. */
    void remove(const utl::Object* object);
    //@}

private:
    void
    init()
    {
        ABORT();
    }
    void deInit()
    {
    }

private:
    uint_t _depth;
    rsd_t _type;
    utl::Hashtable _addedItems;
    utl::Hashtable _removedItems;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_END;
