#include <Instruments/Bond.h>

#include <cmath>
#include <stdexcept>

namespace
{
std::vector<FixedLeg::Period> to_fixed_schedule(const std::vector<Bond::Period>& schedule)
{
    std::vector<FixedLeg::Period> fixed_schedule;
    fixed_schedule.reserve(schedule.size());
    for (const auto& period : schedule)
    {
        fixed_schedule.push_back(FixedLeg::Period{period.start, period.end});
    }
    return fixed_schedule;
}
}

Bond::Bond(double notional,
           double coupon_rate,
           std::vector<Period> schedule,
           std::shared_ptr<const ZeroCouponCurve> curve,
           Actual_360 dcc)
    : curve_(std::move(curve)),
      fixed_leg_(notional,
                 coupon_rate,
                 to_fixed_schedule(schedule),
                 curve_,
                 dcc,
                 true)
{
    if (!curve_)
        throw std::invalid_argument("Bond: null curve");
}

double Bond::price() const
{
    return fixed_leg_.price();
}

double Bond::price_from_yield(double y_continuous) const
{
    double pv = 0.0;
    const auto cfs = fixed_leg_.cashflows();
    for (const auto& cf : cfs)
    {
        const double t = curve_->get_dcf(cf.pay_date);
        pv += cf.amount * std::exp(-y_continuous * t);
    }
    return pv;
}

double Bond::irr_from_price(double market_price, double initial_guess) const
{
    auto f = [&](double y) { return price_from_yield(y) - market_price; };

    // Numerical derivative.
    auto df = [&](double y) {
        const double h = 1e-6;
        return (f(y + h) - f(y - h)) / (2.0 * h);
    };

    const double tol = 1e-12;
    const int max_iter = 50;

    double x = initial_guess;
    for (int iteration = 0; iteration < max_iter; ++iteration)
    {
        const double fx = f(x);
        if (std::abs(fx) < tol)
            return x;

        const double dfx = df(x);
        if (dfx == 0.0)
            throw std::runtime_error("Bond::irr_from_price: zero derivative in Newton-Raphson");

        x -= fx / dfx;
    }

    throw std::runtime_error("Bond::irr_from_price: Newton-Raphson did not converge");
}
