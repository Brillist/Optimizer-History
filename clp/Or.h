#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <clp/Goal.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Exclusive-Or of goals in a solution search.

   Or is a special type of Goal that is attached to a ChoicePoint.  The current search state is
   marked by the ChoicePoint, enabling the different alternative Goals to be tried (backtracking
   to the marked state after each one).

   An Or goal can be labelled by number, enabling backtracking to its ChoicePoint (continuing
   to backtrack through any intervening ChoicePoints until reaching it).

   \see FailEx
   \see Manager
   \see Manager::nextSolution
   \ingroup clp
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class Or : public Goal
{
    UTL_CLASS_DECL(Or, Goal);

public:
    using goal_vector_t = std::vector<Goal*>;
    using iterator = goal_vector_t::iterator;
    using const_iterator = goal_vector_t::const_iterator;

public:
    /** Constructor. */
    Or(Goal* g0, Goal* g1, uint_t label = uint_t_max);

    virtual void copy(const utl::Object& rhs);

    virtual void execute();

    /** Clear goal list. */
    void clear();

    /// \name Accessors (const)
    //@{
    /** Has a label? */
    bool
    isLabeled() const
    {
        return (_label != uint_t_max);
    }

    /** Get the label. */
    uint_t
    label() const
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
    choice(uint_t idx) const
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
    //@}

    /// \name Accessors (non-const)
    //@{
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
    //@}

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

private:
    goal_vector_t _choices;
    uint_t _label;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_END;
