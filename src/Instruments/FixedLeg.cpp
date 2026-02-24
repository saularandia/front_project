#include <Instruments/FixedLeg.h>

#include <cmath>
#include <stdexcept>

FixedLeg::FixedLeg(double notional,
                   double fixed_rate,
                   std::vector<Period> schedule,
                   std::shared_ptr<const ZeroCouponCurve> curve,
                   Actual_360 dcc,
                   bool include_notional_at_maturity)
    : notional_(notional),
      fixed_rate_(fixed_rate),
      schedule_(std::move(schedule)),
      curve_(std::move(curve)),
      dcc_(dcc),
      include_notional_at_maturity_(include_notional_at_maturity)
{
    if (!curve_)
        throw std::invalid_argument("FixedLeg: null curve");
    if (schedule_.empty())
        throw std::invalid_argument("FixedLeg: empty schedule");
}

std::vector<CashFlow> FixedLeg::cashflows() const
{
    std::vector<CashFlow> cfs;
    cfs.reserve(schedule_.size() + (include_notional_at_maturity_ ? 1u : 0u));

    for (size_t i = 0; i < schedule_.size(); ++i)
    {
        const auto& p = schedule_[i];
        const double tau = dcc_(p.start, p.end);
        const double coupon = notional_ * fixed_rate_ * tau;
        cfs.push_back(CashFlow{p.end, coupon});
    }

    if (include_notional_at_maturity_)
    {
        const auto& last = schedule_.back();
        cfs.push_back(CashFlow{last.end, notional_});
    }

    return cfs;
}

double FixedLeg::price() const
{
    const auto cfs = cashflows();
    double pv = 0.0;
    for (const auto& cf : cfs)
    {
        pv += cf.amount * curve_->discount_factor(cf.pay_date);
    }
    return pv;
}
