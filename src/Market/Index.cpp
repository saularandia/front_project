#include <Market/Index.h>

#include <cmath>
#include <stdexcept>

Index::Index(std::shared_ptr<const ZeroCouponCurve> curve, int compounding_frequency)
    : curve_(std::move(curve)), m_(compounding_frequency)
{
    if (!curve_)
        throw std::invalid_argument("Index: null curve");
    if (m_ <= 0)
        throw std::invalid_argument("Index: compounding frequency must be positive");
}

double Index::continuous_from_nominal_m(const double Rm, const int m)
{
    // Rc = m * ln(1 + Rm/m)
    return m * std::log(1.0 + Rm / m);
}

double Index::nominal_m_from_continuous(const double Rc, const int m)
{
    // Rm = m * (exp(Rc/m) - 1)
    return m * (std::exp(Rc / m) - 1.0);
}

double Index::forward_rate_continuous(const Date& start, const Date& end) const
{
    if (end <= start)
        throw std::invalid_argument("Index::forward_rate_continuous: end must be after start");

    const double t0 = curve_->get_dcf(start);
    const double t1 = curve_->get_dcf(end);
    if (t1 <= t0)
        throw std::invalid_argument("Index::forward_rate_continuous: invalid times");

    const double z0 = curve_->get_zc(start);
    const double z1 = curve_->get_zc(end);

    // Rc_fwd = (Z1*T1 - Z0*T0)/(T1 - T0)
    return (z1 * t1 - z0 * t0) / (t1 - t0);
}

double Index::forward_rate_nominal_m(const Date& start, const Date& end) const
{
    const double Rc = forward_rate_continuous(start, end);
    return nominal_m_from_continuous(Rc, m_);
}
