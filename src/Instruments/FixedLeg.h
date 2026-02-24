#ifndef INSTRUMENTS_FIXEDLEG_H
#define INSTRUMENTS_FIXEDLEG_H

#include <DateCountCalculator/day_count_calculator.h>
#include <Instruments/Leg.h>
#include <Market/ZeroCouponCurve.h>

#include <memory>
#include <vector>

class FixedLeg : public Leg
{
public:
    using Date = boost::gregorian::date;

    struct Period { Date start; Date end; };

    FixedLeg(double notional,
             double fixed_rate,
             std::vector<Period> schedule,
             std::shared_ptr<const ZeroCouponCurve> curve,
             Actual_360 dcc = Actual_360{},
             bool include_notional_at_maturity = false);

    std::vector<CashFlow> cashflows() const override;
    double price() const override;

private:
    double notional_;
    double fixed_rate_;
    std::vector<Period> schedule_;
    std::shared_ptr<const ZeroCouponCurve> curve_;
    Actual_360 dcc_;
    bool include_notional_at_maturity_;
};

#endif
