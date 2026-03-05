#ifndef MARKET_INDEX_H
#define MARKET_INDEX_H

#include <Market/ZeroCouponCurve.h>

#include <memory>

// Simple interest-rate index backed by a discount curve.
// For this topic we focus on Euribor 6M (m=2) style conversions.
class Index
{
public:
    using Date = boost::gregorian::date;

    explicit Index(std::shared_ptr<const ZeroCouponCurve> curve, int compounding_frequency = 2);

    // Continuously-compounded forward rate between start and end.
    double forward_rate_continuous(const Date& start, const Date& end) const;

    // Convert the forward continuous rate to an m-times compounded nominal annual rate.
    double forward_rate_nominal_m(const Date& start, const Date& end) const;

    // Return the forward rate between start and end, converted to nominal m-times compounded annual rate.
    double get_rate(const Date& start, const Date& end) const { return forward_rate_nominal_m(start, end); }

    static double continuous_from_nominal_m(double Rm, int m);
    static double nominal_m_from_continuous(double Rc, int m);

private:
    std::shared_ptr<const ZeroCouponCurve> curve_;
    int m_;
};

#endif
