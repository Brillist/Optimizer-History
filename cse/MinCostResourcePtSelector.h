#ifndef CSE_MINCOSTRESOURCEPTSELECTOR_H
#define CSE_MINCOSTRESOURCEPTSELECTOR_H

//////////////////////////////////////////////////////////////////////////////

#include <cls/BrkActivity.h>
#include <cls/ResourceCapPts.h>
#include <cse/Scheduler.h>

//////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////

class ResourceCost;

//////////////////////////////////////////////////////////////////////////////

/**
   For each activity with multiple possible resource/processing-time
   selections, choose the cheapest one.

   \author Adam McKee
*/

//////////////////////////////////////////////////////////////////////////////

class MinCostResourcePtSelector : public Scheduler
{
   UTL_CLASS_DECL(MinCostResourcePtSelector);
   UTL_CLASS_DEFID;
public:
   virtual void run(gop::Ind* ind, gop::IndBuilderContext* context) const;
private:
   typedef std::vector<utl::uint_t> uint_vector_t;
private:
   void setSelectedResources(
      JobOp* op,
      const uint_vector_t& altResIdx) const;
};

//////////////////////////////////////////////////////////////////////////////

CSE_NS_END;

//////////////////////////////////////////////////////////////////////////////

#endif
