#ifndef CSE_OPSEQSELECTOR_H
#define CSE_OPSEQSELECTOR_H

//////////////////////////////////////////////////////////////////////////////

#include <cse/ClevorDataSet.h>
#include <cse/Scheduler.h>
#include <cse/SchedulingContext.h>

//////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////

class OpSeqSelector : public Scheduler
{
   UTL_CLASS_DECL(OpSeqSelector);
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
   //void setResources(const DataSet* dataSet);

   /** Select JobOp Sequence. */
   void setSelectedOpSeq(
	   gop::StringInd<utl::uint_t>* ind,
	   SchedulingContext* context) const;
};

//////////////////////////////////////////////////////////////////////////////

CSE_NS_END;

//////////////////////////////////////////////////////////////////////////////

#endif
