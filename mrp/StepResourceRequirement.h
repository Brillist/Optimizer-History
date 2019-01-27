#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <libutl/Array.h>
#include <libutl/RBtree.h>
#include <cse/ResourceRequirement.h>
#include <cse/ResourceGroupRequirement.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

MRP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
  Resource requirement for a process step. 

  It is an AND requirement, not an alternative requirement. Alternative 
  requirements are handled by StepAltResCapPts.

  \author Joe Zhou
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class StepResourceRequirement : public utl::Object
{
    UTL_CLASS_DECL(StepResourceRequirement, utl::Object);

public:
    virtual void copy(const utl::Object& rhs);

    virtual void
    serialize(utl::Stream& stream, uint_t io, uint_t mode = utl::ser_default);

    /// \name Accessors
    //@{
    /** Resource id. */
    uint_t
    resourceId() const
    {
        return _resourceId;
    }

    /** Resource Id. */
    uint_t
    resourceId()
    {
        return _resourceId;
    }

    /** Capacity. */
    uint_t
    capacity() const
    {
        return _capacity;
    }

    /** Capacity. */
    uint_t&
    capacity()
    {
        return _capacity;
    }

    /** SetupId. */
    uint_t
    setupId() const
    {
        return _setupId;
    }

    /** SetupId. */
    uint_t&
    setupId()
    {
        return _setupId;
    }
    //@}

    /** Create a cse::ResourceRequirement object. */
    cse::ResourceRequirement* createResourceRequirement() const;

    /** Create a cse::ResourceGroupRequirement object. */
    cse::ResourceGroupRequirement* createResourceGroupRequirement(uint_t groupId) const;

    String toString() const;

private:
    void init();
    void deInit();

private:
    uint_t _resourceId;
    uint_t _capacity;
    uint_t _setupId;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

MRP_NS_END;
