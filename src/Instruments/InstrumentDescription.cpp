#include <Instruments/InstrumentDescription.h>

#include <stdexcept>

void MarketDescription::validate() const
{
    if (pillar_dates.empty())
        throw std::invalid_argument("MarketDescription: empty curve");
    if (pillar_dates.size() != zc_rates_continuous.size())
        throw std::invalid_argument("MarketDescription: pillar_dates / zc_rates size mismatch");
}

void LegDescription::validate() const
{
    if (type == none)
        return;

    if (notional <= 0.0)
        throw std::invalid_argument("LegDescription: notional must be positive");

    if (start_dates.size() != end_dates.size())
        throw std::invalid_argument("LegDescription: start_dates / end_dates size mismatch");
    if (start_dates.empty())
        throw std::invalid_argument("LegDescription: empty schedule");

    if (type == fixed)
    {
        // fixed_rate can be 0 in edge cases (e.g. zero coupon), so just accept.
    }
    else if (type == floating)
    {
        if (compounding_frequency <= 0)
            throw std::invalid_argument("LegDescription: compounding_frequency must be positive");
    }
    else
    {
        throw std::invalid_argument("LegDescription: invalid type");
    }
}

void InstrumentDescription::validate() const
{
    market.validate();

    if (type == swap)
    {
        payer.validate();
        receiver.validate();
        if (payer.type == LegDescription::none || receiver.type == LegDescription::none)
            throw std::invalid_argument("InstrumentDescription(swap): both legs must be defined");
    }
    else if (type == bond)
    {
        receiver.validate();
        if (receiver.type != LegDescription::fixed)
            throw std::invalid_argument("InstrumentDescription(bond): receiver leg must be fixed");
    }
    else
    {
        throw std::invalid_argument("InstrumentDescription: invalid type");
    }
}
