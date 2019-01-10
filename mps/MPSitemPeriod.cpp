#include "libmps.h"
#include <libutl/MemStream.h>
#include <libutl/BufferedFDstream.h>
#include <libutl/Time.h>
#include <libutl/Bool.h>
#include "MPSitem.h"
#include "MPSitemPeriod.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef DEBUG
#define DEBUG_UNIT
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(mps::MPSitemPeriod, utl::Object);

////////////////////////////////////////////////////////////////////////////////////////////////////

MPS_NS_BEGIN;

///////////////////////////////////////////////////////////////////////////////////////////////////

bool
MPSitemPeriodOrderingIncId::operator()(const MPSitemPeriod* lhs, const MPSitemPeriod* rhs) const
{
    return (lhs->id() < rhs->id());
}

///////////////////////////////////////////////////////////////////////////////////////////////////

bool
MPSitemPeriodOrderingIncItemId::operator()(const MPSitemPeriod* lhs, const MPSitemPeriod* rhs) const
{
    if (lhs->item()->id() == rhs->item()->id())
    {
        return (lhs->id() < rhs->id());
    }
    else
    {
        return (lhs->item()->id() < rhs->item()->id());
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

bool
MPSitemPeriodOrderingIncST::operator()(const MPSitemPeriod* lhs, const MPSitemPeriod* rhs) const
{
    return (lhs->startTime() < rhs->startTime());
}

///////////////////////////////////////////////////////////////////////////////////////////////////

MPSitemPeriod::MPSitemPeriod(MPSitem* item,
                             uint_t id,
                             time_t startTime,
                             time_t endTime,
                             uint_t forecast,
                             uint_t customerOrder,
                             int_t pab,
                             uint_t mpsQuantity,
                             uint_t atp,
                             mpsitemperiod_status_t status,
                             uint_t onHand)
{
    init();
    _item = item;
    _id = id;
    _startTime = startTime;
    _endTime = endTime;
    _forecast = forecast;
    _customerOrder = customerOrder;
    _pab = pab;
    _mpsQuantity = mpsQuantity;
    _atp = atp;
    _status = status;
    _onHand = onHand;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void
MPSitemPeriod::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(MPSitemPeriod));
    const MPSitemPeriod& mip = (const MPSitemPeriod&)rhs;
    _itemOwner = mip._itemOwner;
    if (_itemOwner)
    {
        _item = utl::clone(mip._item);
    }
    else
    {
        _item = mip._item;
    }
    _id = mip._id;
    _startTime = mip._startTime;
    _endTime = mip._endTime;
    _forecast = mip._forecast;
    _customerOrder = mip._customerOrder;
    _pab = mip._pab;
    _mpsQuantity = mip._mpsQuantity;
    _atp = mip._atp;
    _onHand = mip._onHand;
    _status = mip._status;

    _itemOwner = true;
    _opportunityCost = mip._opportunityCost;
    _opportunityCostPeriod = mip._opportunityCostPeriod;
    _latenessCost = mip._latenessCost;
    _latenessCostPeriod = mip._latenessCostPeriod;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
MPSitemPeriod::serialize(Stream& stream, uint_t io, uint_t mode)
{
    utl::serialize(_item->id(), stream, io);
    utl::serialize(_id, stream, io);
    lut::serialize(_startTime, stream, io);
    lut::serialize(_endTime, stream, io);
    utl::serialize(_forecast, stream, io);
    utl::serialize(_customerOrder, stream, io);
    utl::serialize(_pab, stream, io);
    utl::serialize(_mpsQuantity, stream, io);
    utl::serialize(_atp, stream, io);
    utl::serialize(_onHand, stream, io);
    utl::serialize((uint_t&)_status, stream, io);
    utl::serialize(_opportunityCost, stream, io);
    utl::serialize((uint_t&)_opportunityCostPeriod, stream, io);
    utl::serialize(_latenessCost, stream, io);
    utl::serialize((uint_t&)_latenessCostPeriod, stream, io);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
MPSitemPeriod::setItem(MPSitem* item, bool owner)
{
    if (_itemOwner)
        delete _item;
    _item = item;
    _itemOwner = owner;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
MPSitemPeriod::calculate1()
{
#ifdef DEBUG_UNIT
//     utl::cout << "Before: " << toString() << utl::endlf;
#endif
    // PAB calculation
    if (_startTime < _item->dtf())
    {
        if (_prev == nullptr)
        {
            // for current period, _onHand cannot be null
            ASSERTD(_onHand != uint_t_max);
            _pab = _onHand + _mpsQuantity - _customerOrder;
        }
        else
        {
            _pab = _prev->pab() + _mpsQuantity - _customerOrder;
        }
    }
    else
    {
        if (_prev == nullptr)
        {
            // for current period, _onHand cannot be null
            ASSERTD(_onHand != uint_t_max);
            _pab = _onHand + _mpsQuantity - max(_forecast, _customerOrder);
        }
        else
        {
            _pab = _prev->pab() + _mpsQuantity - max(_forecast, _customerOrder);
        }
        // post-process: takin care of negative pab.
        if ((_pab < 0) && _status == (periodstatus_unplanned))
        {
            uint_t extraQuantity = _item->eoq() * (1 - (_pab / (int_t)_item->eoq()));
            _mpsQuantity += extraQuantity;
            _pab += extraQuantity;
        }
    }
#ifdef DEBUG_UNIT
//     utl::cout << "After:  " << toString() << utl::endlf;
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
MPSitemPeriod::calculate2()
{
#ifdef DEBUG_UNIT
//     utl::cout << "Before:  " << toString() << utl::endlf;
#endif
    // ATP calculation
    if (_prev != nullptr && _mpsQuantity == 0)
    {
        _atp = uint_t_max;
    }
    else
    {
        int_t totalCustomerOrder = _customerOrder;
        MPSitemPeriod* next = _next;
        while ((next != nullptr) && (next->mpsQuantity() == 0))
        {
            totalCustomerOrder += next->customerOrder();
            next = next->next();
        }
        if (_prev == nullptr)
        {
            // ATP rule #1
            _atp = _onHand + _mpsQuantity - totalCustomerOrder;
        }
        else
        {
            // ATP rule #2
            int_t atp = (int_t)_mpsQuantity - totalCustomerOrder;
            if (atp < 0)
            {
                // ATP rule #3
                MPSitemPeriod* prev = _prev;
                while (prev != nullptr)
                {
                    if (prev->atp() != uint_t_max)
                    {
                        int_t prevAtp = prev->atp();
                        if (prevAtp + atp >= 0)
                        {
                            prev->atp() = prevAtp + atp;
                            _atp = 0;
                            break;
                        }
                        else
                        {
                            prev->atp() = 0;
                            atp += prevAtp;
                        }
                    }
                    prev = prev->prev();
                }
                ASSERT(prev != nullptr);
            }
            else
            {
                _atp = atp;
            }
        }
    }
#ifdef DEBUG_UNIT
//     utl::cout << "After:  " << toString() << utl::endlf;
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////

String
MPSitemPeriod::toString() const
{
    MemStream str;
    str << "MPSitemPeriod:"
        << " item:" << _item->id() << ", id:" << _id
        << ", st:" << Time(_startTime).toString("$yyyy/$m/$d")
        << ", et:" << Time(_endTime).toString("$yyyy/$m/$d") << ", fcst:" << _forecast
        << ", custOrder:" << _customerOrder << ", PAB:" << _pab << ", MPSqnty:" << _mpsQuantity
        << ", ATP:";
    if (_atp == uint_t_max)
        str << "null";
    else
        str << _atp;
    str << ", onHand:";
    if (_onHand == uint_t_max)
        str << "null";
    else
        str << _onHand;
    str << ", status:" << _status << ", itemOwner:" << Bool(_itemOwner).toString() << '\0';
    return String((char*)str.get());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
MPSitemPeriod::init()
{
    _item = new MPSitem();
    _itemOwner = true;
    _id = uint_t_max;
    _startTime = -1;
    _endTime = -1;
    _forecast = 0;
    _customerOrder = 0;
    _pab = uint_t_max;
    _mpsQuantity = 0;
    _atp = uint_t_max;
    _onHand = uint_t_max;
    _status = periodstatus_undefined;

    _prev = nullptr;
    _next = nullptr;

    _opportunityCost = 0.0;
    _opportunityCostPeriod = period_undefined;
    _latenessCost = 0.0;
    _latenessCostPeriod = period_undefined;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
MPSitemPeriod::deInit()
{
    setItem(nullptr);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

MPS_NS_END;
