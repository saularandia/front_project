#define BOOST_TEST_MODULE test_market
#include <boost/test/included/unit_test.hpp>

#include <DateCountCalculator/day_count_calculator.h>
#include <Market/Index.h>
#include <Market/ZeroCouponCurve.h>

#include <cmath>
#include <memory>
#include <vector>

BOOST_AUTO_TEST_SUITE(market_suite)

BOOST_AUTO_TEST_CASE(curve_discount_and_forward_rates)
{
    using Date = boost::gregorian::date;

    const Date val = boost::gregorian::from_string("2016/04/01");
    const std::vector<Date> pillars = {
        boost::gregorian::from_string("2016/10/03"),
        boost::gregorian::from_string("2017/04/03"),
        boost::gregorian::from_string("2017/10/02"),
        boost::gregorian::from_string("2018/04/02")
    };

    const double euribor6m_fix = 0.048; // 4.8% nominal annual, m=2
    const double zc1 = Index::continuous_from_nominal_m(euribor6m_fix, 2);

    const std::vector<double> zc = {zc1, 0.0500, 0.0510, 0.0520};

    auto curve = std::make_shared<ZeroCouponCurve>(val, pillars, zc);

    // Validate the starred conversion in the slides: 4.8% -> ~4.74% continuously compounded
    BOOST_TEST(zc1 == 0.047432, boost::test_tools::tolerance(1e-4));

    // Discount factor for first pillar.
    Actual_360 dcc;
    const double T1 = dcc(val, pillars[0]);
    const double expected_df1 = std::exp(-zc1 * T1);
    BOOST_TEST(curve->discount_factor(pillars[0]) == expected_df1, boost::test_tools::tolerance(1e-12));

    Index idx(curve, 2);

    // Forward rate for the second period, converted to nominal m=2.
    const double fwd_2 = idx.get_rate(pillars[0], pillars[1]);
    BOOST_TEST(fwd_2 == 0.0533073, boost::test_tools::tolerance(5e-4));
}

BOOST_AUTO_TEST_SUITE_END()
