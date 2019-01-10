#include "libcse.h"
#include <gop/ConfigEx.h>
#include <clp/FailEx.h>
#include <cls/BrkActivity.h>
#include <cse/JobSeqMutate.h>
#include <cse/JobOpSeqMutate.h>
#include <cse/RuleBasedScheduler.h>
#include <cse/SchedulingContext.h>
#include "ForwardScheduler.h"
#include <libutl/BufferedFDstream.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef DEBUG
// #define DEBUG_SEQUENCE
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef DEBUG_SEQUENCE
#undef new
#include <iomanip>
#include <libutl/gblnew_macros.h>
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;
CLP_NS_USE;
CLS_NS_USE;
GOP_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(cse::ForwardScheduler, cse::Scheduler);

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ForwardScheduler::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(ForwardScheduler));
    const ForwardScheduler& fscheduler = (const ForwardScheduler&)rhs;
    Scheduler::copy(fscheduler);
    _jobStartPosition = fscheduler._jobStartPosition;
    _opStartPosition = fscheduler._opStartPosition;
    _initOptRun = fscheduler._initOptRun;
    _sortedJobs = fscheduler._sortedJobs;
    _sortedJobOps = fscheduler._sortedJobOps;
    _sortedOps = fscheduler._sortedOps;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ForwardScheduler::run(Ind* p_ind, IndBuilderContext* p_context) const
{
    StringInd<uint_t>* ind = nullptr;
#ifdef DEBUG_SEQUENCE
    const gop::String<uint_t>* string = nullptr;
#endif
    if (p_ind != nullptr)
    {
        ASSERTD(dynamic_cast<StringInd<uint_t>*>(p_ind) != nullptr);
        ind = (StringInd<uint_t>*)p_ind;
#ifdef DEBUG_SEQUENCE
        string = ind->getString();
#endif
    }

    ASSERTD(dynamic_cast<SchedulingContext*>(p_context) != nullptr);
    SchedulingContext* context = (SchedulingContext*)p_context;

    // simple run or initialize optimization run?
    bool simpleRun =
        (_sortedJobs.size() == 0 && _sortedJobOps.size() == 0 && _sortedOps.size() == 0);
    bool initOptRun = false;
    if (!simpleRun && _initOptRun)
    {
        initOptRun = true;
        _initOptRun = false;
    }

#ifdef DEBUG_SEQUENCE
    std::ostrstream* os = new std::ostrstream();
    if (!simpleRun)
    {
        std::string tmpstring = string->toString();
        *os << tmpstring;
    }
    *os << std::endl;
#endif

    if (simpleRun || initOptRun)
    {
        FwdScheduler forwardScheduler;
        try
        {
            forwardScheduler.run(context);
        }
        catch (FailEx&)
        {
            if (initOptRun)
            {
                setDefaultInitialSeq(ind, context);
            }
            throw;
        }
        if (initOptRun)
        {
            setInitialSeq(ind, context);
        }
    }
    else // non-init optimization run
    {
        if (_jobStartPosition != uint_t_max)
        {
            JobSequenceScheduler jobSequenceScheduler;
            jobSequenceScheduler.run(context);
        }
        else
        {
            OpSequenceScheduler opSequenceScheduler;
            opSequenceScheduler.run(context);
        }
    }

#ifdef DEBUG_SEQUENCE
    *os << '\0';
    std::string tmpstring = os->str();
    std::cout << tmpstring << std::endl;
    std::ofstream to("c:\\temp\\iterString.txt", std::ios::app);
    to.write(tmpstring.c_str(), tmpstring.length());
    to.close();
    delete os;
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ForwardScheduler::setJobs(const ClevorDataSet* dataSet) const
{
    //init _sortedJobs.
    _sortedJobs.clear();
    const job_set_id_t& jobs = dataSet->jobs();
    job_set_id_t::const_iterator jobIt;
    for (job_set_id_t::const_iterator jobIt = jobs.begin(); jobIt != jobs.end(); jobIt++)
    {
        Job* job = *jobIt;
        _sortedJobs.push_back(job);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ForwardScheduler::setJobOps(const ClevorDataSet* dataSet) const
{
    deleteMapSecond(_sortedJobOps);
    const job_set_id_t& jobs = dataSet->jobs();
    job_set_id_t::const_iterator jobIt;
    for (jobIt = jobs.begin(); jobIt != jobs.end(); ++jobIt)
    {
        Job* job = *jobIt;
        jobop_vector_t* opVect = new jobop_vector_t();
        const jobop_set_id_t& ops = job->allSops();
        jobop_set_id_t::const_iterator opIt;
        for (opIt = ops.begin(); opIt != ops.end(); ++opIt)
        {
            JobOp* op = *opIt;
            opVect->push_back(op);
        }
        _sortedJobOps.insert(job_jobopvector_map_t::value_type(job, opVect));
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ForwardScheduler::setOps(const ClevorDataSet* dataSet) const
{
    _sortedOps.clear();
    const jobop_set_id_t& ops = dataSet->sops();
    jobop_set_id_t::const_iterator opIt;
    for (opIt = ops.begin(); opIt != ops.end(); opIt++)
    {
        JobOp* op = *opIt;
        _sortedOps.push_back(op);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ForwardScheduler::setInitialSeq(StringInd<uint_t>* ind, SchedulingContext* context) const
{
    //reset opSid for workorder-level scheduling
    if (_jobStartPosition != uint_t_max)
    {
        ASSERTD(_sortedJobs.size() > 0);
        job_vector_t::iterator jobIt;
        uint_t opSid = 0;
        for (jobIt = _sortedJobs.begin(); jobIt != _sortedJobs.end(); ++jobIt)
        {
            Job* job = *jobIt;
            job_jobopvector_map_t::iterator jobMapIt = _sortedJobOps.find(job);
            ASSERTD(jobMapIt != _sortedJobOps.end());
            jobop_vector_t* ops = (*jobMapIt).second;
            std::sort(ops->begin(), ops->end(), JobOpSerialIdOrdering());
            opSid = 0;
            jobop_vector_t::iterator opIt;
            for (opIt = ops->begin(); opIt != ops->end(); opIt++)
            {
                JobOp* op = *opIt;
                op->serialId() = opSid++;
            }
        }
    }
    setSequenceString(ind, context);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ForwardScheduler::setDefaultInitialSeq(StringInd<uint_t>* ind, SchedulingContext* context) const
{
    if (_jobStartPosition != uint_t_max)
    {
        //workorder-level scheduling
        std::stable_sort(_sortedJobs.begin(), _sortedJobs.end(), JobDueTimeOrdering());
        std::stable_sort(_sortedJobs.begin(), _sortedJobs.end(), JobLatenessCostDecOrdering());
        std::stable_sort(_sortedJobs.begin(), _sortedJobs.end(), JobSuccessorDepthDecOrdering());

        job_vector_t::iterator jobIt;
        uint_t jobSid = 0;
        uint_t opSid = 0;
        for (jobIt = _sortedJobs.begin(); jobIt != _sortedJobs.end(); ++jobIt)
        {
            Job* job = *jobIt;
            // for inactive jobs, set their sid=uint_t_max,
            // however their ops' sid are set normally.
            if (job->active())
                job->serialId() = jobSid++;
            job_jobopvector_map_t::iterator jobMapIt = _sortedJobOps.find(job);
            ASSERTD(jobMapIt != _sortedJobOps.end());
            jobop_vector_t* ops = (*jobMapIt).second;
            if (_config->forward())
            {
                std::stable_sort(ops->begin(), ops->end(), JobOpFDsuccessorDepthDecOrdering());
            }
            else
            {
                std::stable_sort(ops->begin(), ops->end(), JobOpBDsuccessorDepthDecOrdering());
            }
            //reset opSid=0 for workorder-level scheduling
            if (_jobStartPosition != uint_t_max)
                opSid = 0;
            jobop_vector_t::iterator opIt;
            for (opIt = ops->begin(); opIt != ops->end(); opIt++)
            {
                JobOp* op = *opIt;
                op->serialId() = opSid++;
            }
        }
    }
    else
    {
        //op-level scheduling
        ASSERT(_sortedOps.size() > 0);
        if (_config->forward())
        {
            std::stable_sort(_sortedOps.begin(), _sortedOps.end(),
                             JobOpFDsuccessorDepthDecOrdering());
        }
        else
        {
            std::stable_sort(_sortedOps.begin(), _sortedOps.end(),
                             JobOpBDsuccessorDepthDecOrdering());
        }
        uint_t opSid = 0;
        jobop_vector_t::iterator opIt;
        for (opIt = _sortedOps.begin(); opIt != _sortedOps.end(); opIt++)
        {
            JobOp* op = *opIt;
            // for op-level scheduling, it's necessary to set every inactive
            // op's sid, because it cannot be set later on.
            op->serialId() = opSid++;
        }
    }
    setSequenceString(ind, context);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ForwardScheduler::setSequenceString(StringInd<uint_t>* ind, SchedulingContext* context) const
{
    gop::String<uint_t>& string = ind->string();
    gop::String<uint_t>* newString = ind->newString();
    const ClevorDataSet* dataSet = context->clevorDataSet();
    if (_jobStartPosition != uint_t_max)
    {
        //workorder-level scheduling
        uint_t jobStrPos = _jobStartPosition;
        uint_t opStrPos = _opStartPosition;
        const job_set_id_t& jobs = dataSet->jobs();
        for (job_set_id_t::const_iterator it = jobs.begin(); it != jobs.end(); it++)
        {
            Job* job = *it;
            //             utl::cout << "job:" << job->id()
            //                       << ", jobStrPos:" << jobStrPos
            //                       << ", sid:" << job->serialId()
            //                       << utl::endl;
            string[jobStrPos] = job->serialId();
            if (newString)
                (*newString)[jobStrPos] = job->serialId();
            jobStrPos++;
            const jobop_set_id_t& ops = job->allSops();
            for (jobop_set_id_t::const_iterator opIt = ops.begin(); opIt != ops.end(); opIt++)
            {
                JobOp* op = *opIt;
                //                 utl::cout << "   op:" << op->id()
                //                           << ", opStrPos:" << opStrPos
                //                           << ", sid:" << op->serialId()
                //                           << utl::endl;
                string[opStrPos] = op->serialId();
                if (newString)
                    (*newString)[opStrPos] = op->serialId();
                opStrPos++;
            }
        }
    }
    else
    {
        //operation-level scheduling
        uint_t opStrPos = _opStartPosition;
        const jobop_set_id_t& ops = dataSet->sops();
        for (jobop_set_id_t::const_iterator it = ops.begin(); it != ops.end(); it++)
        {
            JobOp* op = *it;
            //             utl::cout << "op(job):" << op->id()
            //                       << "(" << op->job()->id() << ")"
            //                       << ", opStrPos:" << opStrPos
            //                       << ", sid:" << op->serialId()
            //                       << utl::endl;
            string[opStrPos] = op->serialId();
            if (newString)
                (*newString)[opStrPos] = op->serialId();
            opStrPos++;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ForwardScheduler::init()
{
    _initOptRun = true;
    _jobStartPosition = uint_t_max;
    _opStartPosition = uint_t_max;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
ForwardScheduler::deInit()
{
    deleteMapSecond(_sortedJobOps);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;
