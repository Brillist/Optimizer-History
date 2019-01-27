#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <mrp/MRPdataSet.h>
#include <cse/Job.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

MRP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Manage a MRP run.

   \author Joe Zhou
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class MRPrun : public utl::Object
{
    UTL_CLASS_DECL(MRPrun, utl::Object);
    UTL_CLASS_NO_COPY;

public:
    /// \name Run Control
    //@{
    void initialize(MRPdataSet* dataSet);

    /** Run. */
    bool run();
    //@}

    /// \name Accessors
    //@{
    /** Get MRPdataSet. */
    MRPdataSet*
    dataSet() const
    {
        return _dataSet;
    }

    /** Get MRPdataSet. */
    MRPdataSet*&
    dataSet()
    {
        return _dataSet;
    }

    /** Get dataSetOwner. */
    bool
    dataSetOwner() const
    {
        return _dataSetOwner;
    }

    /** Get dataSetOwner. */
    bool&
    dataSetOwner()
    {
        return _dataSetOwner;
    }
    //@}

private:
    /** recursively create a job tree. */
    bool createJobs(uint_t woId,
                    std::string name,
                    uint_t quantity,
                    ManufactureItem* item,
                    time_t releaseTime,
                    cse::job_set_pref_t& jobs);

    void createPurchaseOrder(uint_t woId,
                             std::string poName,
                             uint_t poQuantity,
                             PurchaseItem* item,
                             time_t releaseTime,
                             uint_t requesterId,
                             requester_type_t reqType);

    void createJobOps(cse::Job* job, ProcessPlan* plan);

    void addResourceToClevorDS(Resource* res);

    void createResourceRequirements(ProcessStep* step, cse::Job* job, cse::JobOp* op);

    void init();
    void deInit();

private:
    MRPdataSet* _dataSet;
    bool _dataSetOwner;
    // _initId is the initialId for newly created Job and Op.
    // It's designed to replace "static uint_t id = 0" in createJobs method.
    uint_t _initId;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

MRP_NS_END;
