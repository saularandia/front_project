#ifndef INSTRUMENTS_INSTRUMENTDESCRIPTION_H
#define INSTRUMENTS_INSTRUMENTDESCRIPTION_H

#include <DateCountCalculator/day_count_calculator.h>

#include <boost/date_time/gregorian/gregorian.hpp>
#include <map>
#include <string>
#include <vector>

struct MarketDescription
{
    using Date = boost::gregorian::date;

    Date valuation_date;
    std::vector<Date> pillar_dates;
    std::vector<double> zc_rates_continuous;

    void validate() const;
};

struct LegDescription
{
    using Date = boost::gregorian::date;

    enum Type { none, fixed, floating };

    Type type = none;
    double notional = 0.0;

    // Fixed leg
    double fixed_rate = 0.0;
    bool include_notional_at_maturity = false;

    // Floating leg
    double spread = 0.0;
    int compounding_frequency = 2; // Euribor 6M => 2

    // Schedule (assume pay at period end)
    std::vector<Date> start_dates;
    std::vector<Date> end_dates;

    // Optional fixings keyed by period start date (nominal annual rate)
    std::map<Date, double> fixings;

    void validate() const;
};

struct InstrumentDescription
{
    enum Type { bond, swap };

    Type type;
    explicit InstrumentDescription(Type t) : type(t) {}

    LegDescription payer;
    LegDescription receiver;
    MarketDescription market;

    void validate() const;
};

#endif
