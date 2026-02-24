#define BOOST_TEST_MODULE test_date_count_calculator
#include <boost/test/unit_test.hpp>
#include <DateCountCalculator/day_count_calculator.h>

BOOST_AUTO_TEST_SUITE(actual_360_suite)

    BOOST_AUTO_TEST_CASE(compute_daycount_from_string)
    {
        short days = Actual_360::compute_daycount("2020-01-01", "2020-01-31");
        BOOST_TEST(days == 30);
    }

    BOOST_AUTO_TEST_CASE(compute_daycount_from_date)
    {
        auto start = DayCountCalculator::make_date("2020-02-01");
        auto end = DayCountCalculator::make_date("2020-03-01");

        short days = Actual_360::compute_daycount(start, end);
        BOOST_TEST(days == 29);
    }

    BOOST_AUTO_TEST_CASE(operator_returns_fraction)
    {
        Actual_360 calc;
        double fraction = calc("2020-01-01", "2020-01-31");

        BOOST_TEST(fraction == (30.0 / 360.0), boost::test_tools::tolerance(1e-15));
    }

    BOOST_AUTO_TEST_CASE(compute_daycount_same_day_is_zero)
    {
        short days = Actual_360::compute_daycount("2020-01-31", "2020-01-31");
        BOOST_TEST(days == 0);
    }

    BOOST_AUTO_TEST_CASE(compute_daycount_reverse_dates_is_negative)
    {
        short days = Actual_360::compute_daycount("2020-01-31", "2020-01-01");
        BOOST_TEST(days == -30);
    }

    BOOST_AUTO_TEST_CASE(operator_accepts_date_objects)
    {
        Actual_360 calc;
        auto start = DayCountCalculator::make_date("2020-01-01");
        auto end = DayCountCalculator::make_date("2020-07-01");

        double fraction = calc(start, end);
        BOOST_TEST(fraction == (182.0 / 360.0), boost::test_tools::tolerance(1e-15));
    }

BOOST_AUTO_TEST_SUITE_END()