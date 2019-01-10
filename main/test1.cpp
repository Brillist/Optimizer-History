#include "main.h"
#include <libutl/Application.h>
#include <libutl/BufferedFDstream.h>
#include <libutl/File.h>
#include <libutl/FileStream.h>
#include <libutl/MD5.h>
#include <libutl/Time.h>
#include <cls/ResourceCalendar.h>
#include <cse/AltResMutate.h>
#include <cse/PtMutate.h>
#include <cse/CompositeResource.h>
#include <cse/DiscreteResource.h>
#include <cse/ForwardScheduler.h>
#include <cse/Job.h>
#include <cse/JobOp.h>
#include <cse/PrecedenceCt.h>
#include <cse/PtSelector.h>
#include <cse/ResCapMutate.h>
#include <cse/ResCapSelector.h>
#include <cse/ResourceGroup.h>
#include <cse/ResourceGroupRequirement.h>
#include <cse/ResourceRequirement.h>
#include <cse/ResourceSequenceList.h>
#include <cse/SchedulerConfiguration.h>
#include <cse/TotalCostEvaluator.h>
#include <cse/TotalCostEvaluatorConfiguration.h>
#include <gop/OptimizerConfiguration.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

LUT_NS_USE;
UTL_NS_USE;
CLS_NS_USE;
CSE_NS_USE;
GOP_NS_USE;
UTL_APP(Test);
UTL_MAIN_RL(Test);

////////////////////////////////////////////////////////////////////////////////////////////////////

void testCompositeResource();
void testDiscreteResource();
void testJob();
void testJobOp();
void testOptimizerConfiguration();
void testPrecedenceCt();
void testResourceCalendar();
void testResourceGroup();
void testResourceSequenceList();
void testSchedulerConfiguration();
void testTotalCostEvaluatorConfiguration();
JobOp* makeJobOp();
bool writeAndVerify(Object* object);
void write(Object* object, const Pathname& path);
bool verify(const Pathname& path, const MD5sum& sum);
MD5sum md5(Stream& stream);

////////////////////////////////////////////////////////////////////////////////////////////////////

