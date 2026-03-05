#define BOOST_TEST_MODULE test_instruments
#include <boost/test/included/unit_test.hpp>
#include <boost/math/tools/roots.hpp>

#include <DateCountCalculator/day_count_calculator.h>
#include <Instruments/Bootstrapping.h>
#include <Instruments/Bond.h>
#include <Instruments/Deposit.h>
#include <Instruments/Factory.h>
#include <Instruments/FixedLeg.h>
#include <Instruments/FloatingLeg.h>
#include <Instruments/InstrumentDescription.h>
#include <Instruments/Swap.h>
#include <Market/Index.h>
#include <Market/ZeroCouponCurve.h>

#include <cmath>
#include <limits>
#include <memory>
#include <vector>

BOOST_AUTO_TEST_SUITE(instruments_suite)

BOOST_AUTO_TEST_CASE(flow_amounts_for_fixed_and_floating_legs)
{
    using Date = boost::gregorian::date;

    const Date val = boost::gregorian::from_string("2016/04/01");
    const Date d6m = boost::gregorian::from_string("2016/10/03");

    auto curve = std::make_shared<ZeroCouponCurve>(val, std::vector<Date>{d6m}, std::vector<double>{0.05});
    auto index = std::make_shared<Index>(curve, 2);

    const std::vector<FixedLeg::Period> fixed_schedule = {
        FixedLeg::Period{val, d6m}
    };

    const std::vector<FloatingLeg::Period> floating_schedule = {
        FloatingLeg::Period{val, d6m}
    };

    const double notional = 100.0;
    const double fixed_rate = 0.05;
    const double fixing = 0.048;
    const double spread = 0.001;

    Actual_360 dcc;
    const double tau = dcc(val, d6m);

    FixedLeg fixed_leg(notional, fixed_rate, fixed_schedule, curve);
    const auto fixed_cfs = fixed_leg.cashflows();

    BOOST_TEST(fixed_cfs.size() == 1u);
    BOOST_TEST(fixed_cfs[0].pay_date == d6m);
    BOOST_TEST(fixed_cfs[0].amount == notional * fixed_rate * tau, boost::test_tools::tolerance(1e-12));

    FloatingLeg floating_leg(notional, index, floating_schedule, curve, spread);
    floating_leg.set_fixing(val, fixing);
    const auto floating_cfs = floating_leg.cashflows();

    BOOST_TEST(floating_cfs.size() == 1u);
    BOOST_TEST(floating_cfs[0].pay_date == d6m);
    BOOST_TEST(floating_cfs[0].amount == notional * (fixing + spread) * tau, boost::test_tools::tolerance(1e-12));
}

BOOST_AUTO_TEST_CASE(discounting_of_leg_cashflows)
{
    using Date = boost::gregorian::date;

    const Date val = boost::gregorian::from_string("2016/04/01");
    const Date d6m = boost::gregorian::from_string("2016/10/03");

    auto curve = std::make_shared<ZeroCouponCurve>(val, std::vector<Date>{d6m}, std::vector<double>{0.05});
    auto index = std::make_shared<Index>(curve, 2);

    const std::vector<FixedLeg::Period> fixed_schedule = {
        FixedLeg::Period{val, d6m}
    };

    const std::vector<FloatingLeg::Period> floating_schedule = {
        FloatingLeg::Period{val, d6m}
    };

    const double notional = 100.0;
    const double fixed_rate = 0.05;
    const double fixing = 0.048;
    const double spread = 0.001;

    Actual_360 dcc;
    const double tau = dcc(val, d6m);
    const double df = curve->discount_factor(d6m);

    FixedLeg fixed_leg(notional, fixed_rate, fixed_schedule, curve);
    const double expected_fixed_pv = (notional * fixed_rate * tau) * df;
    BOOST_TEST(fixed_leg.price() == expected_fixed_pv, boost::test_tools::tolerance(1e-12));

    FloatingLeg floating_leg(notional, index, floating_schedule, curve, spread);
    floating_leg.set_fixing(val, fixing);
    const double expected_floating_pv = (notional * (fixing + spread) * tau) * df;
    BOOST_TEST(floating_leg.price() == expected_floating_pv, boost::test_tools::tolerance(1e-12));
}

BOOST_AUTO_TEST_CASE(swap_example_from_slides)
{
    using Date = boost::gregorian::date;

    const Date val = boost::gregorian::from_string("2016/04/01");
    const std::vector<Date> pay_dates = {
        boost::gregorian::from_string("2016/10/03"),
        boost::gregorian::from_string("2017/04/03"),
        boost::gregorian::from_string("2017/10/02"),
        boost::gregorian::from_string("2018/04/02")
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

    const Date val = boost::gregorian::from_string("2025/01/01");
    const std::vector<Date> pillars = {
        boost::gregorian::from_string("2026/01/01")
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

BOOST_AUTO_TEST_CASE(bond_irr_two_year_using_bond_and_curve_classes)
{
    using Date = boost::gregorian::date;

    const Date val = boost::gregorian::from_string("2025/01/01");
    const Date d1 = boost::gregorian::from_string("2025/07/01");
    const Date d2 = boost::gregorian::from_string("2026/01/01");
    const Date d3 = boost::gregorian::from_string("2026/07/01");
    const Date d4 = boost::gregorian::from_string("2027/01/01");

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

BOOST_AUTO_TEST_CASE(bootstrap_curve_from_deposit_and_swaps)
{
    using Date = boost::gregorian::date;

    // Same semiannual schedule used in the course examples (dates already business-day adjusted).
    const Date val = boost::gregorian::from_string("2016/04/01");
    const Date d6m = boost::gregorian::from_string("2016/10/03");
    const Date d12m = boost::gregorian::from_string("2017/04/03");
    const Date d18m = boost::gregorian::from_string("2017/10/02");
    const Date d24m = boost::gregorian::from_string("2018/04/02");

    Bootstrapping::InstrumentsMapType instruments;
    instruments.emplace(d6m, std::make_shared<Deposit>(val, d6m, 0.05));
    instruments.emplace(d12m, std::make_shared<Swap>(d6m, d12m, 0.055, 2));
    instruments.emplace(d18m, std::make_shared<Swap>(d12m, d18m, 0.06, 2));
    instruments.emplace(d24m, std::make_shared<Swap>(d18m, d24m, 0.064, 2));

    Bootstrapping bs;
    const auto curve = bs(instruments);

    // Expected values with Act/360 accruals and the sequential bootstrap.
    BOOST_TEST(curve.at(d6m) == 0.974949221394719, boost::test_tools::tolerance(1e-12));
    BOOST_TEST(curve.at(d12m) == 0.9461362981333172, boost::test_tools::tolerance(1e-12));
    BOOST_TEST(curve.at(d18m) == 0.9135292118702222, boost::test_tools::tolerance(1e-12));
    BOOST_TEST(curve.at(d24m) == 0.8793138059619998, boost::test_tools::tolerance(1e-12));
}

BOOST_AUTO_TEST_SUITE_END()
