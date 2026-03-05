#ifndef INSTRUMENTS_SWAP_H
#define INSTRUMENTS_SWAP_H

#include <DateCountCalculator/day_count_calculator.h>
#include <Instruments/IPricer.h>
#include <Instruments/Leg.h>

#include <boost/date_time/gregorian/gregorian.hpp>
#include <memory>

class Swap : public IPricer
{
public:
    using Date = boost::gregorian::date;

    Swap(std::unique_ptr<Leg> receiver, std::unique_ptr<Leg> payer);
    // Bootstrap constructor: [start,maturity] represents the accrual interval
    // of the last fixed period used to solve DF(0,maturity).
    Swap(Date start, Date maturity, double fixed_rate, int payments_per_year = 2, Actual_360 dcc = Actual_360{});

    double price() const override;
    double computeDiscountFactor(double& pre_annuity) const override;

    const Leg& receiver_leg() const { return *receiver_; }
    const Leg& payer_leg() const { return *payer_; }

private:
    enum class Mode { pricing, bootstrap };

    Mode mode_;
    std::unique_ptr<Leg> receiver_;
    std::unique_ptr<Leg> payer_;

    Date start_;
    Date maturity_;
    double fixed_rate_ = 0.0;
    int payments_per_year_ = 0;
    Actual_360 dcc_;
};

#endif
