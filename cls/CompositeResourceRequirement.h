#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <cls/PreferredResources.h>
#include <cls/ResourceCapPts.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

class IntActivity;
class CompositeResource;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Resource requirement.

   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class CompositeResourceRequirement : public utl::Object
{
    friend class IntActivity;
    UTL_CLASS_DECL(CompositeResourceRequirement, utl::Object);
    UTL_CLASS_NO_COPY;

public:
    typedef std::set<uint_t> uint_set_t;

public:
    /** Constructor. */
    CompositeResourceRequirement(IntActivity* act,
                                 CompositeResource* res,
                                 uint_t minCap,
                                 uint_t maxCap);

    virtual int compare(const utl::Object& rhs) const;

    /** Get the manager. */
    clp::Manager* manager() const;

    /** Get the activity. */
    IntActivity*
    activity() const
    {
        return _act;
    }

    /** Get the resource. */
    CompositeResource*
    resource() const
    {
        return _res;
    }

    /** Get the resource. */
    CompositeResource*&
    resource()
    {
        return _res;
    }

    /** Get the minimum capacity. */
    uint_t
    minCapacity() const
    {
        return _minCap;
    }

    /** Get the minimum capacity. */
    uint_t&
    minCapacity()
    {
        return _minCap;
    }

    /** Get the maximum capacity. */
    uint_t
    maxCapacity() const
    {
        return _maxCap;
    }

    /** Get the maximum capacity. */
    uint_t&
    maxCapacity()
    {
        return _maxCap;
    }

    /** Get preferred-resources list. */
    const PreferredResources*
    preferredResources() const
    {
        return _preferredResources;
    }

    /** Set preferred-resources list. */
    void
    setPreferredResources(PreferredResources* pr)
    {
        delete _preferredResources;
        _preferredResources = pr;
    }

private:
    void init();
    void deInit();

private:
    IntActivity* _act;
    CompositeResource* _res;
    uint_t _minCap;
    uint_t _maxCap;
    PreferredResources* _preferredResources;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_END;
