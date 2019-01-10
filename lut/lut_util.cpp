#include "liblut.h"
#include <libutl/Time.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

#undef new
#include <iomanip>
#include <libutl/gblnew_macros.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;

////////////////////////////////////////////////////////////////////////////////////////////////////

LUT_NS_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////////////

void
serialize(std::string& str, Stream& stream, uint_t io)
{
    if (io == utl::io_rd)
    {
        utl::String utlString;
        utlString.serialize(stream, io_rd, ser_default);
        str = utlString.get();
    }
    else
    {
        utl::String utlString(str.c_str(), false);
        utlString.serialize(stream, io_wr, ser_default);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
serialize(time_t& tt, Stream& stream, uint_t io)
{
    if (io == io_rd)
    {
        int i;
        utl::serialize(i, stream, io, ser_default);
        tt = i;
    }
    else
    {
        int i = tt;
        utl::serialize(i, stream, io, ser_default);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::string
heading(const std::string& title, char ch, uint_t width)
{
    std::ostringstream ss;
    uint_t i;
    for (i = 0; i < 3; ++i)
    {
        ss << ch;
    }
    ss << " ";
    ss << title;
    ss << " ";
    uint_t rem = width - title.size() - 5;
    for (i = 0; i < rem; ++i)
    {
        ss << ch;
    }
    return ss.str();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::string
getTimeString(time_t t)
{
    struct tm* tm = localtime(&t);
    char buf[256];
    sprintf(buf, "%02u-%02u-%02u %02u:%02u:%02u", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
            tm->tm_hour, tm->tm_min, tm->tm_sec);
    return std::string(buf);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

period_t
stringToPeriod(const std::string& p_str)
{
    std::string str = tolower(p_str);
    if ((str == "hour") || (str == "hourly"))
    {
        return period_hour;
    }
    else if ((str == "day") || (str == "daily"))
    {
        return period_day;
    }
    else if ((str == "week") || (str == "weekly"))
    {
        return period_week;
    }
    else if ((str == "month") || (str == "monthly"))
    {
        return period_month;
    }
    else if ((str == "year") || (str == "yearly") || (str == "annually"))
    {
        return period_year;
    }
    return period_undefined;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::string
periodToString(period_t period)
{
    switch (period)
    {
    case period_hour:
        return "hour";
    case period_day:
        return "day";
    case period_week:
        return "week";
    case period_month:
        return "month";
    case period_year:
        return "year";
    default:
        ABORT();
    }
    return "(null)";
}

////////////////////////////////////////////////////////////////////////////////////////////////////

uint_t
periodToSeconds(period_t period)
{
    switch (period)
    {
    case period_hour:
        return 60 * 60;
    case period_day:
        return 24 * 60 * 60;
    case period_week:
        return 7 * 24 * 60 * 60;
    case period_month:
        return 30 * 24 * 60 * 60;
    case period_year:
        return 365 * 24 * 60 * 60;
    default:
        ABORT();
    }
    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void*
realloc(void* ptr, uint_t oldSize, uint_t newSize)
{
    ASSERT(ptr != nullptr);
    void* newPtr = malloc(newSize);
    ASSERT(newPtr != nullptr);
    memcpy(newPtr, ptr, oldSize);
    free(ptr);
    return newPtr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

time_t
time_date(time_t t)
{
    return (t - time_timeOfDay(t));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

uint_t
time_timeOfDay(time_t t)
{
    struct tm* tms = localtime(&t);
    return (tms->tm_hour * 3600) + (tms->tm_min * 60) + tms->tm_sec;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

uint_t
time_dayOfWeek(time_t t)
{
    struct tm* tms = localtime(&t);
    return tms->tm_wday;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::string
time_str(time_t t)
{
    std::ostringstream ss;
    struct tm* tms = localtime(&t);
    ss << (1900 + tms->tm_year) << "/" << std::setfill('0') << std::setw(2) << (tms->tm_mon + 1)
       << "/" << std::setfill('0') << std::setw(2) << tms->tm_mday << " " << std::setfill('0')
       << std::setw(2) << tms->tm_hour << ":" << std::setfill('0') << std::setw(2) << tms->tm_min
       << ":" << std::setfill('0') << std::setw(2) << tms->tm_sec;
    return ss.str();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool
getFlag(utl::uint32_t flags, utl::uint32_t bit)
{
    utl::uint32_t mask = 1U << bit;
    return (flags & mask);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void
setFlag(utl::uint32_t& flags, utl::uint32_t bit, bool val)
{
    utl::uint32_t mask = 1U << bit;
    if (val)
    {
        flags |= mask;
    }
    else
    {
        flags &= ~mask;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

template <>
int
compare(const double& lhs, const double& rhs)
{
    if (fabs(lhs - rhs) < 0.0000000001)
    {
        return 0;
    }
    else if (lhs < rhs)
    {
        return -1;
    }
    else
    {
        return 1;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::string
readLine(std::istream& is)
{
    char* line = new char[1024];
    is.getline(line, 1024);
    std::string str(line);
    delete[] line;
    return str;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::string
tolower(const std::string& str)
{
    std::string res = str;
    uint_t len = str.length();
    for (uint_t i = 0; i < len; i++)
    {
        res[i] = ::tolower(res[i]);
    }
    return res;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

LUT_NS_END;
