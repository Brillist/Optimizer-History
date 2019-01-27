#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <libutl/Array.h>
#include <libutl/RBtree.h>
#include <cls/ResourceCapPts.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

MRP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

enum rescappt_pt_per_t
{
    cappt_pt_per_piece = 0,
    cappt_pt_per_batch = 1,
    cappt_pt_per_undefined = 2
};

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Alternative resources requirement and alternative capacity & processing 
   time requirement for a process step. 

   \author Joe Zhou
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class StepAltResCapPts : public utl::Object
{
    UTL_CLASS_DECL(StepAltResCapPts, utl::Object);

public:
    virtual void copy(const utl::Object& rhs);

    virtual int compare(const utl::Object& rhs) const;

    virtual void serialize(utl::Stream& stream, uint_t io, uint_t mode = utl::ser_default);

    /// \name Accessors
    //@{
    uint_t
    resourceId() const
    {
        return _resourceId;
    }

    uint_t&
    resourceId()
    {
        return _resourceId;
    }

    /** Number of cap/pt pairs. */
    uint_t
    numCapPts() const
    {
        return _caps.size();
    }

    /** Get cap/pt pair. */
    void
    getCapPt(uint_t idx, uint_t& cap, uint_t& pt) const
    {
        ASSERTD(idx < numCapPts());
        cap = _caps[idx];
        pt = _pts[idx];
    }

    /** Get setupId. */
    uint_t
    getSetupId(uint_t idx)
    {
        ASSERTD(idx < numCapPts());
        return _setupIds[idx];
    }

    /** whether it contains the cap. */
    bool hasCap(uint_t cap);

    /** find the min and max cap. */
    uint_t minCap();
    uint_t maxCap();

    /** add a capacity-processingTime pair. */
    void
    addCapPt(uint_t cap, uint_t pt, rescappt_pt_per_t ptPer, uint_t ptBatchSize, uint_t setupId);
    //@}

    /** create a cls::ResourceCapPts object. */
    cls::ResourceCapPts* createResourceCapPts(uint_t quantity) const;

    String toString() const;

private:
    void
    init()
    {
        _resourceId = uint_t_max;
    }
    void
    deInit()
    {
    }

private:
    uint_t _resourceId;
    std::vector<uint_t> _caps;
    std::vector<uint_t> _pts;
    std::vector<rescappt_pt_per_t> _ptPers;
    std::vector<uint_t> _ptBatchSizes;
    std::vector<uint_t> _setupIds;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

MRP_NS_END;
