#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <cse/ClevorDataSet.h>
#include <cse/DiscreteResource.h>
#include <gop/RevOperator.h>
#include <gop/StringInd.h>
#include <cse/SchedulingContext.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Change the capacity selection for a resource.

   \see ResCapSelector
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class ResCapMutate : public gop::RevOperator
{
    UTL_CLASS_DECL(ResCapMutate, gop::RevOperator);

public:
    virtual void copy(const utl::Object& rhs);

    /** Get the number of resource requirements. */
    uint_t
    numResources() const
    {
        return _resources.size();
    }

    virtual void initialize(const gop::DataSet* dataSet = nullptr);

    virtual bool execute(gop::Ind* ind = nullptr,
                         gop::IndBuilderContext* context = nullptr,
                         bool singleStep = false);

    virtual void accept();

    virtual void undo();

private:
    typedef std::vector<cse::Resource*> res_vector_t;

private:
    void init();
    void deInit();

    /** Init resources. */
    void setResources(const ClevorDataSet* dataSet);

    dres_vector_t _resources;
    gop::StringInd<uint_t>* _moveSchedule;
    uint_vector_t _minCaps;
    uint_vector_t _maxCaps;
    uint_t _moveResIdx;
    uint_t _moveResCap;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;
