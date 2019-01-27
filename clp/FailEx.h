#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <libutl/Exception.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Search failure exception.

   When the search fails (domain of one or more ConstrainedVar is empty),
   FailEx is thrown, so the stack can be properly unwound, and the search
   resumed at a viable choice point if possible.

   If a label is given, the search will fail back to the most recent
   choice point that has a matching label (and a remaining sub-goal).
   Otherwise, the search will fail back to the most recent choice point
   that has a remaining sub-goal.

   \see ChoicePoint
   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class FailEx : public utl::Exception
{
    UTL_CLASS_DECL(FailEx, utl::Exception);

public:
    /**
      Constructor.
      \param label choice point label
   */
    FailEx(String* str, uint_t label = uint_t_max);

    virtual void copy(const utl::Object& rhs);

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

private:
    void init(const String* str = nullptr, uint_t label = uint_t_max);
    void deInit();

private:
    const String* _str;
    uint_t _label;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CLP_NS_END;
