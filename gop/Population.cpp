#include "libgop.h"
#include <libutl/R250.h>
#include "Population.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(gop::Population, utl::Object);

////////////////////////////////////////////////////////////////////////////////////////////////////

GOP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Population::copy(const Object& rhs)
{
    ASSERTD(rhs.isA(Population));
    const Population& pop = (const Population&)rhs;
    clear();
    _inds.reserve(pop.size());
    for (Population::const_iterator it = pop.begin(); it != pop.end(); ++it)
    {
        StringInd<utl::uint_t>& ind = **it;
        add(ind);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Population::clear()
{
    // remove references
    for (uint_t i = 0; i < _inds.size(); i++)
    {
        StringInd<utl::uint_t>* ind = _inds[i];
        if (_owner)
        {
            delete ind;
        }
    }
    _inds.clear();
    _totalScore = 0.0;
    _totalFitness = 0.0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Population::add(const Population& rhs, uint_t beginIdx, uint_t endIdx)
{
    if (endIdx == uint_t_max)
    {
        endIdx = rhs.size();
    }

    Population::const_iterator endIt = rhs.begin() + endIdx;
    for (Population::const_iterator it = rhs.begin() + beginIdx; it != endIt; ++it)
    {
        StringInd<utl::uint_t>& ind = **it;
        add(ind);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Population::add(StringInd<utl::uint_t>* ind)
{
    const Population* indPop = ind->getPop();
    ASSERTD(indPop != this);
    if (indPop != nullptr)
    {
        indPop->onAddRemoveInd();
    }
    _inds.push_back(ind);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Population::set(utl::uint_t idx, StringInd<utl::uint_t>* ind)
{
    ASSERTD(idx < size());
    const Population* indPop = ind->getPop();
    ASSERTD(indPop != this);
    if (indPop != nullptr)
    {
        indPop->onAddRemoveInd();
    }
    _inds[idx] = ind;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Population::sort(IndOrdering* ordering)
{
    std::sort(_inds.begin(), _inds.end(), IndOrderingSTL(ordering));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Population::shuffle(RandNumGen* rng)
{
    bool rngGiven = (rng != nullptr);
    if (!rngGiven)
    {
        rng = new R250();
    }

    lut::shuffle(_inds, *rng);

    if (!rngGiven)
    {
        delete rng;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

double
Population::totalScore() const
{
    if (_totalScore == 0.0)
    {
        for (const_iterator it = begin(); it != end(); ++it)
        {
            const StringInd<utl::uint_t>& ind = **it;
            _totalScore += ind.getScore();
        }
    }
    return _totalScore;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

double
Population::stdDevScore() const
{
    double avg = averageScore();
    double sum = 0.0;
    uint_t i, numInds = size();
    for (i = 0; i < numInds; i++)
    {
        double diff = (_inds[i]->getScore() - avg);
        sum += (diff * diff);
    }
    return sqrt(sum / (double)numInds);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

double
Population::totalFitness() const
{
    if (_totalFitness == 0.0)
    {
        for (const_iterator it = begin(); it != end(); ++it)
        {
            const StringInd<utl::uint_t>& ind = **it;
            _totalFitness += ind.getFitness();
        }
    }
    return _totalFitness;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

double
Population::stdDevFitness() const
{
    double avg = averageFitness();
    double sum = 0.0;
    uint_t i, numInds = size();
    for (i = 0; i < numInds; i++)
    {
        double diff = (_inds[i]->getFitness() - avg);
        sum += (diff * diff);
    }
    return sqrt(sum / (double)numInds);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Population::init(bool owner)
{
    _owner = owner;
    _totalScore = 0.0;
    _totalFitness = 0.0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Population::deInit()
{
    clear();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Population::onAddRemoveInd() const
{
    _totalScore = 0.0;
    _totalFitness = 0.0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Population::onChangeScore() const
{
    _totalScore = 0.0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
Population::onChangeFitness() const
{
    _totalFitness = 0.0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

GOP_NS_END;
