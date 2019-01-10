#ifndef MRP_STEPRESOURCEREQUIREMENT_H
#define MRP_STEPRESOURCEREQUIREMENT_H

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
    UTL_CLASS_DECL(StepResourceRequirement);

public:
    virtual void copy(const utl::Object& rhs);

    virtual void
    serialize(utl::Stream& stream, utl::uint_t io, utl::uint_t mode = utl::ser_default);

    /// \name Accessors
    //@{
    /** Resource id. */
    utl::uint_t
    resourceId() const
    {
        return _resourceId;
    }

    /** Resource Id. */
    utl::uint_t
    resourceId()
    {
        return _resourceId;
    }

    /** Capacity. */
    utl::uint_t
    capacity() const
    {
        return _capacity;
    }

    /** Capacity. */
    utl::uint_t&
    capacity()
    {
        return _capacity;
    }

    /** SetupId. */
    utl::uint_t
    setupId() const
    {
        return _setupId;
    }

    /** SetupId. */
    utl::uint_t&
    setupId()
    {
        return _setupId;
    }
    //@}

    /** Create a cse::ResourceRequirement object. */
    cse::ResourceRequirement* createResourceRequirement() const;

    /** Create a cse::ResourceGroupRequirement object. */
    cse::ResourceGroupRequirement* createResourceGroupRequirement(utl::uint_t groupId) const;

    utl::String toString() const;

private:
    void init();
    void deInit();

private:
    utl::uint_t _resourceId;
    utl::uint_t _capacity;
    utl::uint_t _setupId;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

MRP_NS_END;

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
