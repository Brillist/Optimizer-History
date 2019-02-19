#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <gop/IndBuilderContext.h>
#include <gop/IndEvaluatorConfiguration.h>
#include <gop/Score.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

GOP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Individual evaluation strategy (abstract).

   IndEvaluator provides an interface for different individual evaluation strategies.

   \ingroup gop
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class IndEvaluator : public utl::Object
{
    UTL_CLASS_DECL_ABC(IndEvaluator, utl::Object);

public:
    typedef std::map<std::string, int> string_int_map_t;

public:
    /** Get the name. */
    virtual std::string
    name() const
    {
        ABORT();
        return "";
    }

    /** Audit the next evaluation. */
    void
    auditNext() const
    {
        _audit = true;
    }

    /** Get the audit text. */
    const std::string&
    auditText() const
    {
        return _auditText;
    }

    /** Get the component scores. */
    const string_int_map_t&
    componentScores() const
    {
        return _componentScores;
    }

    /** Get a component score. */
    int getComponentScore(const std::string& componentName) const;

    /** Set a component score. */
    void setComponentScore(const std::string& componentName, int score) const;

    /** Initialize. */
    virtual void
    initialize(const IndEvaluatorConfiguration* cf)
    {
    }

    /**
       Evaluate an constructed individual.

       \return individual's score
       \param context construction context
    */
    virtual Score* eval(const IndBuilderContext* context) const = 0;

protected:
    mutable bool _audit;
    mutable std::string _auditText;

private:
    void
    init()
    {
        _audit = false;
    }
    void
    deInit()
    {
    }

private:
    mutable string_int_map_t _componentScores;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

GOP_NS_END;
