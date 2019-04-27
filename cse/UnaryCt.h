#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Unary constraint type.
   \ingroup cse
*/
enum unary_ct_t
{
    uct_startAt = 0,       /**< start time = t */
    uct_startNoSoonerThan, /**< start time >= t */
    uct_startNoLaterThan,  /**< start time <= t */
    uct_endAt,             /**< end time = t */
    uct_endNoSoonerThan,   /**< end time >= t */
    uct_endNoLaterThan,    /**< end time <= t */
    uct_undefined          /**< undefined/null */
};

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Unary constraint.

   A UnaryCt requires a JobOp's start or end time to relate to a specified time in a specified way.

   \see unary_ct_t
   \ingroup cse
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class UnaryCt : public utl::Object
{
    UTL_CLASS_DECL(UnaryCt, utl::Object);

public:
    /**
      Constructor.
      \param type constraint type
      \param time specified time
   */
    UnaryCt(unary_ct_t type, time_t time)
    {
        _type = type;
        _time = time;
    }

    virtual void copy(const utl::Object& rhs);

    virtual void serialize(utl::Stream& stream, uint_t io, uint_t mode = utl::ser_default);

    /// \name Accessors (const)
    //@{
    /** Get type. */
    unary_ct_t
    type() const
    {
        return _type;
    }

    /** Get time. */
    time_t
    time() const
    {
        return _time;
    }
    //@}

    /// \name Accessors (non-const)
    //@{
    /** Set type. */
    void
    setType(unary_ct_t type)
    {
        _type = type;
    }

    /** Set time. */
    void
    setTime(time_t time)
    {
        _time = time;
    }
    //@}

private:
    void init();
    void
    deInit()
    {
    }

private:
    unary_ct_t _type;
    time_t _time;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

using unaryct_vector_t = std::vector<UnaryCt*>;

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;
