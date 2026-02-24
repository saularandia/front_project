#ifndef MARKET_ZEROCOUPONCURVE_H
#define MARKET_ZEROCOUPONCURVE_H

#include <DateCountCalculator/day_count_calculator.h>

#include <boost/date_time/gregorian/gregorian.hpp>
#include <vector>

// Continuously-compounded zero-coupon curve.
// Rates are expressed as annualized decimals, e.g. 0.052 for 5.2%.
class ZeroCouponCurve
{
public:
    using Date = boost::gregorian::date;

    ZeroCouponCurve(Date valuation_date,
                    std::vector<Date> pillar_dates,
                    std::vector<double> zc_rates_continuous);

    const Date& valuation_date() const { return valuation_date_; }

    // Year fraction from valuation_date to d using Act/360.
    double get_dcf(const Date& d) const;

    // Interpolated continuously-compounded zero rate at date d.
    double get_zc(const Date& d) const;

    // Discount factor DF(0,d) = exp(-zc(d) * T(0,d)).
    double discount_factor(const Date& d) const;

private:
    Date valuation_date_;
    std::vector<Date> pillar_dates_;
    std::vector<double> zc_rates_;
    Actual_360 dcc_;
};

#endif
