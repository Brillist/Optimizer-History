#ifndef CSE_RESOURCEREQUIREMENT_H
#define CSE_RESOURCEREQUIREMENT_H

//////////////////////////////////////////////////////////////////////////////

#include <cls/PreferredResources.h>
#include <cls/ResourceCapPts.h>

//////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////

/**
   Resource requirement.

   \author Adam McKee
*/

//////////////////////////////////////////////////////////////////////////////

class ResourceRequirement : public utl::Object
{
    UTL_CLASS_DECL(ResourceRequirement);
public:
    /** Copy another instance. */
    virtual void copy(const utl::Object& rhs);

    virtual void serialize(
        utl::Stream& stream,
        utl::uint_t io,
        utl::uint_t mode = utl::ser_default);

    /** Get the resource id. */
    utl::uint_t resourceId() const
    { return _resourceId; }

    /** Get the resource id. */
    utl::uint_t& resourceId()
    { return _resourceId; }

    /** Get required capacity. */
    utl::uint_t capacity() const
    { return _capacity; }

    /** Get required capacity. */
    utl::uint_t& capacity()
    { return _capacity; }

    /** Get max capacity */
    utl::uint_t& maxCapacity()
    { return _maxCapacity; }

    /** Get scheduled capacity. */
    utl::uint_t scheduledCapacity() const
    { return _scheduledCapacity; }

    /** Get scheduled capacity. */
    utl::uint_t& scheduledCapacity()
    { return _scheduledCapacity; }

    /** Get begin time. */
    time_t beginTime() const
    { return _beginTime; }

    /** Get begin time. */
    time_t& beginTime()
    { return _beginTime; }

    /** Get end time. */
    time_t endTime() const
    { return _endTime; }

    /** Get end time. */
    time_t& endTime()
    { return _endTime; }

    /** Created by the system? */
    bool isSystem() const
    { return _isSystem; }

    /** Created by the system? */
    bool& isSystem()
    { return _isSystem; }

    /** Get preferred-resources list. */
    const cls::PreferredResources* preferredResources() const
    { return _preferredResources; }

    void setPreferredResources(cls::PreferredResources* pr)
    { delete _preferredResources; _preferredResources = pr; }

    /** Get the cls-res-req. */
    utl::Object* clsResReq() const
    { return _clsResReq; }

    /** Get the cls-res-req. */
    utl::Object*& clsResReq()
    { return _clsResReq; }

    /** Has non-zero cap/pt? */
    bool hasNonZeroCapPt(const cls::ResourceCapPts* resCapPts) const;
private:
    void init();
    void deInit();
private:
    utl::uint_t _resourceId;
    utl::uint_t _capacity;
    utl::uint_t _maxCapacity;
    utl::uint_t _scheduledCapacity;
    time_t _beginTime;
    time_t _endTime;
    bool _isSystem;
    cls::PreferredResources* _preferredResources;
    utl::Object* _clsResReq;
};

//////////////////////////////////////////////////////////////////////////////

typedef std::vector<ResourceRequirement*> resReq_vector_t;

//////////////////////////////////////////////////////////////////////////////

CSE_NS_END;

//////////////////////////////////////////////////////////////////////////////

#endif
