#ifndef LUT_FUNCTOR_H
#define LUT_FUNCTOR_H

////////////////////////////////////////////////////////////////////////////////////////////////////

LUT_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Functor (abstract).

   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class Functor : public utl::Object
{
    UTL_CLASS_DECL_ABC(Functor);
    UTL_CLASS_NO_COPY;
    UTL_CLASS_DEFID;

public:
    /** Do whatever. */
    virtual void execute(void* arg = nullptr) = 0;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

LUT_NS_END;

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
