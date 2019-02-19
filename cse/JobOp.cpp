#include "libcse.h"
#include <libutl/BufferedFDstream.h>
#include <libutl/Bool.h>
#include <libutl/Time.h>
#include <cls/SchedulableBound.h>
#include <cse/CompositeResource.h>
#include <cse/ClevorDataSet.h>
#include <cse/DiscreteResource.h>
#include "JobOp.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;
CLS_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(cse::JobOp);

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

void
JobOp::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(JobOp));
    const JobOp& op = (const JobOp&)rhs;
    clearResReqs();
    bool saveResReqOwner = _resReqOwner;
    init();
    _resReqOwner = saveResReqOwner;

    _id = op._id;
    _serialId = op._serialId;
    _sequenceId = op._sequenceId;
    _name = op._name;
    _job = nullptr;
    _cost = op._cost;
    _processingTime = op._processingTime;

    _type = op._type;
    _status = op._status;

    _frozen = op._frozen;
    _manuallyFrozen = op._manuallyFrozen;

    // previous scheduling result
    _scheduledBy = op._scheduledBy;
    _scheduledProcessingTime = op._scheduledProcessingTime;
    _scheduledRemainingPt = op._scheduledRemainingPt;
    _scheduledStartTime = op._scheduledStartTime;
    _scheduledEndTime = op._scheduledEndTime;
    _scheduledResumeTime = op._scheduledResumeTime;

    // unary constraints
    deleteCont(_unaryCts);
    processUnaryCts();
    copyVector(_unaryCts, op._unaryCts);

    // resource-requirements
    if (_resReqOwner)
    {
        copyVector(_resReqs, op._resReqs);
        copyVector(_resGroupReqs, op._resGroupReqs);
    }
    else
    {
        _resReqs = op._resReqs;
        _resGroupReqs = op._resGroupReqs;
    }

    // res-cap-pts
    _resCapPts = op._resCapPts;
    _resCapPtsAdj = op._resCapPtsAdj;

    // item-requirements
    _itemReqs = op._itemReqs;

    // schedulable-ops index
    _schedulableOpsIdx = uint_t_max;

    // activity
    _act = op._act;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
