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
   Resource requirement.

   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class DiscreteResourceRequirement : public utl::Object
{
    friend class BrkActivity;
    UTL_CLASS_DECL(DiscreteResourceRequirement, utl::Object);
    UTL_CLASS_NO_COPY;

public:
    typedef std::set<uint_t> uint_set_t;

public:
    /** Constructor. */
    DiscreteResourceRequirement(BrkActivity* act, ResourceCapPts* resCapPts);

    /** Constructor. */
    DiscreteResourceRequirement(BrkActivity* act,
                                const utl::Collection& rcps,
                                uint_t numRequired = 1);

    /** Constructor. */
    DiscreteResourceRequirement(BrkActivity* act,
                                const utl::Collection& rcps,
                                const uint_set_t& numRequiredDomain);

    virtual int compare(const utl::Object& rhs) const;

    /** Initialize. */
    void initialize(BrkActivity* act);

    /** Select number of required resources. */
    void selectNumRequired(uint_t numRequired);

    /** Select a resource. */
    bool selectResource(uint_t resId);

    /** Exclude a resource. */
    void excludeResource(uint_t resId);

    /** Get the manager. */
    clp::Manager* manager() const;

    /** Get the activity. */
    BrkActivity*
    activity() const
    {
        return _act;
    }

    /// \name Resource-Cap-Pts
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

    /** Get the one-and-only res-cap-pts. */
    const ResourceCapPts* resCapPts() const;

    /** Find the resource-cap-pts for the given resource. */
    const ResourceCapPts*
    resCapPts(uint_t resId) const
    {
        return (ResourceCapPts*)_rcps.find(utl::Uint(resId));
    }

    /** Find the resource-cap-pts for the given resource. */
    ResourceCapPts*
    resCapPts(uint_t resId)
    {
        return (ResourceCapPts*)_rcps.find(utl::Uint(resId));
    }

    /** Get the set of res-cap-pts. */
    const utl::Hashtable&
    resCapPtsSet() const
    {
        return _rcps;
    }

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
