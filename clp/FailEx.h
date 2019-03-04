#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <libutl/Exception.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Search failure exception.

   FailEx is thrown when the search fails, allowing the stack to be be properly unwound.
   If a label is given, the search will fail back to the most recent choice point that has a
   matching label (and a remaining sub-goal).  Otherwise, the search will fail back to the most
   recent choice point that has a remaining sub-goal (or the root choice point).

   \see ChoicePoint
   \ingroup clp
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class FailEx : public utl::Exception
{
    UTL_CLASS_DECL(FailEx, utl::Exception);

public:
    /**
      Constructor.
      \param str brief description of the reason for throwing the exception
      \param label label of backtracking choice point
    */
    FailEx(const String& str, uint_t label = uint_t_max);

    virtual void copy(const utl::Object& rhs);

    /// \name Accessors (const)
    //@{
    /** Get the string. */
    const String*
    str() const
    {
        return _str;
    }

    /** Get the label. */
    uint_t
    label() const
    {
        return _label;
    }
    //@}

private:
    void init(const String* str = nullptr, uint_t label = uint_t_max);
    void deInit();

private:
    const String* _str;
    uint_t _label;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_END;
