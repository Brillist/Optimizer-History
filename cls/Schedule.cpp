#include "libcls.h"
#include "CapExpMgr.h"
#include "CompositeResource.h"
#include "DiscreteResource.h"
#include "ResourceCalendarMgr.h"
#include "Schedule.h"

//////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;
CLP_NS_USE;

//////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(cls::Schedule, utl::Object);

//////////////////////////////////////////////////////////////////////////////

CLS_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////
/// SerializeResourcesOrdering ///////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

struct ResourceSerialOrdering
    : public std::binary_function<Resource*,Resource*,bool>
{
    bool operator()(const Resource* lhs, const Resource* rhs) const
    {
        if (lhs->isA(CompositeResource))
        {
            if (rhs->isA(DiscreteResource))
            {
                return true;
            }
            ASSERTD(rhs->isA(CompositeResource));
            return (lhs->id() < rhs->id());
        }
        else
        {
            ASSERTD(lhs->isA(DiscreteResource));
            if (rhs->isA(CompositeResource))
            {
                return false;
            }
            ASSERTD(rhs->isA(DiscreteResource));
            DiscreteResource* lhsDres = (DiscreteResource*)lhs;
            DiscreteResource* rhsDres = (DiscreteResource*)rhs;
            uint_t lhsNumCRids = lhsDres->crIds().size();
            uint_t rhsNumCRids = rhsDres->crIds().size();
            if (lhsNumCRids == rhsNumCRids) return (lhs->id() < rhs->id());
            return (lhsNumCRids < rhsNumCRids);
        }
    }
};

//////////////////////////////////////////////////////////////////////////////
/// Schedule /////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

Schedule::Schedule(clp::Manager* mgr)
{
    init();
    _mgr = mgr;
}

//////////////////////////////////////////////////////////////////////////////

bool
Schedule::add(Activity* act)
{
    if (_activities.find(act) != _activities.end())
    {
        return false;
    }

    utl::arrayGrow(
        _activitiesArray,
        _activitiesArraySize,
        (size_t)_activities.size() + 1);
    act->serialId() = _activities.size();
    _activitiesArray[act->serialId()] = act;
    _activities.insert(act);
    act->schedule() = this;
    return true;
}

//////////////////////////////////////////////////////////////////////////////

bool
Schedule::add(Resource* res)
{
    if (_resources.find(res) != _resources.end())
    {
        return false;
    }

    utl::arrayGrow(
        _resourcesArray,
        _resourcesArraySize,
        _resources.size() + 1);
    _resourcesArray[_resources.size()] = res;
    _resources.insert(res);
    res->schedule() = this;
    return true;
}

//////////////////////////////////////////////////////////////////////////////

void
Schedule::serializeResources()
{
    Resource** lim = _resourcesArray + _resources.size();
    std::sort(
        _resourcesArray,
        lim,
        ResourceSerialOrdering());
    uint_t serialId = 0;
    for (Resource** it = _resourcesArray; it != lim; ++it)
    {
        Resource* res = *it;
        res->serialId() = serialId++;
    }
}

//////////////////////////////////////////////////////////////////////////////

void
Schedule::init()
{
    _mgr = nullptr;
    _capExpMgr = new CapExpMgr(this);
    _calendarMgr = new ResourceCalendarMgr(this);
    _activitiesArraySize = 1024;
    _activitiesArray = new Activity*[_activitiesArraySize];
    _resourcesArraySize = 1024;
    _resourcesArray = new Resource*[_resourcesArraySize];
}

//////////////////////////////////////////////////////////////////////////////

void
Schedule::deInit()
{
    delete _capExpMgr;
    delete _calendarMgr;
    deleteCont(_activities);
    deleteCont(_resources);
    delete [] _activitiesArray;
    delete [] _resourcesArray;
}

//////////////////////////////////////////////////////////////////////////////

CLS_NS_END;
