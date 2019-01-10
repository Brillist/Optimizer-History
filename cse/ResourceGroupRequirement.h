#ifndef CSE_RESOURCEGROUPREQUIREMENT_H
#define CSE_RESOURCEGROUPREQUIREMENT_H

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <cls/ResourceCapPts.h>
#include <cls/DiscreteResourceRequirement.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

class JobOp;
class ResourceGroup;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Resource group requirement.

   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class ResourceGroupRequirement : public utl::Object
{
    UTL_CLASS_DECL(ResourceGroupRequirement);

public:
    /** Copy another instance. */
    virtual void copy(const utl::Object& rhs);

    virtual void
    serialize(utl::Stream& stream, utl::uint_t io, utl::uint_t mode = utl::ser_default);

    /** Get the resource-group id. */
    utl::uint_t
    resourceGroupId() const
    {
        return _resourceGroupId;
    }

    /** Get the resource-group id. */
    utl::uint_t&
    resourceGroupId()
    {
        return _resourceGroupId;
    }

    /** Get the capacity. */
    utl::uint_t
    capacity() const
    {
        return _capacity;
    }

    /** Get the capacity. */
    utl::uint_t&
    capacity()
    {
        return _capacity;
    }

    /** Get scheduled resource-id. */
    utl::uint_t
    scheduledResourceId() const
    {
        return _scheduledResourceId;
    }

    /** Get scheduled resource-id. */
    utl::uint_t&
    scheduledResourceId()
    {
        return _scheduledResourceId;
    }

    /** Get scheduled capacity. */
    utl::uint_t
    scheduledCapacity() const
    {
        return _scheduledCapacity;
    }

    /** Get scheduled capacity. */
    utl::uint_t&
    scheduledCapacity()
    {
        return _scheduledCapacity;
    }

    /** Get the cls-res-req. */
    cls::DiscreteResourceRequirement*
    clsResReq() const
    {
        return _clsResReq;
    }

    /** Get the cls-res-req. */
    cls::DiscreteResourceRequirement*&
    clsResReq()
    {
        return _clsResReq;
    }

    /** Has non-zero res/cap/pt ? */
    bool hasNonZeroCapPt(const JobOp* op, const ResourceGroup* resGroup) const;

private:
    void init();
    void
    deInit()
    {
    }

private:
    utl::uint_t _resourceGroupId;
    utl::uint_t _capacity;
    utl::uint_t _scheduledResourceId;
    utl::uint_t _scheduledCapacity;
    cls::DiscreteResourceRequirement* _clsResReq;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

typedef std::vector<ResourceGroupRequirement*> resGroupReq_vector_t;

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_END;

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
