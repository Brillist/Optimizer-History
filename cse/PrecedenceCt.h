#ifndef CSE_PRECEDENCECT_H
#define CSE_PRECEDENCECT_H

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/** Precedence constraint type. */
enum precedence_ct_t
{
    pct_fs = 0,   /**< (lhs.end + delay) <= rhs.start    (F-S) */
    pct_ss = 1,   /**< (lhs.start + delay) <= rhs.start  (S-S) */
    pct_ff = 2,   /**< (lhs.end + delay) <= rhs.end      (F-F) */
    pct_sf = 3,   /**< (lhs.start + delay) <= rhs.end    (S-F) */
    pct_undefined /**< undefined/null */
};

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Precedence constraint.

   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class PrecedenceCt : public utl::Object
{
    UTL_CLASS_DECL(PrecedenceCt, utl::Object);

public:
    /**
      Constructor.
      \param lhsOp left-hand-side operation
      \param rhsOp right-hand-side operation
      \param type type of precedence constraint
      \param delay delay in seconds
   */
    PrecedenceCt(uint_t lhsOpId, uint_t rhsOpId, precedence_ct_t type, uint_t delay)
    {
        _lhsOpId = lhsOpId;
        _rhsOpId = rhsOpId;
        _type = type;
        _delay = delay;
    }

    /** Copy another instance. */
    virtual void copy(const utl::Object& rhs);

    virtual void
    serialize(utl::Stream& stream, uint_t io, uint_t mode = utl::ser_default);

    /** Get lhs op id. */
    uint_t
    lhsOpId() const
    {
        return _lhsOpId;
    }

    /** Get lhs op id. */
    uint_t&
    lhsOpId()
    {
        return _lhsOpId;
    }

    /** Get rhs op id. */
    uint_t
    rhsOpId() const
    {
        return _rhsOpId;
    }

    /** Get rhs op id. */
    uint_t&
    rhsOpId()
    {
        return _rhsOpId;
    }

    /** Get constraint type. */
    precedence_ct_t
    type() const
    {
        return _type;
    }

    /** Get constraint type. */
    precedence_ct_t&
    type()
    {
        return _type;
    }

    /** Get delay. */
    int
    delay() const
    {
        return _delay;
    }

    /** Get delay. */
    int&
    delay()
    {
        return _delay;
    }

private:
    void init();
    void
    deInit()
    {
    }

private:
    uint_t _lhsOpId;
    uint_t _rhsOpId;
    precedence_ct_t _type;
    int _delay;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

typedef std::vector<PrecedenceCt*> pct_vector_t;

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
