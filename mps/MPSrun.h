#ifndef MPS_MPSRUN_H
#define MPS_MPSRUN_H

//////////////////////////////////////////////////////////////////////////////

#include <mrp/WorkOrder.h>
#include <mrp/ManufactureItem.h>
#include <mps/MPSdataSet.h>
#include <mps/MPSitemPeriod.h>

//////////////////////////////////////////////////////////////////////////////

MPS_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////

/**
   Manage a MPS run.

   \author Joe Zhou
*/

//////////////////////////////////////////////////////////////////////////////

class MPSrun : public utl::Object
{
    UTL_CLASS_DECL(MPSrun);
    UTL_CLASS_NO_COPY;
public:
   /// \name Run Control
   //@{
   void initialize(MPSdataSet* dataSet);

   /** Run. */
   bool run();
   bool generateWorkOrders();
   mrp::WorkOrder* generateWorkOrder(
       utl::uint_t id,
       MPSitemPeriod* period,
       mrp::ManufactureItem* item,
       utl::uint_t quantity, 
       time_t releaseTime);
   //@}

   /// \name Accessors
   //@{
   /** Get MPSdataSet. */
   MPSdataSet* dataSet() const
   { return _dataSet; }
   //@}

private:
    void init();
    void deInit();
private:
    MPSdataSet* _dataSet;
    utl::uint_t _initId;
};

//////////////////////////////////////////////////////////////////////////////

MPS_NS_END;

//////////////////////////////////////////////////////////////////////////////

#endif
