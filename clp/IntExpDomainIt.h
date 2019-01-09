#ifndef CLP_INTEXPDOMAINIT_H
#define CLP_INTEXPDOMAINIT_H

//////////////////////////////////////////////////////////////////////////////

CLP_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////

/**
   Integer expression domain.

   \author Adam McKee
*/

//////////////////////////////////////////////////////////////////////////////

class IntExpDomainIt : public utl::Object
{
    UTL_CLASS_DECL_ABC(IntExpDomainIt);
    UTL_CLASS_DEFID;
public:
    /** Move forward. */
    virtual void next()=0;

    /** Move backward. */
    virtual void prev()=0;

    /** At end of domain? */
    bool atEnd() const
    { return (_val == utl::int_t_max); }

    /** Get current value. */
    int get() const
    { return _val; }

    /** Get current value. */
    int operator*() const
    { return _val; }
protected:
    int _val;
};

//////////////////////////////////////////////////////////////////////////////

CLP_NS_END;

//////////////////////////////////////////////////////////////////////////////

#endif
