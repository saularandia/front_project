#include <Instruments/Bootstrapping.h>

Bootstrapping::DiscountCurveType Bootstrapping::operator()(const InstrumentsMapType& instruments) const
{
    DiscountCurveType curve;
    double annuity = 0.0;

    for (const auto& instrument : instruments)
    {
        curve.emplace(
            instrument.first,
            instrument.second->computeDiscountFactor(annuity)
        );
    }

    return curve;
}
