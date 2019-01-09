#ifndef CSE_UNARYCT_H
#define CSE_UNARYCT_H

//////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////

/** Unary constraint type. */
enum unary_ct_t
{
   uct_startAt = 0,           /**< start = t */
   uct_startNoSoonerThan,     /**< start >= t */
   uct_startNoLaterThan,      /**< start <= t */
   uct_endAt,                 /**< end = t */
   uct_endNoSoonerThan,       /**< end >= t */
   uct_endNoLaterThan,        /**< end <= t */
   uct_undefined              /**< undefined/null */
};

//////////////////////////////////////////////////////////////////////////////

/**
   Unary constraint.

   \author Adam McKee
*/

//////////////////////////////////////////////////////////////////////////////

class UnaryCt : public utl::Object
{
   UTL_CLASS_DECL(UnaryCt);
public:
   /**
      Constructor.
      \param type constraint type
      \param time associated time
   */
   UnaryCt(
      unary_ct_t type,
      time_t time)
   {
      _type = type;
      _time = time;
   }

   /** Copy another instance. */
   virtual void copy(const utl::Object& rhs);

   virtual void serialize(
       utl::Stream& stream,
       utl::uint_t io,
       utl::uint_t mode = utl::ser_default);

   /** Get type. */
   unary_ct_t type() const
      { return _type; }

   /** Get type. */
   unary_ct_t& type()
      { return _type; }

   /** Get time. */
   time_t time() const
      { return _time; }

   /** Get time. */
   time_t& time()
      { return _time; }
private:
   void init();
   void deInit() {}
private:
   unary_ct_t _type;
   time_t _time;
};

//////////////////////////////////////////////////////////////////////////////

typedef std::vector<UnaryCt*> unaryct_vect_t;

//////////////////////////////////////////////////////////////////////////////

CSE_NS_END;

//////////////////////////////////////////////////////////////////////////////

#endif
