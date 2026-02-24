#include <Instruments/FloatingLeg.h>

#include <stdexcept>

FloatingLeg::FloatingLeg(double notional,
                         std::shared_ptr<const Index> index,
                         std::vector<Period> schedule,
                         std::shared_ptr<const ZeroCouponCurve> curve,
                         double spread,
                         Actual_360 dcc)
    : notional_(notional),
      index_(std::move(index)),
      schedule_(std::move(schedule)),
      curve_(std::move(curve)),
      spread_(spread),
      dcc_(dcc)
{
    if (!index_)
        throw std::invalid_argument("FloatingLeg: null index");
    if (!curve_)
        throw std::invalid_argument("FloatingLeg: null curve");
    if (schedule_.empty())
        throw std::invalid_argument("FloatingLeg: empty schedule");
}

void FloatingLeg::set_fixing(const Date& start, double rate)
{
    fixings_[start] = rate;
}

std::vector<CashFlow> FloatingLeg::cashflows() const
{
    std::vector<CashFlow> cfs;
    cfs.reserve(schedule_.size());

    for (const auto& p : schedule_)
    {
        const double tau = dcc_(p.start, p.end);
        double rate = 0.0;

        const auto it = fixings_.find(p.start);
        if (it != fixings_.end())
            rate = it->second;
        else
            rate = index_->get_rate(p.start, p.end);

        rate += spread_;

        const double interest = notional_ * rate * tau;
        cfs.push_back(CashFlow{p.end, interest});
    }

    return cfs;
}

double FloatingLeg::price() const
{
    const auto cfs = cashflows();
    double pv = 0.0;
    for (const auto& cf : cfs)
    {
        pv += cf.amount * curve_->discount_factor(cf.pay_date);
    }
    return pv;
}
