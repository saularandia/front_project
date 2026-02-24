#include <Market/ZeroCouponCurve.h>

#include <algorithm>
#include <cmath>
#include <stdexcept>

ZeroCouponCurve::ZeroCouponCurve(Date valuation_date,
                                 std::vector<Date> pillar_dates,
                                 std::vector<double> zc_rates_continuous)
    : valuation_date_(valuation_date),
      pillar_dates_(std::move(pillar_dates)),
      zc_rates_(std::move(zc_rates_continuous))
{
    if (pillar_dates_.empty())
        throw std::invalid_argument("ZeroCouponCurve: empty curve");
    if (pillar_dates_.size() != zc_rates_.size())
        throw std::invalid_argument("ZeroCouponCurve: dates/rates size mismatch");

    // Ensure strictly increasing pillar dates.
    for (size_t i = 1; i < pillar_dates_.size(); ++i)
    {
        if (!(pillar_dates_[i] > pillar_dates_[i - 1]))
            throw std::invalid_argument("ZeroCouponCurve: pillar dates must be strictly increasing");
    }
    if (pillar_dates_.front() <= valuation_date_)
        throw std::invalid_argument("ZeroCouponCurve: first pillar must be after valuation date");
}

double ZeroCouponCurve::get_dcf(const Date& d) const
{
    return dcc_(valuation_date_, d);
}

double ZeroCouponCurve::get_zc(const Date& d) const
{
    if (d <= valuation_date_)
        return 0.0;

    // If beyond last pillar, flat extrapolation.
    if (d >= pillar_dates_.back())
        return zc_rates_.back();

    // If before first pillar, flat extrapolation.
    if (d <= pillar_dates_.front())
        return zc_rates_.front();

    // Locate the bracket [i-1, i].
    auto it = std::upper_bound(pillar_dates_.begin(), pillar_dates_.end(), d);
    const size_t i = static_cast<size_t>(std::distance(pillar_dates_.begin(), it));

    const Date& d0 = pillar_dates_[i - 1];
    const Date& d1 = pillar_dates_[i];
    const double r0 = zc_rates_[i - 1];
    const double r1 = zc_rates_[i];

    const double t0 = get_dcf(d0);
    const double t1 = get_dcf(d1);
    const double t = get_dcf(d);

    // Linear interpolation on zero rates in time.
    const double w = (t - t0) / (t1 - t0);
    return r0 + w * (r1 - r0);
}

double ZeroCouponCurve::discount_factor(const Date& d) const
{
    if (d <= valuation_date_)
        return 1.0;

    const double zc = get_zc(d);
    const double t = get_dcf(d);
    return std::exp(-zc * t);
}