JobOp::serialize(Stream& stream, uint_t io, uint_t)
{
    utl::serialize(_id, stream, io);
    utl::serialize(_sequenceId, stream, io);
    lut::serialize(_name, stream, io);
    utl::serialize(_cost, stream, io);
    utl::serialize(_processingTime, stream, io);
    utl::serialize((uint_t&)_type, stream, io);
    utl::serialize((uint_t&)_status, stream, io);
    utl::serialize(_frozen, stream, io);
    utl::serialize(_manuallyFrozen, stream, io);
    utl::serialize((uint_t&)_scheduledBy, stream, io);
    utl::serialize(_scheduledProcessingTime, stream, io);
    utl::serialize(_scheduledRemainingPt, stream, io);
    lut::serialize(_scheduledStartTime, stream, io);
    lut::serialize(_scheduledEndTime, stream, io);
    lut::serialize(_scheduledResumeTime, stream, io);

    // wipe out data if we are reading
    if (io == io_rd)
    {
        deleteCont(_unaryCts);
        clearResReqs();
        _resCapPts.clear();
        _resCapPtsAdj.clear();
    }

    // unary constraints
    lut::serialize(_unaryCts, stream, io);

    // res-reqs
    lut::serialize(_resReqs, stream, io);

    // res-group-reqs
    lut::serialize(_resGroupReqs, stream, io);

    // res-cap-pts
    _resCapPts.serialize(stream, io);

    // item-requirements (uncommment it when we do inventory system)
    //     if (io == io_rd)
    //     {
    //         deleteCont(_itemReqs);
    //         Array array;
    //         array.serializeIn(stream);
    //         for (auto itemReq_ : array)
    //         {
    //             auto itemReq = utl::cast<ItemRequirement>(itemReq_);
    //             addItemReq(itemReq);
    //         }
    //         array.setOwner(false);
    //     }
    //     else
    //     {
    //         Array array(false);
    //         for (auto itemReq : _itemReqs)
    //         {
    //             array += itemReq;
    //         }
    //         array.serializeOut(stream);
    //     }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool
JobOp::schedulable() const
{
    return hasRequirements() && (_type != op_summary) && (_status != opstatus_complete);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool
JobOp::ignorable() const
{
    // not scheduled => ignorable iff completed precedenceLag
    if (!isScheduled())
    {
        return ((_type == op_precedenceLag) && (_status == opstatus_complete));
    }

    // not complete => not ignorable
    if (_status != opstatus_complete)
    {
        return false;
    }

    // ignorable == not scheduled entirely within [origin,horizon)
    const ClevorDataSet* dataSet = this->dataSet();
    const SchedulerConfiguration* config = dataSet->schedulerConfig();
    ASSERTD(config != nullptr);
    return ((_scheduledStartTime < config->originTime()) ||
            (_scheduledEndTime > config->horizonTime()));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

const ClevorDataSet*
JobOp::dataSet() const
{
    if (_job == nullptr)
        return nullptr;
    return _job->dataSet();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
JobOp::processUnaryCts()
{
    _minStartTime = int_t_min;
    _maxStartTime = int_t_max;
    _minEndTime = int_t_min;
    _maxEndTime = int_t_max;

    unaryct_vect_t::iterator it;
    for (it = _unaryCts.begin(); it != _unaryCts.end(); ++it)
    {
        UnaryCt* uct = *it;

        switch (uct->type())
        {
        case uct_startAt:
            _minStartTime = _maxStartTime = uct->time();
            break;
        case uct_startNoSoonerThan:
            _minStartTime = utl::max(_minStartTime, uct->time());
            break;
        case uct_startNoLaterThan:
            _maxStartTime = utl::min(_maxStartTime, uct->time());
            break;
        case uct_endAt:
            _minEndTime = _maxEndTime = uct->time();
            break;
        case uct_endNoSoonerThan:
            _minEndTime = utl::max(_minEndTime, uct->time());
            break;
        case uct_endNoLaterThan:
            _maxEndTime = utl::min(_maxEndTime, uct->time());
            break;
        default:
            ABORT();
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
JobOp::clearResReqs()
{
    if (_resReqOwner)
    {
        deleteCont(_resReqs);
        deleteCont(_resGroupReqs);
    }
    else
    {
        _resReqs.clear();
        _resGroupReqs.clear();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

const ResourceCapPts*
JobOp::resCapPts(uint_t resId) const
{
    return (const ResourceCapPts*)_resCapPts.find(Uint(resId));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

ResourceCapPts*
JobOp::resCapPtsAdj(uint_t resId) const
{
    return (ResourceCapPts*)_resCapPtsAdj.find(Uint(resId));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
JobOp::setResCapPtsAdj(uint_t timeStep)
{
    _resCapPtsAdj.clear();
    resCapPts_set_t::iterator it;
    for (it = _resCapPts.begin(); it != _resCapPts.end(); ++it)
    {
        ResourceCapPts* resCapPts = *it;
        resCapPts = resCapPts->clone();
        resCapPts->dividePtsBy(timeStep);
        _resCapPtsAdj += resCapPts;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
JobOp::removeResCapPtsAdj(uint_t resId)
{
    _resCapPtsAdj.remove(Uint(resId));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
JobOp::getAllResIds(uint_set_t& resIds) const
{
    getCompositeResIds(resIds);
    getDiscreteResIds(resIds);
    getResGroupResIds(resIds);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
JobOp::unschedule()
{
    _status = opstatus_unstarted;
    _scheduledBy = sa_undefined;
    _scheduledProcessingTime = uint_t_max;
    _scheduledRemainingPt = uint_t_max;
    _scheduledStartTime = -1;
    _scheduledEndTime = -1;
    _scheduledResumeTime = -1;
    unscheduleResReqs();

    if (dataSet() != nullptr)
    {
        removeSystemResReqs();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
JobOp::unscheduleResReqs()
{
    resReq_vector_t::iterator it;
    for (it = _resReqs.begin(); it != _resReqs.end(); ++it)
    {
        ResourceRequirement* resReq = *it;
        resReq->scheduledCapacity() = uint_t_max;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
JobOp::removeImproperResReqs()
{
    // summary op can't have any res-reqs
    if (_type == op_summary)
    {
        clearResReqs();
        return;
    }

    const ClevorDataSet* dataSet = this->dataSet();
    uint_set_t resIds;
    resReq_vector_t::iterator it;
    if (_type == op_interruptible)
    {
        for (it = _resReqs.begin(); it != _resReqs.end(); ++it)
        {
            ResourceRequirement* resReq = *it;
            uint_t resId = resReq->resourceId();

            // prune system req
            if (resReq->isSystem())
            {
                resIds.insert(resId);
                continue;
            }

            // prune non-composite req
            Resource* res = dataSet->findResource(resId);
            if (!res->isA(cse::CompositeResource))
            {
                resIds.insert(resId);
            }
        }

        // prune all resource-group-reqs
        if (_resReqOwner)
        {
            deleteCont(_resGroupReqs);
        }
        else
        {
            _resGroupReqs.clear();
        }
    }
    else
    {
        // Joe: this anyNonSystem is very confusing. Sept 6,2006
        bool anyNonSystem = !_resGroupReqs.empty();
        for (it = _resReqs.begin(); it != _resReqs.end(); ++it)
        {
            ResourceRequirement* resReq = *it;
            uint_t resId = resReq->resourceId();

            if (!resReq->isSystem())
            {
                anyNonSystem = true;
            }

            // prune non-discrete req
            Resource* res = dataSet->findResource(resId);
            if (!res->isA(cse::DiscreteResource))
            {
                resIds.insert(resId);
            }
        }

        // remove unexplained system reqs
        if (!anyNonSystem)
        {
            getSystemResIds(resIds);
        }
    }

    // purge the chosen requirements
    removeResReqs(resIds);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
JobOp::removeSystemResReqs()
{
    uint_set_t resIds;
    getSystemResIds(resIds);
    removeResReqs(resIds);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool
JobOp::isScheduled() const
{
    // started op must define remaining-pt and resume-time
    if (_status == opstatus_started)
    {
        if ((_scheduledRemainingPt == 0) || (_scheduledRemainingPt == uint_t_max))
        {
            return false;
        }

        if (_scheduledResumeTime < 0)
        {
            return false;
        }
    }

    // must define scheduled-{pt,start,end}
    // must define scheduled-capacity for all res-reqs
    if ((_scheduledProcessingTime == uint_t_max) || (_scheduledStartTime < 0) ||
        (_scheduledEndTime < 0) || !resReqsScheduled())
    {
        return false;
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool
JobOp::resReqsScheduled() const
{
    resReq_vector_t::const_iterator it;
    for (it = _resReqs.begin(); it != _resReqs.end(); ++it)
    {
        ResourceRequirement* resReq = *it;
        if (_type == op_interruptible)
            continue;
        if (resReq->scheduledCapacity() == uint_t_max)
            return false;
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool
JobOp::hasRequirements() const
{
    return ((_resReqs.size() > 0) || (_resGroupReqs.size() > 0));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool
JobOp::hasCapacityRequirement() const
{
    if (!hasRequirements() || (_processingTime == 0))
        return false;

    if (_processingTime < uint_t_max)
    {
        return true;
    }

    // check for non-zero res-req
    resReq_vector_t::const_iterator rrIt;
    for (rrIt = _resReqs.begin(); rrIt != _resReqs.end(); ++rrIt)
    {
        ResourceRequirement* resReq = *rrIt;
        uint_t resId = resReq->resourceId();
        const ResourceCapPts* resCapPts = this->resCapPts(resId);
        if (resReq->hasNonZeroCapPt(resCapPts))
        {
            return true;
        }
    }

    // check for non-zero res-group-req
    const ClevorDataSet* dataSet = this->dataSet();
    resGroupReq_vector_t::const_iterator rgrIt;
    for (rgrIt = _resGroupReqs.begin(); rgrIt != _resGroupReqs.end(); ++rgrIt)
    {
        ResourceGroupRequirement* resGroupReq = *rgrIt;
        uint_t resGroupId = resGroupReq->resourceGroupId();
        const ResourceGroup* resGroup = dataSet->findResourceGroup(resGroupId);
        ASSERTD(resGroup != nullptr);
        if (resGroupReq->hasNonZeroCapPt(this, resGroup))
        {
            return true;
        }
    }

    return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool
JobOp::isFixed() const
{
    bool fixedStart = ((_minStartTime != -1) && (_minStartTime == _maxStartTime));
    bool fixedEnd = ((_minEndTime != -1) && (_minEndTime == _maxEndTime));
    return (fixedStart || fixedEnd || _manuallyFrozen);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
JobOp::init()
{
    _id = uint_t_max;
    _serialId = uint_t_max;
    _sequenceId = uint_t_max;
    _job = nullptr;
    _cost = 0.0;
    _processingTime = uint_t_max;
    _type = op_undefined;
    _status = opstatus_undefined;
    _hasHardCt = false;
    _frozen = false;
    _manuallyFrozen = false;
    _scheduledBy = sa_undefined;
    _scheduledProcessingTime = uint_t_max;
    _scheduledRemainingPt = uint_t_max;
    _scheduledStartTime = -1;
    _scheduledEndTime = -1;
    _scheduledResumeTime = -1;
    processUnaryCts();
    _resReqOwner = true;
    _schedulableOpsIdx = uint_t_max;
    _act = nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
JobOp::deInit()
{
    deleteCont(_unaryCts);
    clearResReqs();
    deleteCont(_itemReqs);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
JobOp::removeResReqs(const uint_set_t& resIds)
{
    uint_t resReqIdx = 0;
    while (resReqIdx < _resReqs.size())
    {
        ResourceRequirement* resReq = _resReqs[resReqIdx];
        uint_set_t::iterator it = resIds.find(resReq->resourceId());
        if (it == resIds.end())
        {
            ++resReqIdx;
            continue;
        }
#ifdef DEBUG
//         cerr << "op-" << _id << ": removed res-req: "
//               << resReq->resourceId() << endlf;
#endif
        delete resReq;
        _resReqs[resReqIdx] = _resReqs[_resReqs.size() - 1];
        _resReqs.pop_back();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
JobOp::getSystemResIds(uint_set_t& resIds) const
{
    resReq_vector_t::const_iterator it;
    for (it = _resReqs.begin(); it != _resReqs.end(); ++it)
    {
        ResourceRequirement* resReq = *it;
        if (resReq->isSystem())
        {
            resIds.insert(resReq->resourceId());
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
JobOp::getCompositeResIds(uint_set_t& resIds) const
{
    const ClevorDataSet* dataSet = this->dataSet();
    ASSERTD(dataSet != nullptr);

    resReq_vector_t::const_iterator it;
    for (it = _resReqs.begin(); it != _resReqs.end(); ++it)
    {
        ResourceRequirement* resReq = *it;
        Resource* res = dataSet->findResource(resReq->resourceId());
        ASSERTD(res != nullptr);
        if (dynamic_cast<CompositeResource*>(res) == nullptr)
        {
            continue;
        }
        CompositeResource* cres = (CompositeResource*)res;
        const ResourceGroup* resGroup = dataSet->findResourceGroup(cres->resourceGroupId());
        ASSERTD(resGroup != nullptr);
        addItems(resIds, resGroup->resIds());
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
JobOp::getDiscreteResIds(uint_set_t& resIds) const
{
    const ClevorDataSet* dataSet = this->dataSet();
    ASSERTD(dataSet != nullptr);

    resReq_vector_t::const_iterator it;
    for (it = _resReqs.begin(); it != _resReqs.end(); ++it)
    {
        ResourceRequirement* resReq = *it;
        Resource* res = dataSet->findResource(resReq->resourceId());
        ASSERTD(res != nullptr);
        if (dynamic_cast<DiscreteResource*>(res) == nullptr)
        {
            continue;
        }
        resIds.insert(resReq->resourceId());
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
JobOp::getResGroupResIds(uint_set_t& resIds) const
{
    const ClevorDataSet* dataSet = this->dataSet();
    ASSERTD(dataSet != nullptr);

    resGroupReq_vector_t::const_iterator it;
    for (it = _resGroupReqs.begin(); it != _resGroupReqs.end(); ++it)
    {
        ResourceGroupRequirement* resGroupReq = *it;
        uint_t resGroupId = resGroupReq->resourceGroupId();
        ResourceGroup* resGroup = dataSet->findResourceGroup(resGroupId);
        ASSERTD(resGroup != nullptr);
        addItems(resIds, resGroup->resIds());
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

String
JobOp::toString() const
{
    MemStream str;
    str << "op:" << _id << ", type:" << _type << ", status:" << _status
        << ", frozen:" << Bool(_frozen).toString() << ", st:["
        << Time(_minStartTime).toString("$yy/$m/$d-$h:$nn") << ","
        << Time(_maxStartTime).toString("$yy/$m/$d-$h:$nn") << "], et:["
        << Time(_minEndTime).toString("$yy/$m/$d-$h:$nn") << ","
        << Time(_maxEndTime).toString("$yy/$m/$d-$h:$nn") << "]";
    str << '\0';
    return String((char*)str.get());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;
