#define BOOST_TEST_MODULE test_market
#include <boost/test/included/unit_test.hpp>

#include <DateCountCalculator/day_count_calculator.h>
#include <Market/Index.h>
#include <Market/ZeroCouponCurve.h>

#include <cmath>
#include <memory>
#include <sstream>
#include <vector>

namespace
{
boost::gregorian::date parse_ddmmyy(const std::string& text)
{
    int day = 0;
    int month = 0;
    int year = 0;
    char slash_1 = '\0';
    char slash_2 = '\0';

    std::istringstream in(text);
    in >> day >> slash_1 >> month >> slash_2 >> year;

    if (!in || slash_1 != '/' || slash_2 != '/')
    {
        throw std::runtime_error("Invalid date format, expected dd/mm/yy");
    }

    year += (year < 100) ? 2000 : 0;
    return boost::gregorian::date(year, month, day);
}
}

BOOST_AUTO_TEST_SUITE(market_suite)

BOOST_AUTO_TEST_CASE(curve_discount_and_forward_rates)
{
    using Date = boost::gregorian::date;

    const Date val = parse_ddmmyy("01/04/16");
    const std::vector<Date> pillars = {
        parse_ddmmyy("03/10/16"),
        parse_ddmmyy("03/04/17"),
        parse_ddmmyy("02/10/17"),
        parse_ddmmyy("02/04/18")
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
