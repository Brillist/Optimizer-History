#include "libgop.h"
#include "OptimizerConfiguration.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(gop::OptimizerConfiguration);

////////////////////////////////////////////////////////////////////////////////////////////////////

GOP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

void
OptimizerConfiguration::clear()
{
    deInit();
    init();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
OptimizerConfiguration::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(OptimizerConfiguration));
    const OptimizerConfiguration& cf = (const OptimizerConfiguration&)rhs;
    _minIterations = cf._minIterations;
    _maxIterations = cf._maxIterations;
    _improvementGap = cf._improvementGap;
    setInd(cf.ind());
    setIndBuilder(lut::clone(cf._indBuilder));
    _context = cf._context;
    copyVector(_objectives, cf._objectives);
    copyVector(_ops, cf._ops);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
OptimizerConfiguration::serialize(Stream& stream, uint_t io, uint_t)
{
    if (io == io_rd)
    {
        deleteCont(_objectives);
        deleteCont(_ops);
    }
    utl::serialize(_minIterations, stream, io);
    utl::serialize(_maxIterations, stream, io);
    utl::serialize(_improvementGap, stream, io);
    utl::serialize(_indBuilder, stream, io, ser_default);
    lut::serialize<Objective*>(_objectives, stream, io);
    lut::serialize<Operator*>(_ops, stream, io);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
OptimizerConfiguration::init()
{
    _minIterations = uint_t_max;
    _maxIterations = uint_t_max;
    _improvementGap = uint_t_max;
    _ind = nullptr;
    _indBuilder = nullptr;
    _context = nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
OptimizerConfiguration::deInit()
{
    delete _ind;
    delete _indBuilder;
    deleteCont(_objectives);
    deleteCont(_ops);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

GOP_NS_END;
