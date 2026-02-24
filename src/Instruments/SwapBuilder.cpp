#include <Instruments/SwapBuilder.h>

#include <Instruments/FactoryRegistrator.h>
#include <Instruments/FixedLeg.h>
#include <Instruments/FloatingLeg.h>
#include <Instruments/Swap.h>
#include <Market/Index.h>
#include <Market/ZeroCouponCurve.h>

#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

namespace {

std::vector<FixedLeg::Period> build_fixed_schedule(const LegDescription& leg)
{
    std::vector<FixedLeg::Period> sched;
    sched.reserve(leg.start_dates.size());
    for (size_t i = 0; i < leg.start_dates.size(); ++i)
        sched.push_back(FixedLeg::Period{leg.start_dates[i], leg.end_dates[i]});
    return sched;
}

std::vector<FloatingLeg::Period> build_float_schedule(const LegDescription& leg)
{
    std::vector<FloatingLeg::Period> sched;
    sched.reserve(leg.start_dates.size());
    for (size_t i = 0; i < leg.start_dates.size(); ++i)
        sched.push_back(FloatingLeg::Period{leg.start_dates[i], leg.end_dates[i]});
    return sched;
}

std::unique_ptr<Leg> build_leg(const LegDescription& leg,
                              const std::shared_ptr<const ZeroCouponCurve>& curve,
                              const std::shared_ptr<const Index>& index)
{
    if (leg.type == LegDescription::fixed)
    {
        return std::make_unique<FixedLeg>(leg.notional,
                                          leg.fixed_rate,
                                          build_fixed_schedule(leg),
                                          curve,
                                          Actual_360{},
                                          leg.include_notional_at_maturity);
    }
    if (leg.type == LegDescription::floating)
    {
        auto fl = std::make_unique<FloatingLeg>(leg.notional,
                                                index,
                                                build_float_schedule(leg),
                                                curve,
                                                leg.spread,
                                                Actual_360{});
        for (const auto& kv : leg.fixings)
            fl->set_fixing(kv.first, kv.second);
        return fl;
    }

    throw std::invalid_argument("SwapBuilder: unsupported leg type");
}

} // namespace

InstrumentDescription::Type SwapBuilder::getId()
{
    return InstrumentDescription::Type::swap;
}

std::unique_ptr<IPricer> SwapBuilder::build(const InstrumentDescription& instrument)
{
    auto curve = std::make_shared<ZeroCouponCurve>(instrument.market.valuation_date,
                                                   instrument.market.pillar_dates,
                                                   instrument.market.zc_rates_continuous);
    const int m = (instrument.receiver.type == LegDescription::floating)
                      ? instrument.receiver.compounding_frequency
                      : instrument.payer.compounding_frequency;
    auto index = std::make_shared<Index>(curve, m);

    auto receiver = build_leg(instrument.receiver, curve, index);
    auto payer = build_leg(instrument.payer, curve, index);

    return std::make_unique<Swap>(std::move(receiver), std::move(payer));
}

namespace register_swap {
    FactoryRegistrator<SwapBuilder> registration;
}
