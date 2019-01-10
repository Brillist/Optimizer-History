#include "libclp.h"
#include "RevSetDelta.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(clp::RevSetDelta, utl::Object);

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

RevSetDelta::RevSetDelta(rsd_t type, uint_t depth)
{
    _depth = depth;
    _type = type;

    // make hash-tables
    switch (_type)
    {
    case rsd_add:
        _addedItems = new Hashtable(false);
        _removedItems = nullptr;
        break;
    case rsd_remove:
        _addedItems = nullptr;
        _removedItems = new Hashtable(false);
        break;
    case rsd_both:
        _addedItems = new Hashtable(false);
        _removedItems = new Hashtable(false);
        break;
    default:
        ABORT();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
RevSetDelta::add(const Object* object)
{
    ASSERTD(_type != rsd_remove);
    if (_type == rsd_both)
    {
        // already removed object => add cancels remove
        if (_removedItems->remove(*object))
        {
            return;
        }
    }
    _addedItems->add(object);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
RevSetDelta::remove(const Object* object)
{
    ASSERTD(_type != rsd_add);
    if (_type == rsd_both)
    {
        // already added object => remove cancels add
        if (_addedItems->remove(*object))
        {
            return;
        }
    }
    _removedItems->add(object);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
RevSetDelta::deInit()
{
    delete _addedItems;
    delete _removedItems;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_END;
