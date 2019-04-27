#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <cls/DiscreteResource.h>
#include <cls/ResourceCalendar.h>
#include <cse/JobOp.h>
#include <cse/Resource.h>
#include <cse/ResourceCost.h>
#include <cse/ResourceSequenceList.h>
#include <cse/SchedulerConfiguration.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////
// ResourceSequenceRuleApplication /////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

class ResourceSequenceRuleApplication : public utl::Object
{
    UTL_CLASS_DECL(ResourceSequenceRuleApplication, utl::Object);
    UTL_CLASS_DEFID;

public:
    /** Constructor. */
    ResourceSequenceRuleApplication(const JobOp* lhsOp,
                                    const JobOp* rhsOp,
                                    const ResourceSequenceRule* rsr)
    {
        _lhsOp = lhsOp;
        _rhsOp = rhsOp;
        _rsr = rsr;
    }

    /** Copy another instance. */
    virtual void copy(const utl::Object& rhs);

    /** Get lhs-op. */
    const JobOp*
    lhsOp() const
    {
        return _lhsOp;
    }

    /** Get rhs-op. */
    const JobOp*
    rhsOp() const
    {
        return _rhsOp;
    }

    /** Get sequence-record. */
    const ResourceSequenceRule*
    rule() const
    {
        return _rsr;
    }

private:
    const JobOp* _lhsOp;
    const JobOp* _rhsOp;
    const ResourceSequenceRule* _rsr;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

using rsra_vector_t = std::vector<ResourceSequenceRuleApplication>;

////////////////////////////////////////////////////////////////////////////////////////////////////
// DiscreteResource ////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Discrete resource.

   \ingroup cls
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class DiscreteResource : public cse::Resource
{
    UTL_CLASS_DECL(DiscreteResource, cse::Resource);

public:
    /** Constructor. */
    DiscreteResource(uint_t id, const std::string& name)
        : Resource(id, name)
    {
        init();
    }

    /** Copy another instance. */
    virtual void copy(const utl::Object& rhs);

    virtual void serialize(utl::Stream& stream, uint_t io, uint_t mode = utl::ser_default);

    /** Get sequence-id. */
    uint_t
    sequenceId() const
    {
        return _sequenceId;
    }

    /** Get sequence-id. */
    uint_t&
    sequenceId()
    {
        return _sequenceId;
    }

    /** Get resource-sequence-list. */
    const ResourceSequenceList*
    sequenceList() const
    {
        return _rsl;
    }

    /** Get resource-sequence-list. */
    const ResourceSequenceList*&
    sequenceList()
    {
        return _rsl;
    }

    /** Get resource-sequence-rule-applications. */
    const rsra_vector_t&
    sequenceRuleApplications() const
    {
        return _rsras;
    }

    /** Get resource-sequence-rule-applications. */
    rsra_vector_t&
    sequenceRuleApplications()
    {
        return _rsras;
    }

    /** Get min-capacity. */
    uint_t
    minCap() const
    {
        return _minCap;
    }

    /** Get min-capacity. */
    uint_t&
    minCap()
    {
        return _minCap;
    }

    /** Get existing-capacity */
    uint_t
    existingCap() const
    {
        return _existingCap;
    }

    /** Get existing-capacity */
    uint_t&
    existingCap()
    {
        return _existingCap;
    }

    /** Get max-capacity. */
    uint_t
    maxCap() const
    {
        return _maxCap;
    }

    /** Get max-capacity. */
    uint_t&
    maxCap()
    {
        return _maxCap;
    }

    /** Get step-capacity. */
    uint_t
    stepCap() const
    {
        return _stepCap;
    }

    /** Get step-capacity. */
    uint_t&
    stepCap()
    {
        return _stepCap;
    }

    /** Get selected-capacity. */
    uint_t
    selectedCap() const
    {
        return _selectedCap;
    }

    /** Get selected-capacity. */
    uint_t&
    selectedCap()
    {
        return _selectedCap;
    }

    /** Get cost record. */
    ResourceCost*
    cost() const
    {
        return _cost;
    }

    /** Set cost record. */
    void
    setCost(ResourceCost* cost)
    {
        delete _cost;
        _cost = cost;
    }

    /** Get default calendar. */
    const cls::ResourceCalendar*
    defaultCalendar() const
    {
        return _defaultCalendar;
    }

    /** Set default calendar. */
    void
    setDefaultCalendar(cls::ResourceCalendar* cal)
    {
        delete _defaultCalendar;
        _defaultCalendar = cal;
    }

    /** Get detailed calendar. */
    const cls::ResourceCalendar*
    detailedCalendar() const
    {
        return _detailedCalendar;
    }

    /** Set detailed calendar. */
    void
    setDetailedCalendar(cls::ResourceCalendar* cal)
    {
        delete _detailedCalendar;
        _detailedCalendar = cal;
    }

    /** Make current calendar. */
    cls::ResourceCalendar* makeCurrentCalendar(const SchedulerConfiguration* config) const;

    /** Get the cls-resource. */
    cls::DiscreteResource*
    clsResource() const
    {
        return utl::cast<cls::DiscreteResource>(_clsResource);
    }

    /** Set the cls-resource. */
    void setCLSresource(cls::Resource* clsResource)
    {
        _clsResource = clsResource;
    }

protected:
    uint_t _sequenceId;
    const ResourceSequenceList* _rsl;
    rsra_vector_t _rsras;
    uint_t _minCap;
    uint_t _existingCap;
    uint_t _maxCap;
    uint_t _stepCap;
    uint_t _selectedCap;
    ResourceCost* _cost;
    cls::ResourceCalendar* _defaultCalendar;
    cls::ResourceCalendar* _detailedCalendar;

private:
    void init();
    void deInit();

    void applyDefaultCalendar(const SchedulerConfiguration* config,
                              cls::ResourceCalendar* outSched,
                              cls::ResourceCalendar* inSched) const;

    void overrideCalendar(const SchedulerConfiguration* config,
                          cls::ResourceCalendar* outSched,
                          cls::ResourceCalendar* inSched,
                          cls::rcs_t type) const;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

using dres_vector_t = std::vector<DiscreteResource*>;
using dres_set_id_t = std::set<DiscreteResource*, ResourceIdOrdering>;

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;
