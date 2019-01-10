#ifndef MRP_STEPALTRESCAPPTS_H
#define MRP_STEPALTRESCAPPTS_H

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
    UTL_CLASS_DECL(StepAltResCapPts);

public:
    virtual void copy(const utl::Object& rhs);

    virtual int compare(const utl::Object& rhs) const;

    virtual void
    serialize(utl::Stream& stream, utl::uint_t io, utl::uint_t mode = utl::ser_default);

    /// \name Accessors
    //@{
    utl::uint_t
    resourceId() const
    {
        return _resourceId;
    }

    utl::uint_t&
    resourceId()
    {
        return _resourceId;
    }

    /** Number of cap/pt pairs. */
    utl::uint_t
    numCapPts() const
    {
        return _caps.size();
    }

    /** Get cap/pt pair. */
    void
    getCapPt(utl::uint_t idx, utl::uint_t& cap, utl::uint_t& pt) const
    {
        ASSERTD(idx < numCapPts());
        cap = _caps[idx];
        pt = _pts[idx];
    }

    /** Get setupId. */
    utl::uint_t
    getSetupId(utl::uint_t idx)
    {
        ASSERTD(idx < numCapPts());
        return _setupIds[idx];
    }

    /** whether it contains the cap. */
    bool hasCap(utl::uint_t cap);

    /** find the min and max cap. */
    utl::uint_t minCap();
    utl::uint_t maxCap();

    /** add a capacity-processingTime pair. */
    void addCapPt(utl::uint_t cap,
                  utl::uint_t pt,
                  rescappt_pt_per_t ptPer,
                  utl::uint_t ptBatchSize,
                  utl::uint_t setupId);
    //@}

    /** create a cls::ResourceCapPts object. */
    cls::ResourceCapPts* createResourceCapPts(utl::uint_t quantity) const;

    utl::String toString() const;

private:
    void
    init()
    {
        _resourceId = utl::uint_t_max;
    }
    void
    deInit()
    {
    }

private:
    utl::uint_t _resourceId;
    std::vector<utl::uint_t> _caps;
    std::vector<utl::uint_t> _pts;
    std::vector<rescappt_pt_per_t> _ptPers;
    std::vector<utl::uint_t> _ptBatchSizes;
    std::vector<utl::uint_t> _setupIds;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

MRP_NS_END;

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
