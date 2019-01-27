#include "libcse.h"
#include <libutl/RBtree.h>
#include <lut/Factory.h>
#include <gop/IndEvaluator.h>
#include <gop/Operator.h>
#include <gop/Optimizer.h>
#include <gop/StringInd.h>
#include <clp/BoundCt.h>
#include <clp/CycleGroup.h>
#include <clp/ConstrainedBound.h>
#include <clp/IntExp.h>
#include <clp/RevArray.h>
#include <clp/RevSet.h>
#include <cls/ResourceCapPts.h>
#include <cse/Job.h>
#include <cse/JobOp.h>
#include <cse/Scheduler.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;
GOP_NS_USE;
CLP_NS_USE;
CLS_NS_USE;
CSE_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// UTL namespace //////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

//UTL_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_INSTANTIATE_TPL(utl::TRBtree, clp::ConstrainedBound);
UTL_INSTANTIATE_TPL(utl::TRBtree, clp::CycleGroup);
UTL_INSTANTIATE_TPL(utl::TRBtree, cse::JobOp);
UTL_INSTANTIATE_TPL(utl::TRBtree, cls::ResourceCapPts);
UTL_INSTANTIATE_TPL(utl::TRBtreeIt, clp::ConstrainedBound);
UTL_INSTANTIATE_TPL(utl::TRBtreeIt, clp::CycleGroup);
UTL_INSTANTIATE_TPL(utl::TRBtreeIt, cse::JobOp);
UTL_INSTANTIATE_TPL(utl::TRBtreeIt, cls::ResourceCapPts);

////////////////////////////////////////////////////////////////////////////////////////////////////

//UTL_NS_END;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// LUT namespace //////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

//LUT_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_INSTANTIATE_TPL(lut::Factory, gop::IndEvaluator);
UTL_INSTANTIATE_TPL(lut::Factory, gop::Operator);
UTL_INSTANTIATE_TPL(lut::Factory, gop::Optimizer);
UTL_INSTANTIATE_TPL(lut::Factory, cse::Scheduler);

////////////////////////////////////////////////////////////////////////////////////////////////////

//LUT_NS_END;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// GOP namespace //////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

//GOP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_INSTANTIATE_TPL(gop::String, uint_t);
UTL_INSTANTIATE_TPL(gop::StringInd, uint_t);

////////////////////////////////////////////////////////////////////////////////////////////////////

//GOP_NS_END;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// CLP namespace //////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

//CLP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_INSTANTIATE_TPL(clp::RevArray, uint_t);
UTL_INSTANTIATE_TPL(clp::RevArray, clp::Bound*);
UTL_INSTANTIATE_TPL(clp::RevArray, clp::BoundCt*);
UTL_INSTANTIATE_TPL(clp::RevArray, clp::IntExp*);
UTL_INSTANTIATE_TPL(clp::RevArray, cse::Job*);
UTL_INSTANTIATE_TPL(clp::RevArray, cse::JobOp*);
UTL_INSTANTIATE_TPL(clp::RevSet, clp::CycleGroup);
UTL_INSTANTIATE_TPL(clp::RevSet, clp::ConstrainedBound);

////////////////////////////////////////////////////////////////////////////////////////////////////

//CLP_NS_END;
