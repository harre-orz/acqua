#pragma once

#include <ctime>
#include <locale>
#include <iomanip>
#include "boost/date_time/time_zone_base.hpp"
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/local_time/local_time.hpp>

namespace acqua { namespace log { namespace detail {

template <typename Derived, typename DateTime>
class formatter;

template <typename Derived>
class formatter<Derived, boost::posix_time::ptime>
{
    using os = std::ostream;
    using wos = std::wostream;
    using ptime = boost::posix_time::ptime;
    using date = boost::gregorian::date;
    using time = boost::posix_time::time_duration;
    using zone = boost::local_time::time_zone_ptr;

public:
    static ptime now()
    {
        return boost::posix_time::microsec_clock::local_time();
    }

    static zone tz()
    {
        // TODO:
        return zone(new boost::local_time::posix_time_zone("JST-9"));
    }

    template <typename Out, typename Fmt>
    void format(Out & os, Fmt const & fmt)
    {
        ptime now = static_cast<Derived *>(this)->now();
        date d = now.date();
        time t = now.time_of_day();
        zone z;

        std::size_t end = 0, beg;
        do {
            beg = end;
            if ((end = fmt.find('%', beg) + 1) == 0)
                end = fmt.size() + 1;
            os.write(fmt.c_str() + beg, end - beg - 1);

            if (end < fmt.size() && fmt[end] == '-') {
                os << std::left;
                ++end;
            }

          CONTINUE:
            if (end >= fmt.size())
                break;
            switch(fmt[end++]) {
                case '0':
                    os << std::setfill('0');
                case '1' ... '9': {
                    int w = fmt[end-1] - '0';
                    while(std::isdigit(fmt[end]))
                        w = w * 10 + fmt[end++] - '0';
                    os << std::setw(w);
                    goto CONTINUE;
                }
                case '%':
                    os << '%';
                    break;
                case 'A':
                    static_cast<Derived *>(this)->replace_weekday_name(os, d);
                    break;
                case 'a':
                    static_cast<Derived *>(this)->replace_abbreviated_weekday_name(os, d);
                    break;
                case 'B':
                    static_cast<Derived *>(this)->replace_month_name(os, d);
                    break;
                case 'b':
                    static_cast<Derived *>(this)->replace_abbreviated_month_name(os, d);
                    break;
                case 'C':
                    static_cast<Derived *>(this)->replace_century(os, d);
                    break;
                case 'c':
                    static_cast<Derived *>(this)->replace_date_time(os, now);
                    break;
              //case 'D':
                case 'd':
                    static_cast<Derived *>(this)->replace_day(os, d);
                    break;
              //case 'E':
              //case 'e':
                case 'F':
                    static_cast<Derived *>(this)->replace_simple_date_string(os, d);
                    break;
                case 'f':  // extends
                    static_cast<Derived *>(this)->replace_logger_function(os);
                    break;
                case 'H':
                    static_cast<Derived *>(this)->replace_24_hour_clock(os, t);
                    break;
              //case 'h':
                case 'I':
                    static_cast<Derived *>(this)->replace_12_hour_clock(os, t);
                    break;
                case 'i':   // extends
                    static_cast<Derived *>(this)->replace_logger_info(os);
                    break;
              //case 'J':
                case 'j':
                    static_cast<Derived *>(this)->replace_day_of_year(os, d);
                    break;
             // case 'K':
             // case 'k':
                case 'L':  // extends
                    static_cast<Derived *>(this)->replace_level_name(os);
                    break;
                case 'l':  // extends
                    static_cast<Derived *>(this)->replace_logger_location(os);
                    break;
                case 'M':
                    static_cast<Derived *>(this)->replace_minute(os, t);
                    break;
                case 'm':
                    static_cast<Derived *>(this)->replace_month(os, d);
                    break;
             // case 'N':
             // case 'n':
             // case 'O':
             // case 'o':
                case 'P':
                    //static_cast<Derived *>(this)->replace_am_pm_string(os, now);
                    break;
                case 'p':
                    //static_cast<Derived *>(this)->replace_lower_am_pm_string(os, now);
                    break;
             // case 'Q':
             // case 'q':
             // case 'R':
             // case 'r':
                case 'S':
                    static_cast<Derived *>(this)->replace_second(os, t);
                    break;
                case 's':
                    //static_cast<Derived *>(this)->replace_unix_epoch(os, now);
                    break;
                case 'T':
                    static_cast<Derived *>(this)->replace_simple_time_string(os, t);
                    break;
                case 't':  // extends
                    static_cast<Derived *>(this)->replace_thread_id(os);
                    break;
                case 'U':  // extends
                    static_cast<Derived *>(this)->replace_millisecond(os, t);
                    break;
                case 'u':  // extends
                    static_cast<Derived *>(this)->replace_microsecond(os, t);
                    break;
              //case 'V':
                case 'v':  // extends
                    static_cast<Derived *>(this)->replace_logger_message(os);
                    break;
                case 'W':
                    static_cast<Derived *>(this)->replace_week_of_year(os, d);
                    break;
                case 'w':
                    static_cast<Derived *>(this)->replace_day_of_week(os, d);
                    break;
              //case 'X':
              //case 'x':
                case 'Y':
                    static_cast<Derived *>(this)->replace_year(os, d);
                    break;
                case 'y':
                    static_cast<Derived *>(this)->replace_year_without_century(os, d);
                    break;
                case 'Z':
                    if (!z) z = static_cast<Derived *>(this)->tz();
                    static_cast<Derived *>(this)->replace_time_zone_name(os, z);
                    break;
                case 'z':
                    if (!z) z = static_cast<Derived *>(this)->tz();
                    static_cast<Derived *>(this)->replace_base_utc_offset(os, z);
                    break;
                default:
                    os << '%' << fmt[end];
                    break;
            }
        } while(true);
    }

private:
    static void replace_weekday_name(os & os, date const & now) { os << now.day_of_week().as_long_string(); }
    static void replace_weekday_name(wos & os, date const & now) { os << now.day_of_week().as_long_wstring(); }
    static void replace_abbreviated_weekday_name(os & os, date const & now) { os << now.day_of_week().as_short_string(); }
    static void replace_abbreviated_weekday_name(wos & os, date const & now) { os << now.day_of_week().as_short_wstring(); }
    static void replace_month_name(os & os, date const & now) { os << now.month().as_long_string(); }
    static void replace_month_name(wos & os, date const & now) { os << now.month().as_long_string(); }
    static void replace_abbreviated_month_name(os & os, date const & now) { os << now.month().as_long_string(); }
    static void replace_abbreviated_month_name(wos & os, date const & now) { os << now.month().as_long_wstring(); }
    template <typename Out> static void replace_date_time(Out & os, ptime const & now) { os << now; }
    template <typename Out> static void replace_day(Out & os, date const & now) { os << now.day(); }
    template <typename Out> static void replace_simple_date_string(Out & os, date const & now) { os << boost::gregorian::to_iso_extended_string(now); }
    template <typename Out> static void replace_24_hour_clock(Out & os, time const & now) { os << now.hours(); }
    template <typename Out> static void replace_12_hour_clock(Out & os, time const & now) { os << (now.hours() % 12); }
    template <typename Out> static void replace_day_of_year(Out & os, date const & now) { os << now.day_of_year(); }
    template <typename Out> static void replace_minute(Out & os, time const & now) { os << now.minutes(); }
    template <typename Out> static void replace_month(Out & os, date const & now) { os << now.month(); }
    template <typename Out> static void replace_second(Out & os, time const & now) { os << now.seconds(); }
    template <typename Out> static void replace_simple_time_string(Out & os, time const & now)
    {
        long t = now.total_seconds();
        typename Out::char_type str[9];
        str[8] = 0;
        str[7] = '0' + t % 10;
        t /= 10;
        str[6] = '0' + t % 6;
        t /= 6;
        str[5] = ':';
        str[4] = '0' + t % 10;
        t /= 10;
        str[3] = '0' + t % 6;
        t /= 6;
        str[2] = ':';
        str[1] = '0' + t % 10;
        t /= 10;
        str[0] = '0' + t;
        os << str;
    }
    template <typename Out> static void replace_week_of_year(Out & os, date const & now) { os << now.week_number(); }
    template <typename Out> static void replace_day_of_week(Out & os, date const & now) { os << now.day_of_week(); }
    template <typename Out> static void replace_year(Out & os, date const & now) { os << now.year(); }
    template <typename Out> static void replace_year_without_century(Out & os, date const & now) { os << now.year() % 100; }
    template <typename Out> static void replace_time_zone_name(Out & os, zone const & now) { os << now->to_posix_string(); }
    template <typename Out> static void replace_base_utc_offset(Out & os, zone const & now)
    {
        typename Out::char_type tz[6];
        auto off = now->base_utc_offset().total_seconds();
        tz[0] = (off < 0 ? '-' : '+');
        off = std::abs(off / 60);
        tz[4] = '0' + off % 10;
        off /= 10;
        tz[3] = '0' + off % 6;
        off /= 6;
        tz[2] = '0' + off % 10;
        off /= 10;
        tz[1] = '0' + off;
        tz[5] = 0;
        os << tz;
    }
    template <typename Out> static void replace_millisecond(Out & os, time const & now) { os << std::setfill('0') << std::setw(3) << now.total_milliseconds() % 1000; }
    template <typename Out> static void replace_microsecond(Out & os, time const & now) { os << std::setfill('0') << std::setw(6) << now.total_microseconds() % 1000000; }
    template <typename Out> static void replace_century(Out & os, date const & now) { os << (now.year() / 100 + 1); }

    template <typename Out> void replace_level_name(Out & os) const { os << static_cast<Derived const *>(this)->level; }
    template <typename Out> void replace_abbreviated_level_name(Out & os) const { os << severity_symbol(static_cast<Derived const *>(this)->level); }
    template <typename Out> void replace_logger_message(Out & os) { os << &static_cast<Derived *>(this)->buffer; }
    template <typename Out> void replace_thread_id(Out & os) { os << static_cast<Derived *>(this)->tid; }
    template <typename Out> void replace_logger_info(Out &) {}
    template <typename Out> void replace_logger_location(Out & os) const
    {
        auto const * this_ = static_cast<Derived const *>(this);
        std::copy_n(this_->file, std::strlen(this_->file), std::ostreambuf_iterator<typename Out::char_type>(os));
        os << ':' << this_->line;
    }
    template <typename Out> void replace_logger_function(Out & os) const
    {
        auto const * this_ = static_cast<Derived const *>(this);
        std::copy_n(this_->func, std::strlen(this_->func), std::ostreambuf_iterator<typename Out::char_type>(os));
    }
};

} } }
