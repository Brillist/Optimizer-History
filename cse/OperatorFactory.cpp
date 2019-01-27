#include "libcse.h"
#include "AltResMutate.h"
#include "CapMutate.h"
#include "PtMutate.h"
#include "ResCapMutate.h"
#include "JobSeqMutate.h"
#include "JobOpSeqMutate.h"
#include "OpSeqMutate.h"
#include "AltJobMutate.h"
#include "OperatorFactory.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

LUT_NS_USE;
GOP_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(cse::OperatorFactory);

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

Operator*
OperatorFactory::make(void* param, void*) const
{
    ASSERTD(param != nullptr);
    Operator* op = nullptr;
    std::string name((const char*)param);
    if (name == "AltResMutate")
    {
        op = new AltResMutate();
    }
    else if (name == "CapMutate")
    {
        op = new CapMutate();
    }
    else if (name == "PtMutate")
    {
        op = new PtMutate();
    }
    else if (name == "ResCapMutate")
    {
        op = new ResCapMutate();
    }
    else if (name == "JobSeqMutate")
    {
        op = new JobSeqMutate();
    }
    else if (name == "JobOpSeqMutate")
    {
        op = new JobOpSeqMutate();
    }
    else if (name == "OpSeqMutate")
    {
        op = new OpSeqMutate();
    }
    else if (name == "AltJobMutate")
    {
        op = new AltJobMutate();
    }
    return op;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;
