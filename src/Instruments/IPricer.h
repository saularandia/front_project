#ifndef INSTRUMENTS_IPRICER_H
#define INSTRUMENTS_IPRICER_H

#include <stdexcept>

class IPricer
{
public:
    virtual ~IPricer() = default;
    virtual double price() const = 0;

    // Optional hook for curve bootstrapping.
    // Default implementation throws because most pricers are not bootstrapping instruments.
    virtual double computeDiscountFactor(double& /*pre_annuity*/) const
    {
        throw std::logic_error("IPricer::computeDiscountFactor not implemented for this instrument");
    }
};

#endif
