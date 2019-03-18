#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <libutl/Hashtable.h>
#include <libutl/Uint.h>
#include <clp/IntVar.h>
#include <cls/ResourceCapPts.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

class BrkActivity;
class DiscreteResource;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   A BrkActivity's requirement for one or more interchangeable DiscreteResource%s.

   \see BrkActivity
   \see DiscreteResource
   \ingroup cls
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class DiscreteResourceRequirement : public utl::Object
{
    friend class BrkActivity;
    UTL_CLASS_DECL(DiscreteResourceRequirement, utl::Object);
    UTL_CLASS_NO_COPY;

public:
    /**
       Constructor.
       \param act related BrkActivity
       \param resCapPts possible `[capacity,processing-time]` combinations
    */
    DiscreteResourceRequirement(BrkActivity* act, ResourceCapPts* resCapPts);

    /**
       Constructor.
       \param act related BrkActivity
       \param resCapPts possible `[capacity,processing-time]` combinations
       \param numRequired number of required resources (default: 1)
    */
    DiscreteResourceRequirement(BrkActivity* act,
                                const utl::Collection& resCapPts,
                                uint_t numRequired = 1);

    /**
       Constructor.
       \param act related BrkActivity
       \param resCapPts possible `[capacity,processing-time]` combinations
       \param numRequiredDomain possible values for the number of required resources
    */
    DiscreteResourceRequirement(BrkActivity* act,
                                const utl::Collection& resCapPts,
                                const uint_set_t& numRequiredDomain);

    virtual int compare(const utl::Object& rhs) const;

    /** Initialize. */
    void initialize(BrkActivity* act);

    /** Get the manager. */
    clp::Manager* manager() const;

    /** Get the activity. */
    BrkActivity*
    activity() const
    {
        return _act;
    }

    /// \name ResourceCapPts
    //@{
    /** How many resource-cap-pts ? */
    uint_t
    numCapPts() const
    {
        return _rcps.size();
    }

    /** Get the resource-cap-pts at the given index. */
    const ResourceCapPts*
    resIdxCapPts(uint_t idx) const
    {
        return (const ResourceCapPts*)_rcpsArray[idx];
    }

    /** Get the one-and-only ResourceCapPts. */
    const ResourceCapPts* resCapPts() const;

    /** Find the ResourceCapPts for the given resource. */
    const ResourceCapPts*
    resCapPts(uint_t resId) const
    {
        return utl::cast<ResourceCapPts>(_rcps.find(utl::Uint(resId)));
    }

    /** Find the ResourceCapPts for the given resource. */
    ResourceCapPts*
    resCapPts(uint_t resId)
    {
        return utl::cast<ResourceCapPts>(_rcps.find(utl::Uint(resId)));
    }

    /** Get the set of ResourceCapPts. */
    const utl::Hashtable&
    resCapPtsSet() const
    {
        return _rcps;
    }
    //@}

    /// \name Resource Selection (const)
    //@{
    /** Get number of required resources. */
    const clp::IntVar&
    numRequired() const
    {
        return *_numRequired;
    }

    /** Get possible resources. */
    const clp::IntVar&
    possibleResources() const
    {
        return *_possibleResources;
    }

    /** Get selected resources. */
    const clp::IntVar&
    selectedResources() const
    {
        return *_selectedResources;
    }
    //@}

    /// \name Resource Selection (non-const)
    //@{
    /** Select number of required resources. */
    void selectNumRequired(uint_t numRequired);

    /** Select a resource. */
    bool selectResource(uint_t resId);

    /** Exclude a resource. */
    void excludeResource(uint_t resId);
    //@}

private:
    void
    init()
    {
        ABORT();
    }

    void init(BrkActivity* act, const utl::Collection& rcps, const uint_set_t& numRequiredDomain);

    void deInit();

    void getSelectedCalendarIds(uint_set_t& calendarIds);

    void selectPt(uint_t pt);

    void checkAllSelected();

    bool _selectResource(uint_t resId);

    void addTimetableBounds();

    void getAllPts(uint_set_t& pts);

    void getAllResIds(uint_set_t& resIds);

private:
    BrkActivity* _act;
    utl::Hashtable _rcps;
    utl::Array _rcpsArray;
    bool _allSelected;
    clp::IntVar* _numRequired;
    clp::IntVar* _possibleResources;
    clp::IntVar* _selectedResources;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_END;
