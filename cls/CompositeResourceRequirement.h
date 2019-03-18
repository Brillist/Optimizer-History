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
   An IntActivity's requirement for a CompositeResource's capacity.

   \see IntActivity
   \see CompositeResource
   \ingroup cls
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class CompositeResourceRequirement : public utl::Object
{
    friend class IntActivity;
    UTL_CLASS_DECL(CompositeResourceRequirement, utl::Object);
    UTL_CLASS_NO_COPY;

public:
    /**
       Constructor.
       \param act IntActivity that has the requirement
       \param res the required CompositeResource
       \param minCap minimum required resource capacity
       \param maxCap maximum required resource capacity
    */
    CompositeResourceRequirement(IntActivity* act,
                                 CompositeResource* res,
                                 uint_t minCap,
                                 uint_t maxCap);

    virtual int compare(const utl::Object& rhs) const;

    /// \name Accessors (const)
    //@{
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

    /** Get the minimum capacity. */
    uint_t
    minCapacity() const
    {
        return _minCap;
    }

    /** Get the maximum capacity. */
    uint_t
    maxCapacity() const
    {
        return _maxCap;
    }

    /** Get preferred-resources list. */
    const PreferredResources*
    preferredResources() const
    {
        return _preferredResources;
    }
    //@}

    /// \name Accessors (non-const)
    //@{
    /** Set the resource. */
    void
    setResource(CompositeResource* res)
    {
        _res = res;
    }

    /** Set the minimum capacity. */
    void
    setMinCapacity(uint_t minCap)
    {
        _minCap = minCap;
    }

    /** Set the maximum capacity. */
    void
    setMaxCapacity(uint_t maxCap)
    {
        _maxCap = maxCap;
    }

    /** Set preferred-resources list. */
    void
    setPreferredResources(PreferredResources* pr)
    {
        delete _preferredResources;
        _preferredResources = pr;
    }
    //@}

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
