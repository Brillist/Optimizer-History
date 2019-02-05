#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <mrp/WorkOrder.h>
#include <mrp/ManufactureItem.h>
#include <mps/MPSdataSet.h>
#include <mps/MPSitemPeriod.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

MPS_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Manage a MPS run.

   \author Joe Zhou
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class MPSrun : public utl::Object
{
    UTL_CLASS_DECL(MPSrun, utl::Object);
    UTL_CLASS_NO_COPY;

public:
    /// \name Run Control
    //@{
    void initialize(MPSdataSet* dataSet);

    /** Run. */
    bool run();
    bool generateWorkOrders();
    mrp::WorkOrder* generateWorkOrder(uint_t id,
                                      MPSitemPeriod* period,
                                      mrp::ManufactureItem* item,
                                      uint_t quantity,
                                      time_t releaseTime);
    //@}

    /// \name Accessors
    //@{
    /** Get MPSdataSet. */
    MPSdataSet*
    dataSet() const
    {
        return _dataSet;
    }
    //@}

private:
    void init();
    void deInit();

private:
    MPSdataSet* _dataSet;
    uint_t _initId;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

MPS_NS_END;
