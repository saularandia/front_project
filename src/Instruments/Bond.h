#ifndef INSTRUMENTS_BOND_H
#define INSTRUMENTS_BOND_H

#include <DateCountCalculator/day_count_calculator.h>
#include <Instruments/FixedLeg.h>
#include <Instruments/IPricer.h>
#include <Market/ZeroCouponCurve.h>

#include <memory>
#include <vector>

class Bond : public IPricer
{
public:
    using Date = boost::gregorian::date;

    struct Period { Date start; Date end; };

    Bond(double notional,
         double coupon_rate,
         std::vector<Period> schedule,
         std::shared_ptr<const ZeroCouponCurve> curve,
         Actual_360 dcc = Actual_360{});

    double price() const override;

    // Price a fixed coupon bond using a flat continuously-compounded yield y.
    double price_from_yield(double y_continuous) const;

    // Compute IRR (continuously compounded) that matches a target market price.
    double irr_from_price(double market_price, double initial_guess = 0.05) const;

private:
    std::shared_ptr<const ZeroCouponCurve> curve_;
    FixedLeg fixed_leg_;
};

#endif
