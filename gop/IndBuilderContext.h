#ifndef GOP_INDBUILDERCONTEXT_H
#define GOP_INDBUILDERCONTEXT_H

//////////////////////////////////////////////////////////////////////////////

GOP_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////

class DataSet;

//////////////////////////////////////////////////////////////////////////////

/**
   Individual construction context (abstract).

   \author Adam McKee
*/

//////////////////////////////////////////////////////////////////////////////

class IndBuilderContext : public utl::Object
{
    UTL_CLASS_DECL_ABC(IndBuilderContext);
public:
    /** Copy another instance. */
    virtual void copy(const utl::Object& rhs);

    /** Clear the context. */
    virtual void clear();

    /** Get the data-set. */
    virtual const DataSet* dataSet() const=0;

    /** Construction failed? */
    bool failed() const
    { return _failed; }

    /** Construction failed? */
    bool& failed()
    { return _failed; }
private:
    void init()
    { _failed = false; }
    void deInit() {}
private:
    bool _failed;
};

//////////////////////////////////////////////////////////////////////////////

GOP_NS_END;

//////////////////////////////////////////////////////////////////////////////

#endif
