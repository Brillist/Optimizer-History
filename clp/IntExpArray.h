#ifndef CLP_INTEXPARRAY_H
#define CLP_INTEXPARRAY_H

//////////////////////////////////////////////////////////////////////////////

#include <clp/IntExp.h>

//////////////////////////////////////////////////////////////////////////////

CLP_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////

/**
   Array of constrained integer expressions.

   \author Adam McKee
*/

//////////////////////////////////////////////////////////////////////////////

class IntExpArray : public utl::Object
{
    UTL_CLASS_DECL(IntExpArray);
public:
    typedef std::vector<IntExp*> intexp_vector_t;
    typedef intexp_vector_t::iterator iterator;
    typedef intexp_vector_t::const_iterator const_iterator;
public:
    /** Constructor. */
    IntExpArray(
        IntExp* v0,
        IntExp* v1);

    /** Constructor. */
    IntExpArray(
        IntExp* v0,
        IntExp* v1,
        IntExp* v2);

    /** Constructor. */
    IntExpArray(
        IntExp* v0,
        IntExp* v1,
        IntExp* v2,
        IntExp* v3);

    /** Constructor. */
    IntExpArray(
        IntExp* v0,
        IntExp* v1,
        IntExp* v2,
        IntExp* v3,
        IntExp* v4);

    /** Copy another instance. */
    virtual void copy(const utl::Object& rhs);

    /** Get the manager. */
    Manager* manager()
    { return _mgr; }

    /** Set the manager. */
    void setManager(Manager* mgr)
    { _mgr = mgr; }

    /** Get number of variables. */
    utl::uint_t size() const
    { return _exps.size(); }

    /** Set the \b managed flag. */
    void setManaged(bool managed);

    /** Get the smallest of the minimum values. */
    int minMin() const;

    /** Get the largest of the minimum values. */
    int maxMin() const;

    /** Get the smallest of the maximum values. */
    int minMax() const;

    /** Get the largest of the maximum values. */
    int maxMax() const;

    /** Add a variable. */
    void add(IntExp* var)
    { setManager(var->manager()); _exps.push_back(var); }

    /** Get the variable at the given index. */
    const IntExp* get(utl::uint_t idx) const
    { ASSERTD(idx < size()); return _exps[idx]; }

    /** Get the variable at the given index. */
    IntExp* get(utl::uint_t idx)
    { ASSERTD(idx < size()); return _exps[idx]; }

    /** Get the variable at the given index. */
    const IntExp& operator()(utl::uint_t idx) const
    { return *get(idx); }

    /** Get the variable at the given index. */
    IntExp& operator()(utl::uint_t idx)
    { return *get(idx); }

    /** Get the variable at the given index. */
    const IntExp* operator[](utl::uint_t idx) const
    { return get(idx); }

    /** Get the variable at the given index. */
    IntExp* operator[](utl::uint_t idx)
    { return get(idx); }

    /** Get begin iterator (const). */
    const_iterator begin() const
    { return _exps.begin(); }

    /** Get end iterator (const). */
    const_iterator end() const
    { return _exps.end(); }

    /** Get begin iterator. */
    iterator begin()
    { return _exps.begin(); }

    /** Get end iterator. */
    iterator end()
    { return _exps.end(); }
private:
    void init()
    { _mgr = nullptr; }
    void deInit() {}
    Manager* _mgr;
    intexp_vector_t _exps;
};

//////////////////////////////////////////////////////////////////////////////

CLP_NS_END;

//////////////////////////////////////////////////////////////////////////////

#endif
