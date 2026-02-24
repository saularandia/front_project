#include <Instruments/BondBuilder.h>

#include <Instruments/Bond.h>
#include <Instruments/FactoryRegistrator.h>
#include <Market/ZeroCouponCurve.h>

#include <memory>
#include <stdexcept>
#include <vector>

namespace {
std::vector<Bond::Period> build_bond_schedule(const LegDescription& leg)
{
    std::vector<Bond::Period> sched;
    sched.reserve(leg.start_dates.size());
    for (size_t i = 0; i < leg.start_dates.size(); ++i)
        sched.push_back(Bond::Period{leg.start_dates[i], leg.end_dates[i]});
    return sched;
}
}

InstrumentDescription::Type BondBuilder::getId()
{
    return InstrumentDescription::Type::bond;
}

std::unique_ptr<IPricer> BondBuilder::build(const InstrumentDescription& instrument)
{
    auto curve = std::make_shared<ZeroCouponCurve>(instrument.market.valuation_date,
                                                   instrument.market.pillar_dates,
                                                   instrument.market.zc_rates_continuous);

    // Bond parameters are read from receiver leg description.
    const auto& leg = instrument.receiver;

    return std::make_unique<Bond>(leg.notional,
                                  leg.fixed_rate,
                                  build_bond_schedule(leg),
                                  curve,
                                  Actual_360{});
}

namespace register_bond {
    FactoryRegistrator<BondBuilder> registration;
}
