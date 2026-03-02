#ifndef INSTRUMENTS_DEPOSIT_H
#define INSTRUMENTS_DEPOSIT_H

#include <DateCountCalculator/day_count_calculator.h>
#include <Instruments/IPricer.h>

#include <boost/date_time/gregorian/gregorian.hpp>

// Simple money-market deposit quote used for discount-factor bootstrapping.
// Market quote is a simple interest rate R over [start,end] with year fraction b(start,end).
// Discount factor:
//   DF(0,end) = 1 / (1 + R * b(start,end)).
class Deposit : public IPricer
{
public:
    using Date = boost::gregorian::date;

    Deposit(Date start, Date end, double rate, Actual_360 dcc = Actual_360{});

    // Not used by the bootstrap exercise; provided to satisfy IPricer.
    double price() const override;

    double computeDiscountFactor(double& pre_annuity) const override;

private:
    Date start_;
    Date end_;
    double rate_;
    Actual_360 dcc_;
};

#endif