int
Test::run(int, char**)
{
    setSerializeMode(ser_readable);

    testCompositeResource();
    testDiscreteResource();
    testJob();
    testJobOp();
    testOptimizerConfiguration();
    testPrecedenceCt();
    testResourceCalendar();
    testResourceGroup();
    testResourceSequenceList();
    testSchedulerConfiguration();
    testTotalCostEvaluatorConfiguration();

    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
testCompositeResource()
{
    cse::CompositeResource cres(1, "composite-resource-1");
    cres.resourceGroupId() = 10;
    ASSERT(writeAndVerify(cres));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
testDiscreteResource()
{
    cse::DiscreteResource dres(2, "discrete-resource-2");
    dres.sequenceId() = 0;
    dres.minCap() = 100;
    dres.maxCap() = 500;
    dres.stepCap() = 100;

    // set cost
    ResourceCost* resCost = new ResourceCost();
    resCost->resolution() = 0;
    resCost->costPerUnitHired() = 100.0;
    resCost->costPerHour() = 10.0;
    resCost->costPerDay() = 50.0;
    resCost->costPerWeek() = 100.0;
    resCost->costPerMonth() = 500.0;
    resCost->maxIdleTime() = 86400;
    dres.setCost(resCost);

    ResourceCalendar* resCal = new ResourceCalendar();
    ResourceCalendarSpan* rcs;
    rcs = new ResourceCalendarSpan(Time::today(8, 0, 0), Time::today(12, 0, 0), rcs_exception,
                                   rcss_available);
    *resCal += rcs;
    dres.setDefaultCalendar(resCal);

    ASSERT(writeAndVerify(dres));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
testJob()
{
    Job job;
    job.id() = 5;
    job.name() = "job-5";
    job.dueTime() = Time::today();
    job.opportunityCost() = 100.0;
    job.opportunityCostPeriod() = lut::period_day;
    job.latenessCost() = 55.0;
    job.latenessCostPeriod() = lut::period_week;
    job.add(makeJobOp());
    ASSERT(writeAndVerify(job));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
testJobOp()
{
    JobOp* op = makeJobOp();
    ASSERT(writeAndVerify(op));
    delete op;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
testOptimizerConfiguration()
{
    OptimizerConfiguration cf;

    cf.minIterations() = 100;
    cf.maxIterations() = 1000;
    cf.improvementGap() = 500;

    // scheduler
    Scheduler* scheduler_1 = new ResCapSelector();
    Scheduler* scheduler_2 = new PtSelector();
    Scheduler* scheduler_3 = new ForwardScheduler();
    scheduler_1->setNestedScheduler(scheduler_2);
    scheduler_2->setNestedScheduler(scheduler_3);
    cf.setIndBuilder(scheduler_1);

    // objective
    IndEvaluator* tce = new TotalCostEvaluator();
    Objective* obj = new Objective(tce, mmg_minimize);
    cf.add(obj);

    // operators
    Operator* op;
    op = new ResCapMutate(50.0);
    cf.add(op);
    op = new AltResMutate(50.0);
    cf.add(op);
    op = new PtMutate(50.0);
    cf.add(op);

    ASSERT(writeAndVerify(cf));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
testPrecedenceCt()
{
    PrecedenceCt pct(0, 1, pct_ff, 0);
    ASSERT(writeAndVerify(pct));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
testResourceCalendar()
{
    ResourceCalendar resCal;

    ResourceCalendarSpan* rcs;

    rcs = new ResourceCalendarSpan(Time::today(8, 0, 0), Time::today(12, 0, 0), rcs_exception,
                                   rcss_available);
    resCal += rcs;

    rcs = new ResourceCalendarSpan(Time::today(12, 0, 0), Time::today(13, 0, 0), rcs_exception,
                                   rcss_onBreak);
    resCal += rcs;

    ASSERT(writeAndVerify(resCal));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
testResourceGroup()
{
    ResourceGroup resGroup(0);
    resGroup.name() = "res-group-0";
    resGroup.add(0);
    resGroup.add(1);
    resGroup.add(5);
    ASSERT(writeAndVerify(resGroup));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
testResourceSequenceList()
{
    ResourceSequenceList rsl;
    rsl.id() = 1;
    rsl.add(0, 1, 1800, 100.0);
    ASSERT(writeAndVerify(rsl));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
testSchedulerConfiguration()
{
    SchedulerConfiguration sc;
    sc.originTime() = Time::today();
    sc.horizonTime() = sc.originTime() + 86400;
    sc.timeStep() = 3600;
    sc.autoFreezeDuration() = 86400;
    ASSERT(writeAndVerify(sc));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
testTotalCostEvaluatorConfiguration()
{
    TotalCostEvaluatorConfiguration cf;
    cf.interestRate() = 11.25;
    cf.interestRatePeriod() = period_month;
    cf.overheadCost() = 1000.50;
    cf.overheadCostPeriod() = period_day;
    ASSERT(writeAndVerify(cf));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

JobOp*
makeJobOp()
{
    JobOp* op = new JobOp();
    op->id() = 1;
    op->sequenceId() = 2;
    op->name() = "op-1";
    op->cost() = 100.0;
    op->processingTime() = 3600;
    op->type() = op_normal;
    op->status() = opstatus_unstarted;
    op->manuallyFrozen() = false;

    UnaryCt* uct = new UnaryCt(uct_startAt, Time::today());
    op->add(uct);

    cse::ResourceRequirement* rr = new cse::ResourceRequirement();
    rr->resourceId() = 5;
    rr->capacity() = 100;
    op->addResReq(rr);

    cse::ResourceGroupRequirement* rgr = new cse::ResourceGroupRequirement();
    rgr->resourceGroupId() = 10;
    rgr->capacity() = 200;
    op->addResGroupReq(rgr);

    ResourceCapPts* rcp = new ResourceCapPts();
    rcp->resourceId() = 75;
    rcp->addCapPt(100, 3600);
    rcp->addCapPt(200, 1800);
    op->add(rcp);

    return op;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool
writeAndVerify(Object* object)
{
    Pathname path(object->getClassName());
    path += ".txt";

    // write the object and compute MD5
    write(object, path);
    FileStream fs(path, io_rd);
    MD5sum sum = md5(fs);

    // verify the object
    return verify(path, sum);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
write(Object* object, const Pathname& path)
{
    FileStream fs(path, io_wr | fs_create | fs_trunc);
    object->serializeOutBoxed(fs);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool
verify(const Pathname& path, const MD5sum& sum)
{
    // read object from file
    FileStream inFile(path, io_rd);
    Object* object = Object::serializeInBoxed(inFile);

    // serialize again to buffer
    FileStream tmpFile("tmp.txt", io_rd | io_wr | fs_create | fs_trunc);
    object->serializeOutBoxed(tmpFile);
    delete object;
    tmpFile.rewind();
    MD5sum sum2 = md5(tmpFile);

    // compare md5-sums
    if (sum == sum2)
    {
        File("tmp.txt").unlink();
        return true;
    }
    else
    {
        return false;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

MD5sum
md5(Stream& stream)
{
    MD5 md5;
    byte_t array[512];
    for (;;)
    {
        try
        {
            uint_t numBytes = stream.read(array, 512);
            ASSERTD(numBytes > 0);
            md5.add(array, numBytes);
        }
        catch (StreamEOFex&)
        {
            break;
        }
    }
    return md5.get();
}
