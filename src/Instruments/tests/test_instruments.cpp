#define BOOST_TEST_MODULE test_instruments
#include <boost/test/included/unit_test.hpp>
#include <boost/math/tools/roots.hpp>

#include <DateCountCalculator/day_count_calculator.h>
#include <Instruments/Bond.h>
#include <Instruments/Factory.h>
#include <Instruments/GoalSeeker.h>
#include <Instruments/InstrumentDescription.h>
#include <Instruments/Swap.h>
#include <Market/Index.h>
#include <Market/ZeroCouponCurve.h>

#include <cmath>
#include <limits>
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

BOOST_AUTO_TEST_SUITE(instruments_suite)

BOOST_AUTO_TEST_CASE(swap_example_from_slides)
{
    using Date = boost::gregorian::date;

    const Date val = parse_ddmmyy("01/04/16");
    const std::vector<Date> pay_dates = {
        parse_ddmmyy("03/10/16"),
        parse_ddmmyy("03/04/17"),
        parse_ddmmyy("02/10/17"),
        parse_ddmmyy("02/04/18")
    };

    // Build periods as [start,end] with payment at end.
    const std::vector<Date> start_dates = {
        val,
        pay_dates[0],
        pay_dates[1],
        pay_dates[2]
    };

    const std::vector<Date> end_dates = pay_dates;

    // Market: zero rates continuously compounded.
    const double euribor6m_fix = 0.048; // first fixing
    const double zc1 = Index::continuous_from_nominal_m(euribor6m_fix, 2);

    MarketDescription market;
    market.valuation_date = val;
    market.pillar_dates = pay_dates;
    market.zc_rates_continuous = {zc1, 0.0500, 0.0510, 0.0520};

    InstrumentDescription desc(InstrumentDescription::swap);
    desc.market = market;

    // Receiver: floating
    desc.receiver.type = LegDescription::floating;
    desc.receiver.notional = 100'000'000.0;
    desc.receiver.compounding_frequency = 2;
    desc.receiver.spread = 0.0;
    desc.receiver.start_dates = start_dates;
    desc.receiver.end_dates = end_dates;

    // First period fixing is known at inception.
    desc.receiver.fixings[start_dates[0]] = euribor6m_fix;

    // Payer: fixed 5%
    desc.payer.type = LegDescription::fixed;
    desc.payer.notional = 100'000'000.0;
    desc.payer.fixed_rate = 0.05;
    desc.payer.start_dates = start_dates;
    desc.payer.end_dates = end_dates;

    auto pricer = Factory::instance()(desc);

    const double npv = pricer->price();

    // Expected numbers using Act/360 for both accrual and curve time, and continuous-comp DF.
    // (Receiver - payer) should be positive because fixed rate 5% is below par.
    BOOST_TEST(npv == 495775.94, boost::test_tools::tolerance(5e-3));
}

BOOST_AUTO_TEST_CASE(bond_irr_roundtrip)
{
    using Date = boost::gregorian::date;

    const Date val = parse_ddmmyy("01/01/25");
    const std::vector<Date> pillars = {
        parse_ddmmyy("01/01/26")
    };
    const std::vector<double> zc = {0.05};

    auto curve = std::make_shared<ZeroCouponCurve>(val, pillars, zc);

    // 1Y bond with annual coupon 5% (simplified single period).
    Bond::Period p{val, pillars[0]};
    Bond bond(100.0, 0.05, {p}, curve);

    const double y = 0.04; // 4% continuous yield
    const double price = bond.price_from_yield(y);

    const double irr = bond.irr_from_price(price, 0.05);

    BOOST_TEST(irr == y, boost::test_tools::tolerance(1e-8));
}

BOOST_AUTO_TEST_CASE(bond_irr_two_year_compare_with_boost_roots)
{
    const std::vector<double> times = {0.5, 1.0, 1.5, 2.0};
    const std::vector<double> zc_rates = {0.050, 0.058, 0.064, 0.068};
    const std::vector<double> cashflows = {3.0, 3.0, 3.0, 103.0};

    const double market_pv = 98.39;

    double pv_from_curve = 0.0;
    for (size_t i = 0; i < times.size(); ++i)
    {
        pv_from_curve += cashflows[i] * std::exp(-zc_rates[i] * times[i]);
    }
    BOOST_TEST(pv_from_curve == market_pv, boost::test_tools::tolerance(5e-4));

    auto f = [&](double y)
    {
        double pv = 0.0;
        for (size_t i = 0; i < times.size(); ++i)
        {
            pv += cashflows[i] * std::exp(-y * times[i]);
        }
        return pv - market_pv;
    };

    auto df = [&](double y)
    {
        double derivative = 0.0;
        for (size_t i = 0; i < times.size(); ++i)
        {
            derivative += -times[i] * cashflows[i] * std::exp(-y * times[i]);
        }
        return derivative;
    };

    const double irr_newton = GoalSeeker::newton_raphson(f, df, 0.06);

    auto f_boost = [&](double y)
    {
        return std::make_pair(f(y), df(y));
    };

    boost::uintmax_t max_iter = 100;
    const int digits = std::numeric_limits<double>::digits;
    const double irr_boost = boost::math::tools::newton_raphson_iterate(
        f_boost,
        0.06,
        0.0,
        1.0,
        digits,
        max_iter);

    BOOST_TEST(irr_newton == irr_boost, boost::test_tools::tolerance(1e-10));
}

BOOST_AUTO_TEST_CASE(bond_irr_two_year_using_bond_and_curve_classes)
{
    using Date = boost::gregorian::date;

    const Date val = parse_ddmmyy("01/01/25");
    const Date d1 = parse_ddmmyy("01/07/25");
    const Date d2 = parse_ddmmyy("01/01/26");
    const Date d3 = parse_ddmmyy("01/07/26");
    const Date d4 = parse_ddmmyy("01/01/27");

    const std::vector<Date> pillars = {d1, d2, d3, d4};
    const std::vector<double> zc = {0.050, 0.058, 0.064, 0.068};

    auto curve = std::make_shared<ZeroCouponCurve>(val, pillars, zc);

    std::vector<Bond::Period> schedule = {
        Bond::Period{val, d1},
        Bond::Period{d1, d2},
        Bond::Period{d2, d3},
        Bond::Period{d3, d4}
    };

    Bond bond(100.0, 0.06, schedule, curve);

    const double market_price = 98.39;
    const double irr_bond = bond.irr_from_price(market_price, 0.06);

    auto f = [&](double y)
    {
        return bond.price_from_yield(y) - market_price;
    };

    auto df = [&](double y)
    {
        const double h = 1e-6;
        return (f(y + h) - f(y - h)) / (2.0 * h);
    };

    auto f_boost = [&](double y)
    {
        return std::make_pair(f(y), df(y));
    };

    boost::uintmax_t max_iter = 100;
    const int digits = std::numeric_limits<double>::digits;
    const double irr_boost = boost::math::tools::newton_raphson_iterate(
        f_boost,
        0.06,
        0.0,
        1.0,
        digits,
        max_iter);

    BOOST_TEST(irr_bond == irr_boost, boost::test_tools::tolerance(1e-10));
}

BOOST_AUTO_TEST_SUITE_END()
