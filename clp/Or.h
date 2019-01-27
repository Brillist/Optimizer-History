#ifndef CLP_OR_H
#define CLP_OR_H

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <clp/Goal.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Logical OR of goals.

   An Or goal creates a choice point (see ChoicePoint) in the search.

   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class Or : public Goal
{
    UTL_CLASS_DECL(Or, Goal);

public:
    typedef std::vector<Goal*> goal_vector_t;
    typedef goal_vector_t::iterator iterator;
    typedef goal_vector_t::const_iterator const_iterator;

public:
    /** Constructor. */
    Or(Goal* g0, Goal* g1, uint_t label = uint_t_max);

    /** Constructor. */
    Or(Goal* g0, Goal* g1, Goal* g2, uint_t label = uint_t_max);

    /** Constructor. */
    Or(Goal* g0, Goal* g1, Goal* g2, Goal* g3, uint_t label = uint_t_max);

    /** Constructor. */
    Or(Goal* g0, Goal* g1, Goal* g2, Goal* g3, Goal* g4, uint_t label = uint_t_max);

    virtual void copy(const utl::Object& rhs);

    virtual void
    execute()
    {
    }

    /** Clear goal list. */
    void clear();

    /** Has a label? */
    bool
    isLabeled() const
    {
        return (_label != uint_t_max);
    }

    /** Get the label. */
    uint_t     label() const
    {
        return _label;
    }

    /** Get the number of choices. */
    uint_t
    numChoices() const
    {
        return _choices.size();
    }

    /** Get the choice at the given index. */
    Goal*
    getChoice(uint_t idx) const
    {
        ASSERTD(idx < _choices.size());
        return _choices[idx];
    }

    /** Get begin iterator (const). */
    const_iterator
    begin() const
    {
        return _choices.begin();
    }
    /** Get end iterator (const). */
    const_iterator
    end() const
    {
        return _choices.end();
    }

    /** Get begin iterator. */
    iterator
    begin()
    {
        return _choices.begin();
    }
    /** Get end iterator. */
    iterator
    end()
    {
        return _choices.end();
    }

private:
    void
    init()
    {
        _label = uint_t_max;
    }
    void
    deInit()
    {
        clear();
    }
    void addRefs();

    goal_vector_t _choices;
    uint_t _label;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_END;

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
