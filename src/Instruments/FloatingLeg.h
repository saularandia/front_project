#ifndef INSTRUMENTS_FLOATINGLEG_H
#define INSTRUMENTS_FLOATINGLEG_H

#include <DateCountCalculator/day_count_calculator.h>
#include <Instruments/Leg.h>
#include <Market/Index.h>
#include <Market/ZeroCouponCurve.h>

#include <map>
#include <memory>
#include <vector>

class FloatingLeg : public Leg
{
public:
    using Date = boost::gregorian::date;

    struct Period { Date start; Date end; };

    FloatingLeg(double notional,
                std::shared_ptr<const Index> index,
                std::vector<Period> schedule,
                std::shared_ptr<const ZeroCouponCurve> curve,
                double spread = 0.0,
                Actual_360 dcc = Actual_360{});

    // Set a fixing for the period starting at `start` (nominal annual rate, like Euribor 6M).
    void set_fixing(const Date& start, double rate);

    std::vector<CashFlow> cashflows() const override;
    double price() const override;

private:
    double notional_;
    std::shared_ptr<const Index> index_;
    std::vector<Period> schedule_;
    std::shared_ptr<const ZeroCouponCurve> curve_;
    double spread_;
    Actual_360 dcc_;

    std::map<Date, double> fixings_;
};

#endif
