#include <Instruments/Swap.h>

#include <stdexcept>

Swap::Swap(std::unique_ptr<Leg> receiver, std::unique_ptr<Leg> payer)
    : mode_(Mode::pricing), receiver_(std::move(receiver)), payer_(std::move(payer))
{
    if (!receiver_ || !payer_)
        throw std::invalid_argument("Swap: null leg");
}

Swap::Swap(Date start, Date maturity, double fixed_rate, int payments_per_year, Actual_360 dcc)
    : mode_(Mode::bootstrap),
      start_(start),
      maturity_(maturity),
      fixed_rate_(fixed_rate),
      payments_per_year_(payments_per_year),
      dcc_(dcc)
{
    if (!(maturity_ > start_))
        throw std::invalid_argument("Swap: maturity must be after start date");
    if (payments_per_year_ <= 0)
        throw std::invalid_argument("Swap: payments_per_year must be positive");
    if (12 % payments_per_year_ != 0)
        throw std::invalid_argument("Swap: payments_per_year must divide 12 (whole-month periods)");
}

double Swap::price() const
{
    if (mode_ != Mode::pricing)
        throw std::logic_error("Swap::price is not available for bootstrap quote swaps");

    return receiver_->price() - payer_->price();
}

double Swap::computeDiscountFactor(double& pre_annuity) const
{
    if (mode_ != Mode::bootstrap)
        throw std::logic_error("Swap::computeDiscountFactor is only available for bootstrap quote swaps");

    const double tau_last = dcc_(start_, maturity_);
    const double df = (1.0 - fixed_rate_ * pre_annuity) / (1.0 + fixed_rate_ * tau_last);

    if (!(df > 0.0))
        throw std::runtime_error("Swap: bootstrapped discount factor is non-positive (check inputs/order)");

    pre_annuity += df * tau_last;
    return df;
}
