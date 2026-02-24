#ifndef DAY_COUNT_CALCULATOR_H
#define DAY_COUNT_CALCULATOR_H

#include <boost/date_time/gregorian/gregorian.hpp>
#include <string>

class DayCountCalculator
{
public:
    static inline boost::gregorian::date make_date(const std::string& date)
    {
        return boost::gregorian::from_string(date);
    }
};

class Actual_360 : public DayCountCalculator
{
public:
    static inline short compute_daycount(const std::string& from, const std::string& to)
    {
        boost::gregorian::date from_date = make_date(from);
        boost::gregorian::date to_date = make_date(to);
        return compute_daycount(from_date, to_date);
    }

    static inline short compute_daycount(const boost::gregorian::date& from,
                                         const boost::gregorian::date& to)
    {
        return static_cast<short>((to - from).days());
    }

    template <class DATE>
    double operator()(const DATE& start, const DATE& end) const
    {
        return compute_daycount(start, end) / 360.0;
    }
};

#endif