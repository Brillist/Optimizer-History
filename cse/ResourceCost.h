#ifndef CSE_RESOURCECOST_H
#define CSE_RESOURCECOST_H

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
   Record costs associated with resource employment.

   Resource cost calculation has the following components:

   \arg <b>cost per unit hired</b> : cost of hiring one unit.
        This can be used to model training costs for example.

   \arg <b>cost per time-slot</b> : cost associated with using 1 unit of
        resource capacity for a duration of 1 time slot.

   \arg <b>minimum unemployment time-slots</b> : a unit of resource capacity
        cannot be dropped unless it will remain unused for at least this
        number of time-slots.  This models the fact that resources cannot be
        dropped and re-allocated on a continuous basis.

   \author Adam McKee
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class ResourceCost : public utl::Object
{
    UTL_CLASS_DECL(ResourceCost, utl::Object);

public:
    /** Clear self. */
    virtual void clear();

    /** Copy another instance. */
    virtual void copy(const utl::Object& rhs);

    virtual void
    serialize(utl::Stream& stream, uint_t io, uint_t mode = utl::ser_default);

    /** Get the cost. */
    double
    cost() const
    {
        return _cost;
    }

    /** Get the cost. */
    double&
    cost()
    {
        return _cost;
    }

    /** Get the resolution. */
    uint_t
    resolution() const
    {
        return _resolution;
    }

    /** Get the resolution. */
    uint_t&
    resolution()
    {
        return _resolution;
    }

    /** Get the cost per unit hired. */
    double
    costPerUnitHired() const
    {
        return _costPerUnitHired;
    }

    /** Get the cost per unit hired. */
    double&
    costPerUnitHired()
    {
        return _costPerUnitHired;
    }

    /** Get the cost per hour. */
    double
    costPerHour() const
    {
        return _costPerHour;
    }

    /** Get the cost per hour. */
    double&
    costPerHour()
    {
        return _costPerHour;
    }

    /** Get the cost per day. */
    double
    costPerDay() const
    {
        return _costPerDay;
    }

    /** Get the cost per day. */
    double&
    costPerDay()
    {
        return _costPerDay;
    }

    /** Get the cost per week. */
    double
    costPerWeek() const
    {
        return _costPerWeek;
    }

    /** Get the cost per week. */
    double&
    costPerWeek()
    {
        return _costPerWeek;
    }

    /** Get the cost per month. */
    double
    costPerMonth() const
    {
        return _costPerMonth;
    }

    /** Get the cost per month. */
    double&
    costPerMonth()
    {
        return _costPerMonth;
    }

    /** Get the minimum employment time. */
    uint_t
    minEmploymentTime() const
    {
        return _minEmploymentTime;
    }

    /** Get the minimum unemployment time. */
    uint_t&
    minEmploymentTime()
    {
        return _minEmploymentTime;
    }

    /** Get the maximum idle time. */
    uint_t
    maxIdleTime() const
    {
        return _maxIdleTime;
    }

    /** Get the maximum idle time. */
    uint_t&
    maxIdleTime()
    {
        return _maxIdleTime;
    }

private:
    void init();
    void
    deInit()
    {
    }

    double _cost;
    uint_t _resolution;
    double _costPerUnitHired;
    double _costPerHour;
    double _costPerDay;
    double _costPerWeek;
    double _costPerMonth;
    uint_t _minEmploymentTime;
    uint_t _maxIdleTime;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

CSE_NS_END;

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
