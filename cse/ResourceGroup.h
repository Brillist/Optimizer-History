#ifndef CSE_RESOURCEGROUP_H
#define CSE_RESOURCEGROUP_H

//////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////

/**
   Resource group.

   \author Adam McKee
*/

//////////////////////////////////////////////////////////////////////////////

class ResourceGroup : public utl::Object
{
   UTL_CLASS_DECL(ResourceGroup);
public:
   /** Constructor. */
   ResourceGroup(utl::uint_t id)
      { init(); _id = id; }

   /** Copy another instance. */
   virtual void copy(const utl::Object& rhs);

   virtual void serialize(
       utl::Stream& stream,
       utl::uint_t io,
       utl::uint_t mode = utl::ser_default);

   /** Get a human-readable string representation. */
   virtual utl::String toString() const;

   /** Add a resource id. */
   void add(utl::uint_t resId)
      { _resIds.insert(resId); }

   /** Remove a resource id. */
   void remove(utl::uint_t resId)
      { _resIds.erase(resId); }

   /** Get the id. */
   utl::uint_t id() const
      { return _id; }

   /** Get the id. */
   utl::uint_t& id()
      { return _id; }

   /** Get the name. */
   const std::string& name() const
      { return _name; }

   /** Get the name. */
   std::string& name()
      { return _name; }

   /** Get the set of resource ids. */
   const lut::uint_set_t& resIds() const
      { return _resIds; }

   /** Get the set of resoruce ids. */
   lut::uint_set_t& resIds()
   { return _resIds; }
private:
   void init();
   void deInit() {}

   utl::uint_t _id;
   std::string _name;
   lut::uint_set_t _resIds;
};

//////////////////////////////////////////////////////////////////////////////

/**
   Order ResourceGroup objects by id.

   \author Adam McKee
*/

//////////////////////////////////////////////////////////////////////////////

struct ResourceGroupIdOrdering
   : public std::binary_function<ResourceGroup*,ResourceGroup*,bool>
{
   bool operator()(const ResourceGroup* lhs, const ResourceGroup* rhs) const
   {
      return (lhs->id() < rhs->id());
   }
};

//////////////////////////////////////////////////////////////////////////////

typedef std::set<ResourceGroup*, ResourceGroupIdOrdering> resgroup_set_t;

//////////////////////////////////////////////////////////////////////////////

CSE_NS_END;

//////////////////////////////////////////////////////////////////////////////

#endif
