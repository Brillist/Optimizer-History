#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

GOP_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

class DataSet;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Individual construction context (abstract).

   \ingroup gop
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class IndBuilderContext : public utl::Object
{
    UTL_CLASS_DECL_ABC(IndBuilderContext, utl::Object);

public:
    virtual void copy(const utl::Object& rhs);

    /** Clear this context. */
    virtual void clear();

    /** Get the data-set. */
    virtual const DataSet* dataSet() const = 0;

    /** Has construction failed? */
    bool
    failed() const
    {
        return _failed;
    }

    /** Indicate construction failure. */
    void
    setFailed(bool failed = true)
    {
        _failed = failed;
    }

private:
    void
    init()
    {
        _failed = false;
    }
    void
    deInit()
    {
    }

private:
    bool _failed;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

GOP_NS_END;
