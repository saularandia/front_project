#include <Instruments/Deposit.h>

#include <stdexcept>

Deposit::Deposit(Date start, Date end, double rate, Actual_360 dcc)
    : start_(start), end_(end), rate_(rate), dcc_(dcc)
{
    if (!(end_ > start_))
        throw std::invalid_argument("Deposit: end date must be after start date");
}

double Deposit::price() const
{
    // A deposit needs a discount curve to compute PV. This class is meant to be used
    // as a market quote for bootstrapping, so pricing is intentionally unsupported.
    throw std::logic_error("Deposit::price is not implemented (use computeDiscountFactor for bootstrapping)");
}

double Deposit::computeDiscountFactor(double& pre_annuity) const
{
    const double tau = dcc_(start_, end_);
    const double df = 1.0 / (1.0 + rate_ * tau);

    // The bootstrapping algorithm keeps an accumulated fixed-leg annuity:
    // A += tau_i * DF(0, t_i)
    pre_annuity += df * tau;
    return df;
}
