#ifndef CSE_RESOURCECAPSELECTOR_H
#define CSE_RESOURCECAPSELECTOR_H

//////////////////////////////////////////////////////////////////////////////

#include <cse/ClevorDataSet.h>
#include <cse/DiscreteResource.h>
#include <cse/Scheduler.h>
#include <cse/SchedulingContext.h>

//////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////

/**
   Select resource capacities, and generate a schedule.

   For each resource with multiple possible capacities, ResCapSelector
   chooses one of the possible capacities, then invokes another
   schedule-builder.
*/

//////////////////////////////////////////////////////////////////////////////

class ResCapSelector : public Scheduler
{
    UTL_CLASS_DECL(ResCapSelector);
public:
    virtual void copy(const utl::Object& rhs);

   virtual void setStringBase(gop::Operator* op) const;

   virtual utl::uint_t stringSize(const ClevorDataSet& dataSet) const;

   virtual void initialize(
      const gop::DataSet* dataSet = nullptr,
      utl::uint_t stringBase = 0);

   virtual void initializeInd(
      gop::Ind* ind,
      const gop::DataSet* dataSet,
      utl::RandNumGen* rng = nullptr,
      void* param = nullptr);

   //joe's debug code
   virtual void initializeRandomInd(
      gop::Ind* ind,
      const gop::DataSet* dataSet,
      utl::RandNumGen* rng = nullptr,
      void* param = nullptr);

   /** Make a schedule. */
   virtual void run(gop::Ind* ind, gop::IndBuilderContext* context) const;
private:
   typedef std::vector<cse::Resource*> res_vector_t;
private:
   void init();
   void deInit() {}

   /** Make a list of resources. */
   void setResources(const ClevorDataSet* dataSet);

   /** Select resource capacities. */
   void setSelectedResCaps(
      gop::StringInd<utl::uint_t>* ind,
      SchedulingContext* context) const;
private:
   dres_vector_t _resources;
};

//////////////////////////////////////////////////////////////////////////////

CSE_NS_END;

//////////////////////////////////////////////////////////////////////////////

#endif
