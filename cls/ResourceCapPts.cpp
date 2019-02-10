#include "libcls.h"
#include <libutl/BufferedFDstream.h>
#include "BrkActivity.h"
#include "CompositeResource.h"
#include "ESboundTimetable.h"
#include "IntActivity.h"
#include "LFboundTimetable.h"
#include "ResourceCapPts.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef DEBUG
#define DEBUG_UNIT
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;
CLP_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// CapPt //////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

CapPt::CapPt(uint_t cap, uint_t pt)
{
    _cap = cap;
    _pt = pt;
    _object = nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
CapPt::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(CapPt));
    const CapPt& cp = (const CapPt&)rhs;
    Object::copy(cp);
    _cap = cp._cap;
    _pt = cp._pt;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int
CapPt::compare(const Object& rhs) const
{
    if (!rhs.isA(CapPt))
    {
        return Object::compare(rhs);
    }
    // only compare pt, not cap. -Joe
    const CapPt& cp = (const CapPt&)rhs;
    return utl::compare(_pt, cp._pt);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
CapPt::init()
{
    _cap = _pt = uint_t_max;
    _object = nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
CapPt::deInit()
{
    delete _object;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// CapPtOrderingCap ///////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

class CapPtOrderingCap : public Ordering
{
    UTL_CLASS_DECL(CapPtOrderingCap, Ordering);
    UTL_CLASS_DEFID;

public:
    virtual int cmp(const Object* lhs, const Object* rhs) const;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

int
CapPtOrderingCap::cmp(const Object* lhs, const Object* rhs) const
{
    uint_t lhsCap, rhsCap;

    if (lhs->isA(Uint))
    {
        lhsCap = ((Uint*)lhs)->get();
    }
    else
    {
        ASSERTD(lhs->isA(CapPt));
        CapPt* capPt = (CapPt*)lhs;
        lhsCap = capPt->capacity();
    }

    if (rhs->isA(Uint))
    {
        rhsCap = ((Uint*)rhs)->get();
    }
    else
    {
        ASSERTD(rhs->isA(CapPt));
        CapPt* capPt = (CapPt*)rhs;
        rhsCap = capPt->capacity();
    }

    return utl::compare(lhsCap, rhsCap);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// CapPtOrderingPt ////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

class CapPtOrderingPt : public Ordering
{
    UTL_CLASS_DECL(CapPtOrderingPt, Ordering);
    UTL_CLASS_DEFID;

public:
    virtual int cmp(const Object* lhs, const Object* rhs) const;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

int
CapPtOrderingPt::cmp(const Object* lhs, const Object* rhs) const
{
    uint_t lhsPt, rhsPt;

    if (lhs->isA(Uint))
    {
        lhsPt = ((Uint*)lhs)->get();
    }
    else
    {
        ASSERTD(lhs->isA(CapPt));
        CapPt* capPt = (CapPt*)lhs;
        lhsPt = capPt->processingTime();
    }

    if (rhs->isA(Uint))
    {
        rhsPt = ((Uint*)rhs)->get();
    }
    else
    {
        ASSERTD(rhs->isA(CapPt));
        CapPt* capPt = (CapPt*)rhs;
        rhsPt = capPt->processingTime();
    }

    return utl::compare(lhsPt, rhsPt);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// CapPtHashCap ///////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

class CapPtHashCap : public HashFunction
{
    virtual HashFunction*
    clone() const
    {
        return new CapPtHashCap();
    }
    virtual size_t hash(const Object* object, size_t size) const;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

size_t
CapPtHashCap::hash(const Object* object, size_t size) const
{
    if (object->isA(Uint))
    {
        return object->hash(size);
    }

    ASSERTD(object->isA(CapPt));
    const CapPt* capPt = (const CapPt*)object;
    return (capPt->capacity() % size);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// CapPtHashPt ////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

class CapPtHashPt : public HashFunction
{
    virtual HashFunction*
    clone() const
    {
        return new CapPtHashPt();
    }
    virtual size_t hash(const Object* object, size_t size) const;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

size_t
CapPtHashPt::hash(const Object* object, size_t size) const
{
    if (object->isA(Uint))
    {
        return object->hash(size);
    }

    ASSERTD(object->isA(CapPt));
    const CapPt* capPt = (const CapPt*)object;
    return (capPt->processingTime() % size);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// ResourceCapPts /////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void
ResourceCapPts::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(ResourceCapPts));
    const ResourceCapPts& rcp = (const ResourceCapPts&)rhs;
    _resId = rcp._resId;
    _res = nullptr;
    _capPts = rcp._capPts;
    _capPtsArray.clear();
    _capPtsHTcap.excise();
    _capPtsHTpt.excise();
    _stateDepth = 0;
    _numPossible = 0;
    _selectedCapPt = nullptr;

    for (auto capPt_ : _capPts)
    {
        auto capPt = utl::cast<CapPt>(capPt_);
        _capPtsHTcap += capPt;
        _capPtsHTpt += capPt;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ResourceCapPts::serialize(Stream& stream, uint_t io, uint_t)
{
    _resId.serialize(stream, io);

    // to maintain compatibility... maybe change it later (or not)
    std::vector<uint_t> caps;
    std::vector<uint_t> pts;
    if (io == io_wr)
    {
        for (auto capPt_ : _capPts)
        {
            auto capPt = utl::cast<CapPt>(capPt_);
            caps.push_back(capPt->capacity());
            pts.push_back(capPt->processingTime());
        }
    }
    lut::serialize<uint_t>(caps, stream, io);
    lut::serialize<uint_t>(pts, stream, io);
    if (io == io_rd)
    {
        clear();
        uint_t numCapPts = caps.size();
        for (uint_t i = 0; i < numCapPts; ++i)
        {
            uint_t cap = caps[i];
            uint_t pt = pts[i];
            addCapPt(cap, pt);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ResourceCapPts::initialize(Activity* act)
{
    // init _capPtsArray
    _capPtsArray.clear();
    _capPtsArray.setIncrement(1);
    _capPtsArray.reserve(_capPts.size());
    for (auto capPt_ : _capPts)
    {
        auto capPt = utl::cast<CapPt>(capPt_);
        _capPtsArray += capPt;
    }

    // make objects
    if (act->isA(BrkActivity))
    {
        initialize((BrkActivity*)act);
    }
    else
    {
        ABORT();
        ASSERTD(act->isA(IntActivity));
        initialize((IntActivity*)act);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ResourceCapPts::clear()
{
    _capPts.clear();
    _capPtsHTcap.clear();
    _capPtsHTpt.clear();
    _selectedCapPt = nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool
ResourceCapPts::addCapPt(uint_t cap, uint_t pt)
{
    // only reject the add if the pt already exists
    // this change is for PCL
    if (findPt(pt) != nullptr)
    {
        return false;
    }

    //     utl::cout << "   res:" << _resId
    //               << ", cap:" << cap
    //               << ", pt:" << pt
    //               << utl::endlf;
    CapPt* capPt = new CapPt(cap, pt);
    _capPts += capPt;
    _capPtsHTcap += capPt;
    _capPtsHTpt += capPt;
    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ResourceCapPts::dividePtsBy(uint_t divisor)
{
    _capPtsHTcap.clear();
    _capPtsHTpt.clear();
    for (auto capPt_ : _capPts)
    {
        auto capPt = utl::cast<CapPt>(capPt_);
        capPt->processingTime() /= divisor;
        _capPtsHTcap += capPt;
        _capPtsHTpt += capPt;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

const CapPt*
ResourceCapPts::selectPt(uint_t pt) const
{
    if (_selectedCapPt != nullptr)
    {
        ASSERTD(_selectedCapPt->processingTime() == pt);
        return _selectedCapPt;
    }

    Manager* mgr = _res->manager();
    const CapPt* capPt = findPt(pt);
    ASSERTD(capPt != nullptr);
    mgr->revSet(_selectedCapPt);
    _selectedCapPt = capPt;
    return _selectedCapPt;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ResourceCapPts::decNumPossible() const
{
    saveState();
    if (--_numPossible == 0)
    {
        _drr->excludeResource(_res->id());
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

const CapPt*
ResourceCapPts::findCap(uint_t cap) const
{
    return (const CapPt*)_capPtsHTcap.find(Uint(cap));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

const CapPt*
ResourceCapPts::findPt(uint_t pt) const
{
    return (const CapPt*)_capPtsHTpt.find(Uint(pt));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool
ResourceCapPts::hasNonZeroCapPt() const
{
    for (auto capPt_ : _capPts)
    {
        auto capPt = utl::cast<CapPt>(capPt_);
        if ((capPt->capacity() != 0) && (capPt->processingTime() != 0))
        {
            return true;
        }
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ResourceCapPts::init()
{
    _resId = uint_t_max;
    _res = nullptr;
    _capPtsArray.setOwner(false);
    _capPtsHTcap.setOwner(false);
    _capPtsHTpt.setOwner(false);
    _capPtsHTcap.setOrdering(new CapPtOrderingCap());
    _capPtsHTpt.setOrdering(new CapPtOrderingPt());
    _capPtsHTcap.setHashFunction(new CapPtHashCap());
    _capPtsHTpt.setHashFunction(new CapPtHashPt());
    _selected = false;
    _selectedCapPt = nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ResourceCapPts::initialize(BrkActivity* act)
{
    ASSERTD(_res->isA(DiscreteResource));

    // create ESboundTimetable objects for all cap/pt pairs
    // and init _capPtsArray
    bool forward = act->forward();
    const IntExp& possiblePts = act->possiblePts();
    uint_t maxCap = 0;
    for (auto capPt_ : _capPts)
    {
        auto capPt = utl::cast<CapPt>(capPt_);
        ASSERTD(capPt->object() == nullptr);

        // skip impossible cap/pt
        if (!possiblePts.has(capPt->processingTime()))
        {
            continue;
        }

        maxCap = utl::max(maxCap, capPt->capacity());

        Bound* ttBound;
        if (forward)
        {
            ttBound = new ESboundTimetable(act, this, capPt, 0);
        }
        else
        {
            ttBound = new LFboundTimetable(act, this, capPt, 0);
        }

        ttBound->name() =
            "-> res-" + _resId.toString() + " (" + Uint(capPt->capacity()).toString() + ")";
        capPt->setObject(ttBound);
        ++_numPossible;
    }

    auto dres = utl::cast<DiscreteResource>(_res);
    dres->maxReqCap() += maxCap;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CLS_NS_END;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(cls::CapPt);
UTL_CLASS_IMPL(cls::ResourceCapPts);
UTL_CLASS_IMPL(cls::CapPtOrderingCap);
UTL_CLASS_IMPL(cls::CapPtOrderingPt);
